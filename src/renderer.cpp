#include "src/include/allocator.hpp"
#include "src/include/log.hpp"
#include "src/include/object.hpp"
#include "src/include/renderer.hpp"
#include "src/include/structs.hpp"
#include "src/include/vulkan_context.hpp"
#include "vulkan/vulkan.hpp"

#include <GLFW/glfw3.h>

#include <set>

namespace groot {

Renderer::Renderer(
  GLFWwindow * window,
  const VulkanContext * context,
  Allocator * allocator,
  Settings& settings
) {
  m_colorFormat = checkFormat(context, settings);
  m_depthFormat = getDepthFormat(context);
  m_presentMode = checkPresentMode(context, settings);
  m_extent = getExtent(window, context);

  m_clearColor.float32[0] = settings.background_color.x;
  m_clearColor.float32[1] = settings.background_color.y;
  m_clearColor.float32[2] = settings.background_color.z;
  m_clearColor.float32[3] = settings.background_color.w;

  m_flightFrames = settings.flight_frames;

  vk::SurfaceCapabilitiesKHR capabilities = context->gpu().getSurfaceCapabilitiesKHR(context->surface());
  unsigned int imageCount = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
    imageCount = capabilities.maxImageCount;

  vk::SwapchainCreateInfoKHR createInfo{
    .surface          = context->surface(),
    .minImageCount    = imageCount,
    .imageFormat      = m_colorFormat.format,
    .imageColorSpace  = m_colorFormat.colorSpace,
    .imageExtent      = m_extent,
    .imageArrayLayers = 1,
    .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
    .presentMode      = m_presentMode
  };

  m_swapchain = context->device().createSwapchainKHR(createInfo);
  m_images = context->device().getSwapchainImagesKHR(m_swapchain);

  for (const auto& image : m_images) {
    m_views.emplace_back(context->device().createImageView(vk::ImageViewCreateInfo{
      .image    = image,
      .viewType = vk::ImageViewType::e2D,
      .format   = m_colorFormat.format,
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    }));
  }

  m_depthImage = allocator->allocateImage(vk::ImageCreateInfo{
    .imageType    = vk::ImageType::e2D,
    .format       = m_depthFormat,
    .extent       = { m_extent.width, m_extent.height, 1 },
    .mipLevels    = 1,
    .arrayLayers  = 1,
    .samples      = vk::SampleCountFlagBits::e1,
    .tiling       = vk::ImageTiling::eOptimal,
    .usage        = vk::ImageUsageFlagBits::eDepthStencilAttachment
  });

  m_depthView = context->device().createImageView(vk::ImageViewCreateInfo{
    .image    = m_depthImage,
    .viewType = vk::ImageViewType::e2D,
    .format   = m_depthFormat,
    .subresourceRange = {
      .aspectMask = vk::ImageAspectFlagBits::eDepth,
      .levelCount = 1,
      .layerCount = 1
    }
  });

  m_cmds = context->createRenderBuffers(m_flightFrames);
  m_fences = context->createFlightFences(m_flightFrames);
  m_imageSemaphores = context->createRenderSemaphores(m_flightFrames);
  m_renderSemaphores = context->createRenderSemaphores(m_flightFrames);
}

std::pair<unsigned int, unsigned int> Renderer::extent() const {
  return std::make_pair(m_extent.width, m_extent.height);
}

vk::Format Renderer::depthFormat() const {
  return m_depthFormat;
}

unsigned int Renderer::prepFrame(const vk::Device& device) const {
  if (device.waitForFences(m_fences[m_frameIndex], true, 1000000000) != vk::Result::eSuccess)
    Log::runtime_error("hung waiting for next frame");

  auto [res, imgIndex] = device.acquireNextImageKHR(
    m_swapchain, 1000000000, m_imageSemaphores[m_frameIndex], nullptr
  );
  if (res != vk::Result::eSuccess)
    Log::runtime_error("hung waiting for next render target");

  device.resetFences(m_fences[m_frameIndex]);

  m_cmds[m_frameIndex].reset();
  m_cmds[m_frameIndex].begin(vk::CommandBufferBeginInfo{});

  vk::ImageMemoryBarrier barrier{
    .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
    .oldLayout      = vk::ImageLayout::eUndefined,
    .newLayout      = vk::ImageLayout::eColorAttachmentOptimal,
    .image          = m_images[imgIndex],
    .subresourceRange = {
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .levelCount = 1,
      .layerCount = 1
    }
  };

  m_cmds[m_frameIndex].pipelineBarrier(
    vk::PipelineStageFlagBits::eTopOfPipe,
    vk::PipelineStageFlagBits::eColorAttachmentOutput,
    vk::DependencyFlags(),
    nullptr,
    nullptr,
    barrier
  );

  return imgIndex;
}

const vk::CommandBuffer& Renderer::renderBuffer() const {
  return m_cmds[m_frameIndex];
}

void Renderer::render(
  const VulkanContext * context,
  const std::set<Object>& scene,
  const std::unordered_map<RID, unsigned long, RID::Hash>& resources,
  unsigned int imgIndex
) {
  vk::RenderingAttachmentInfo color{
      .imageView    = m_views[imgIndex],
      .imageLayout  = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp       = vk::AttachmentLoadOp::eClear,
      .storeOp      = vk::AttachmentStoreOp::eStore,
      .clearValue   = { m_clearColor }
    };

    vk::RenderingAttachmentInfo depth{
      .imageView    = m_depthView,
      .imageLayout  = vk::ImageLayout::eDepthAttachmentOptimal,
      .loadOp       = vk::AttachmentLoadOp::eClear,
      .storeOp      = vk::AttachmentStoreOp::eDontCare,
      .clearValue   = { .depthStencil = { 1, 0 } }
    };

    m_cmds[m_frameIndex].beginRendering(vk::RenderingInfo{
      .renderArea           = { .extent = m_extent },
      .layerCount           = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments    = &color,
      .pDepthAttachment     = &depth
    });

    m_cmds[m_frameIndex].setViewport(0, vk::Viewport{
      .x        = 0,
      .y        = 0,
      .width    = static_cast<float>(m_extent.width),
      .height   = static_cast<float>(m_extent.height),
      .minDepth = 0,
      .maxDepth = 1
    });

    m_cmds[m_frameIndex].setScissor(0, vk::Rect2D{ .extent = m_extent });

  for (const auto& object : scene) {
    PipelineHandle * pipeline = reinterpret_cast<PipelineHandle *>(resources.at(object.m_pipeline));
    DescriptorSetHandle * set = reinterpret_cast<DescriptorSetHandle *>(resources.at(object.m_set));
    MeshHandle * mesh = reinterpret_cast<MeshHandle *>(resources.at(object.m_mesh));

    m_cmds[m_frameIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->pipeline);

    m_cmds[m_frameIndex].bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics,
      pipeline->layout,
      0,
      1,
      &set->set,
      0,
      nullptr
    );

    m_cmds[m_frameIndex].bindVertexBuffers(0, mesh->vertexBuffer, { 0 });
    m_cmds[m_frameIndex].bindIndexBuffer(mesh->indexBuffer, 0, vk::IndexType::eUint32);
    m_cmds[m_frameIndex].drawIndexed(mesh->indexCount, 1, 0, 0, 0);
  }

