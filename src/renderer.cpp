#include "src/include/allocator.hpp"
#include "src/include/engine.hpp"
#include "src/include/renderer.hpp"

#include <numbers>

namespace ge {

const unsigned int& Renderer::frameIndex() const {
  return m_frameIndex;
}

void Renderer::initialize(Engine& engine) {
  checkFormat(engine);
  checkPresentMode(engine);
  auto [imageCount, transform] = checkExtent(engine);

  createSwapchain(engine, imageCount, transform);
  createViews(engine);

  auto [tmp_dMem, tmp_dImg, tmp_dView] = Allocator::depthResources(engine);
  m_depthMem = std::move(tmp_dMem);
  m_depthImage = std::move(tmp_dImg);
  m_depthView = std::move(tmp_dView);

  m_renderCmds = engine.getCmds(QueueFamilyType::Main, engine.m_settings.buffer_mode);

  createSyncObjects(engine);

  float ar = static_cast<float>(engine.m_settings.extent.width) / engine.m_settings.extent.height;
  m_projection = mat4::perspective(std::numbers::pi / 3.0f, ar, 0.01f, 10000.0f);
}

void Renderer::render(const Engine& engine) {
  if (engine.m_context.device().waitForFences(*m_flightFences[m_frameIndex], true, ge_timeout) != vk::Result::eSuccess)
    throw std::runtime_error("groot-engine: hung waiting for flight fence");

  auto [res, imgIndex] = m_swapchain.acquireNextImage(ge_timeout, m_imageSemaphores[m_frameIndex], nullptr);
  if (res != vk::Result::eSuccess) throw std::runtime_error("groot-engine: failed to get next swapchain image");

  engine.m_context.device().resetFences(*m_flightFences[m_frameIndex]);

  m_renderCmds[m_frameIndex].reset();
  m_renderCmds[m_frameIndex].begin({});

  transitionImages(imgIndex);
  preDraw(engine, imgIndex);
  draw(engine);

  m_renderCmds[m_frameIndex].end();

  const vk::PipelineStageFlags waitStage =  vk::PipelineStageFlagBits::eColorAttachmentOutput;
  vk::SubmitInfo renderSubmit{
    .waitSemaphoreCount   = 1,
    .pWaitSemaphores      = &*m_imageSemaphores[m_frameIndex],
    .pWaitDstStageMask    = &waitStage,
    .commandBufferCount   = 1,
    .pCommandBuffers      = &*m_renderCmds[m_frameIndex],
    .signalSemaphoreCount = 1,
    .pSignalSemaphores    = &*m_renderSemaphores[m_frameIndex]
  };

  engine.m_context.queueFamily(QueueFamilyType::Main).queue.submit(renderSubmit, m_flightFences[m_frameIndex]);

  vk::PresentInfoKHR present{
    .waitSemaphoreCount = 1,
    .pWaitSemaphores    = &*m_renderSemaphores[m_frameIndex],
    .swapchainCount     = 1,
    .pSwapchains        = &*m_swapchain,
    .pImageIndices      = &imgIndex
  };

  if (engine.m_context.queueFamily(QueueFamilyType::Main).queue.presentKHR(present) != vk::Result::eSuccess)
    throw std::runtime_error("groot-engine: failed to present image");

  m_frameIndex = (m_frameIndex + 1) % engine.m_settings.buffer_mode;
}

void Renderer::checkFormat(Engine& engine) const {
  std::vector<vk::SurfaceFormatKHR> formats = engine.m_context.gpu().getSurfaceFormatsKHR(engine.m_surface);

  for (const auto& format : formats) {
    if (format.format == engine.m_settings.format && format.colorSpace == engine.m_settings.color_space)
      return;
  }

  engine.m_settings.format = formats[0].format;
  engine.m_settings.color_space = formats[0].colorSpace;
}

void Renderer::checkPresentMode(Engine& engine) const {
  for (const auto& presentMode : engine.m_context.gpu().getSurfacePresentModesKHR(engine.m_surface)) {
    if (presentMode == engine.m_settings.present_mode)
      return;
  }

  engine.m_settings.present_mode = vk::PresentModeKHR::eFifo;
}

std::pair<unsigned int, vk::SurfaceTransformFlagBitsKHR> Renderer::checkExtent(Engine& engine) const {
  vk::SurfaceCapabilitiesKHR capabilities = engine.m_context.gpu().getSurfaceCapabilitiesKHR(engine.m_surface);

  unsigned int imageCount = capabilities.minImageCount + 1;
  if (imageCount > capabilities.maxImageCount && capabilities.maxImageCount > 0)
    imageCount = capabilities.maxImageCount;

  if (capabilities.currentExtent.width != std::numeric_limits<unsigned int>::max()) {
    engine.m_settings.extent = capabilities.currentExtent;
    return std::make_pair(imageCount, capabilities.currentTransform);
  }

  int width, height;
  glfwGetFramebufferSize(engine.m_window, &width, &height);

  engine.m_settings.extent = vk::Extent2D{
    std::clamp(static_cast<unsigned int>(width),
      capabilities.minImageExtent.width, capabilities.maxImageExtent.width
    ),
    std::clamp(static_cast<unsigned int>(height),
      capabilities.minImageExtent.height, capabilities.maxImageExtent.height
    )
  };

  return std::make_pair(imageCount, capabilities.currentTransform);
}

void Renderer::createSwapchain(
  const Engine& engine,
  const unsigned int& imageCount,
  const vk::SurfaceTransformFlagBitsKHR& transform
) {
  m_swapchain = engine.m_context.device().createSwapchainKHR(vk::SwapchainCreateInfoKHR{
    .surface            = engine.m_surface,
    .minImageCount      = imageCount,
    .imageFormat        = engine.m_settings.format,
    .imageColorSpace    = engine.m_settings.color_space,
    .imageExtent        = engine.m_settings.extent,
    .imageArrayLayers   = 1,
    .imageUsage         = vk::ImageUsageFlagBits::eColorAttachment,
    .imageSharingMode   = vk::SharingMode::eExclusive,
    .preTransform       = transform,
    .compositeAlpha     = vk::CompositeAlphaFlagBitsKHR::eOpaque,
    .presentMode        = engine.m_settings.present_mode,
    .clipped            = true
  });

  m_images = std::move(m_swapchain.getImages());
}

void Renderer::createViews(const Engine& engine) {
  for (const auto& image : m_images) {
    m_views.emplace_back(engine.m_context.device().createImageView(vk::ImageViewCreateInfo{
      .image            = image,
      .viewType         = vk::ImageViewType::e2D,
      .format           = engine.m_settings.format,
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    }));
  }
}

void Renderer::createSyncObjects(const Engine& engine) {
  m_flightFences = Allocator::fences(engine, engine.m_settings.buffer_mode, true);
  m_imageSemaphores = Allocator::semaphores(engine, engine.m_settings.buffer_mode);
  m_renderSemaphores = Allocator::semaphores(engine, engine.m_settings.buffer_mode);
}

void Renderer::transitionImages(const unsigned int& imgIndex) {
  vk::ImageMemoryBarrier barrier{
    .dstAccessMask    = vk::AccessFlagBits::eColorAttachmentWrite,
    .newLayout        = vk::ImageLayout::eColorAttachmentOptimal,
    .image            = m_images[imgIndex],
    .subresourceRange = {
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .levelCount = 1,
      .layerCount = 1
    }
  };

  m_renderCmds[m_frameIndex].pipelineBarrier(
    vk::PipelineStageFlagBits::eTopOfPipe,
    vk::PipelineStageFlagBits::eColorAttachmentOutput,
    vk::DependencyFlags(),
    nullptr,
    nullptr,
    barrier
  );

  barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
  barrier.dstAccessMask = vk::AccessFlagBits::eNone;
  barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
  barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;

  m_renderCmds[m_frameIndex].pipelineBarrier(
    vk::PipelineStageFlagBits::eColorAttachmentOutput,
    vk::PipelineStageFlagBits::eBottomOfPipe,
    vk::DependencyFlags(),
    nullptr,
    nullptr,
    barrier
  );

  barrier.srcAccessMask = barrier.dstAccessMask;
  barrier.oldLayout = vk::ImageLayout::eUndefined;
  barrier.newLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
  barrier.image = m_depthImage;
  barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

  m_renderCmds[m_frameIndex].pipelineBarrier(
    vk::PipelineStageFlagBits::eTopOfPipe,
    vk::PipelineStageFlagBits::eEarlyFragmentTests,
    vk::DependencyFlags(),
    nullptr,
    nullptr,
    barrier
  );
}

void Renderer::preDraw(const Engine& engine, const unsigned int& imgIndex) {
  vk::RenderingAttachmentInfo color{
    .imageView    = m_views[imgIndex],
    .imageLayout  = vk::ImageLayout::eColorAttachmentOptimal,
    .loadOp       = vk::AttachmentLoadOp::eClear,
    .storeOp      = vk::AttachmentStoreOp::eStore,
    .clearValue   = { engine.m_settings.background_color }
  };

  vk::RenderingAttachmentInfo depth{
    .imageView    = m_depthView,
    .imageLayout  = vk::ImageLayout::eDepthAttachmentOptimal,
    .loadOp       = vk::AttachmentLoadOp::eClear,
    .storeOp      = vk::AttachmentStoreOp::eDontCare,
    .clearValue   = { .depthStencil = { 1, 0 } }
  };

  m_renderCmds[m_frameIndex].beginRendering(vk::RenderingInfo{
    .renderArea = {
      .extent = engine.m_settings.extent
    },
    .layerCount           = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments    = &color,
    .pDepthAttachment     = &depth
  });

  m_renderCmds[m_frameIndex].setViewport(0, vk::Viewport{
    .x        = 0,
    .y        = 0,
    .width    = static_cast<float>(engine.m_settings.extent.width),
    .height   = static_cast<float>(engine.m_settings.extent.height),
    .minDepth = 0,
    .maxDepth = 1
  });

  m_renderCmds[m_frameIndex].setScissor(0, vk::Rect2D{ .extent = engine.m_settings.extent });
}

void Renderer::draw(const Engine& engine) {
  EngineData engineData{
    .view           = m_view,
    .projection     = m_projection,
  };

  unsigned int materialIndex = 0;
  for (const auto& [material, pipeline, layout, sets] : engine.m_materials) {
    if (!engine.m_objects.hasObjects(material)) continue;

    m_renderCmds[m_frameIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

    m_renderCmds[m_frameIndex].bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics,
      *layout,
      0,
      *sets[m_frameIndex],
      nullptr
    );

    const auto& [vertexBuffer, indexBuffer, indirectBuffer, commandCount] = engine.m_objects[material];

    m_renderCmds[m_frameIndex].pushConstants(
      *layout,
      vk::ShaderStageFlagBits::eAll,
      0,
      vk::ArrayProxy<const char>(sizeof(EngineData), reinterpret_cast<const char *>(&engineData))
    );

    m_renderCmds[m_frameIndex].bindVertexBuffers(0, *vertexBuffer, { 0 });
    m_renderCmds[m_frameIndex].bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);

    m_renderCmds[m_frameIndex].drawIndexedIndirect(indirectBuffer, 0, commandCount, sizeof(Object::Command));
  }
  m_renderCmds[m_frameIndex].endRendering();
}

} // namespace ge