  m_cmds[m_frameIndex].endRendering();

  vk::ImageMemoryBarrier barrier{
    .srcAccessMask  = vk::AccessFlagBits::eColorAttachmentWrite,
    .oldLayout      = vk::ImageLayout::eColorAttachmentOptimal,
    .newLayout      = vk::ImageLayout::ePresentSrcKHR,
    .image          = m_images[imgIndex],
    .subresourceRange = {
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .levelCount = 1,
      .layerCount = 1
    }
  };

  m_cmds[m_frameIndex].pipelineBarrier(
    vk::PipelineStageFlagBits::eColorAttachmentOutput,
    vk::PipelineStageFlagBits::eBottomOfPipe,
    vk::DependencyFlags(),
    nullptr,
    nullptr,
    barrier
  );

  m_cmds[m_frameIndex].end();

  context->submitRender(RenderInfo{
    .cmdBuf           = m_cmds[m_frameIndex],
    .fence            = m_fences[m_frameIndex],
    .imageSemaphore   = m_imageSemaphores[m_frameIndex],
    .renderSemaphore  = m_renderSemaphores[m_frameIndex],
    .swapchain        = m_swapchain,
    .imgIndex         = imgIndex
  });

  m_frameIndex = (m_frameIndex + 1) % m_flightFrames;
}

void Renderer::destroy(const VulkanContext * context, Allocator * allocator) {
  context->destroyRenderBuffers(m_cmds);

  for (const auto& fence : m_fences)
    context->device().destroyFence(fence);

  for (const auto& semaphore : m_imageSemaphores)
    context->device().destroySemaphore(semaphore);

  for (const auto& semaphore : m_renderSemaphores)
    context->device().destroySemaphore(semaphore);

  context->device().destroyImageView(m_depthView);
  allocator->destroyImage(m_depthImage);

  for (const auto& view : m_views)
    context->device().destroyImageView(view);

  context->device().destroySwapchainKHR(m_swapchain);
}

vk::SurfaceFormatKHR Renderer::checkFormat(const VulkanContext * context, Settings& settings) const {
  std::vector<vk::SurfaceFormatKHR> formats = context->gpu().getSurfaceFormatsKHR(context->surface());
  for (const auto& format : formats) {
    if (format.format == static_cast<vk::Format>(settings.color_format) && format.colorSpace == static_cast<vk::ColorSpaceKHR>(settings.color_space))
      return format;
  }

  vk::SurfaceFormatKHR format = formats.front();
  Log::warn(std::format("chosen format/colorspace incompatible. using {}/{} instead",
    vk::to_string(format.format), vk::to_string(format.colorSpace)
  ));

  settings.color_format = static_cast<Format>(format.format);
  settings.color_space = static_cast<ColorSpace>(format.colorSpace);

  return format;
}

vk::Format Renderer::getDepthFormat(const VulkanContext * context) const {
  vk::FormatProperties props = context->gpu().getFormatProperties(vk::Format::eD32Sfloat);
  if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
    return vk::Format::eD32Sfloat;

  props = context->gpu().getFormatProperties(vk::Format::eD24UnormS8Uint);
  if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
    return vk::Format::eD24UnormS8Uint;

  return vk::Format::eD16Unorm;
}

vk::PresentModeKHR Renderer::checkPresentMode(const VulkanContext * context, Settings& settings) const {
  std::vector<vk::PresentModeKHR> modes = context->gpu().getSurfacePresentModesKHR(context->surface());
  for (const auto& mode : modes) {
    if (mode == static_cast<vk::PresentModeKHR>(settings.render_mode))
      return mode;
  }

  settings.render_mode = RenderMode::VSync;

  Log::warn("Chosen render mode not available. Defaulting to VSync");
  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Renderer::getExtent(GLFWwindow * window, const VulkanContext * context) const {
  vk::SurfaceCapabilitiesKHR capabilities = context->gpu().getSurfaceCapabilitiesKHR(context->surface());

  if (capabilities.currentExtent.width != UINT32_MAX)
    return capabilities.currentExtent;

  vk::Extent2D extent;

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  extent.width = std::clamp(
    static_cast<unsigned int>(width),
    capabilities.minImageExtent.width,
    capabilities.maxImageExtent.width
  );

  extent.height = std::clamp(
    static_cast<unsigned int>(height),
    capabilities.minImageExtent.height,
    capabilities.maxImageExtent.height
  );

  return extent;
}

} // namespace groot
