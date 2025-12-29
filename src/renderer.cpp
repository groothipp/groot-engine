#include "src/include/allocator.hpp"
#include "src/include/gui.hpp"
#include "src/include/log.hpp"
#include "src/include/object.hpp"
#include "src/include/renderer.hpp"
#include "src/include/structs.hpp"
#include "src/include/vulkan_context.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

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

  vk::SurfaceCapabilitiesKHR capabilities = context->gpu().getSurfaceCapabilitiesKHR(context->surface());
  unsigned int imageCount = std::clamp(
    settings.flight_frames, capabilities.minImageCount, capabilities.maxImageCount
  );
  m_flightFrames = imageCount;
  settings.flight_frames = imageCount;

  vk::SwapchainCreateInfoKHR createInfo{
    .surface          = context->surface(),
    .minImageCount    = imageCount,
    .imageFormat      = m_colorFormat.format,
    .imageColorSpace  = m_colorFormat.colorSpace,
    .imageExtent      = m_extent,
    .imageArrayLayers = 1,
    .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment  |
                        vk::ImageUsageFlagBits::eStorage          |
                        vk::ImageUsageFlagBits::eTransferDst,
    .presentMode      = m_presentMode
  };

  m_swapchain = context->device().createSwapchainKHR(createInfo);
  m_images = context->device().getSwapchainImagesKHR(m_swapchain);

  std::vector<vk::CommandBuffer> cmds = context->transferCmds(1);
  vk::CommandBuffer& cmd = cmds[0];
  cmd.begin(vk::CommandBufferBeginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

  m_drawImages.reserve(m_images.size());
  m_drawViews.reserve(m_images.size());
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

    m_drawImages.emplace_back(allocator->allocateImage(vk::ImageCreateInfo{
      .imageType    = vk::ImageType::e2D,
      .format       = m_colorFormat.format,
      .extent       = { m_extent.width, m_extent.height, 1 },
      .mipLevels    = 1,
      .arrayLayers  = 1,
      .samples      = vk::SampleCountFlagBits::e1,
      .tiling       = vk::ImageTiling::eOptimal,
      .usage        = vk::ImageUsageFlagBits::eColorAttachment  |
                      vk::ImageUsageFlagBits::eStorage          |
                      vk::ImageUsageFlagBits::eTransferSrc
    }));

    m_drawViews.emplace_back(context->device().createImageView(vk::ImageViewCreateInfo{
      .image    = m_drawImages.back(),
      .viewType = vk::ImageViewType::e2D,
      .format   = m_colorFormat.format,
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    }));

    vk::ImageMemoryBarrier drawBarrier{
      .newLayout  = vk::ImageLayout::eGeneral,
      .image      = m_drawImages.back(),
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    };

    vk::ImageMemoryBarrier renderBarrier{
      .newLayout  = vk::ImageLayout::ePresentSrcKHR,
      .image      = image,
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    };

    cmd.pipelineBarrier(
      vk::PipelineStageFlagBits::eTopOfPipe,
      vk::PipelineStageFlagBits::eBottomOfPipe,
      vk::DependencyFlags(),
      nullptr,
      nullptr,
      { drawBarrier, renderBarrier }
    );
  }

  cmd.end();

  vk::Fence transferFence = context->device().createFence({});

  auto [index, transferQueue] = context->transferQueue();
  transferQueue.submit(vk::SubmitInfo{
    .commandBufferCount = 1,
    .pCommandBuffers    = &cmd
  }, transferFence);

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

  vk::DescriptorPoolSize poolSizes[] = {
    { vk::DescriptorType::eSampler, 1000 },
    { vk::DescriptorType::eCombinedImageSampler, 1000 },
    { vk::DescriptorType::eSampledImage, 1000 },
    { vk::DescriptorType::eStorageImage, 1000 },
    { vk::DescriptorType::eUniformTexelBuffer, 1000 },
    { vk::DescriptorType::eStorageTexelBuffer, 1000 },
    { vk::DescriptorType::eUniformBuffer, 1000 },
    { vk::DescriptorType::eStorageBuffer, 1000 },
    { vk::DescriptorType::eUniformBufferDynamic, 1000 },
    { vk::DescriptorType::eStorageBufferDynamic, 1000 },
    { vk::DescriptorType::eInputAttachment, 1000 }
  };

  m_dispatchCmds = context->computeCmds(m_flightFrames);
  m_drawCmds = context->graphicsCmds(m_flightFrames);
  m_postProcessCmds = context->computeCmds(m_flightFrames);
  m_uiCmds = context->graphicsCmds(m_flightFrames);

  for (unsigned int i = 0; i < m_flightFrames; ++i) {
    m_flightFences.emplace_back(
      context->device().createFence(vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled })
    );
    m_imageSemaphores.emplace_back(context->device().createSemaphore({}));
    m_dispatchSemaphores.emplace_back(context->device().createSemaphore({}));
    m_drawSemaphores.emplace_back(context->device().createSemaphore({}));
    m_postProcessSemaphores.emplace_back(context->device().createSemaphore({}));
    m_uiSemaphores.emplace_back(context->device().createSemaphore({}));
  }

  m_guiDescriptorPool = context->device().createDescriptorPool(vk::DescriptorPoolCreateInfo{
    .flags          = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
    .maxSets        = 1000,
    .poolSizeCount  = 11,
    .pPoolSizes     = poolSizes
  });

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui::StyleColorsDark();

  ImGuiStyle& style = ImGui::GetStyle();

  ImVec4 darkBrown    = ImVec4(0.08f, 0.06f, 0.04f, 1.00f);
  ImVec4 brown        = ImVec4(0.40f, 0.30f, 0.20f, 1.00f);
  ImVec4 lightBrown   = ImVec4(0.60f, 0.45f, 0.30f, 1.00f);
  ImVec4 darkGreen    = ImVec4(0.20f, 0.25f, 0.15f, 1.00f);
  ImVec4 green        = ImVec4(0.35f, 0.40f, 0.25f, 1.00f);
  ImVec4 lightGreen   = ImVec4(0.50f, 0.55f, 0.35f, 1.00f);

  style.Colors[ImGuiCol_WindowBg] = darkBrown;
  style.Colors[ImGuiCol_ChildBg]  = darkBrown;
  style.Colors[ImGuiCol_PopupBg]  = darkBrown;

  style.Colors[ImGuiCol_TitleBg]          = brown;
  style.Colors[ImGuiCol_TitleBgActive]    = green;
  style.Colors[ImGuiCol_TitleBgCollapsed] = darkBrown;

  style.Colors[ImGuiCol_Button]         = brown;
  style.Colors[ImGuiCol_ButtonHovered]  = lightBrown;
  style.Colors[ImGuiCol_ButtonActive]   = green;

  style.Colors[ImGuiCol_Header]         = darkGreen;
  style.Colors[ImGuiCol_HeaderHovered]  = green;
  style.Colors[ImGuiCol_HeaderActive]   = lightGreen;

  style.Colors[ImGuiCol_FrameBg]        = darkBrown;
  style.Colors[ImGuiCol_FrameBgHovered] = brown;
  style.Colors[ImGuiCol_FrameBgActive]  = green;

  style.Colors[ImGuiCol_CheckMark]        = lightGreen;
  style.Colors[ImGuiCol_SliderGrab]       = green;
  style.Colors[ImGuiCol_SliderGrabActive] = lightGreen;

  style.Colors[ImGuiCol_Tab]                = brown;
  style.Colors[ImGuiCol_TabHovered]         = lightBrown;
  style.Colors[ImGuiCol_TabActive]          = green;
  style.Colors[ImGuiCol_TabUnfocused]       = darkBrown;
  style.Colors[ImGuiCol_TabUnfocusedActive] = brown;

  style.Colors[ImGuiCol_Border]       = lightBrown;
  style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

  style.Colors[ImGuiCol_Text]         = ImVec4(0.90f, 0.90f, 0.85f, 1.00f);
  style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

  style.Colors[ImGuiCol_ScrollbarBg]          = darkBrown;
  style.Colors[ImGuiCol_ScrollbarGrab]        = brown;
  style.Colors[ImGuiCol_ScrollbarGrabHovered] = lightBrown;
  style.Colors[ImGuiCol_ScrollbarGrabActive]  = green;

  style.Colors[ImGuiCol_ResizeGrip]         = brown;
  style.Colors[ImGuiCol_ResizeGripHovered]  = lightBrown;
  style.Colors[ImGuiCol_ResizeGripActive]   = green;

  style.WindowRounding = 5.0f;
  style.FrameRounding  = 3.0f;
  style.GrabRounding   = 3.0f;

  ImGui_ImplGlfw_InitForVulkan(window, true);

  auto [queueFamily, graphicsQueue] = context->graphicsQueue();
  ImGui_ImplVulkan_InitInfo guiInitInfo{
    .Instance             = context->instance(),
    .PhysicalDevice       = context->gpu(),
    .Device               = context->device(),
    .QueueFamily          = queueFamily,
    .Queue                = graphicsQueue,
    .DescriptorPool       = m_guiDescriptorPool,
    .MinImageCount        = capabilities.minImageCount,
    .ImageCount           = imageCount,
    .PipelineInfoMain     = {
      .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
      .PipelineRenderingCreateInfo = {
        .sType                    = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount     = 1,
        .pColorAttachmentFormats  = reinterpret_cast<VkFormat *>(&m_colorFormat.format)
      }
    },
    .UseDynamicRendering  = true
  };

  ImGui_ImplVulkan_Init(&guiInitInfo);

  if (context->device().waitForFences(transferFence, true, 1000000000) != vk::Result::eSuccess)
    Log::runtime_error("Hung waiting for swapchain image transitions");

  context->destroyTransferCmds(cmds);
  context->device().destroyFence(transferFence);
}

vk::Format Renderer::depthFormat() const {
  return m_depthFormat;
}

std::pair<unsigned int, unsigned int> Renderer::extent() const {
  return std::make_pair(m_extent.width, m_extent.height);
}

std::pair<const vk::Image&, const vk::ImageView&> Renderer::renderTarget(unsigned int index) const {
  return { m_images[index], m_views[index] };
}

std::pair<const vk::Image&, const vk::ImageView&> Renderer::drawTarget(unsigned int index) const {
  return { m_drawImages[index], m_drawViews[index] };
}

unsigned int Renderer::frameIndex() const {
  return m_frameIndex;
}

void Renderer::destroy(const VulkanContext * context, Allocator * allocator) {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  context->device().destroyDescriptorPool(m_guiDescriptorPool);

  context->device().destroyImageView(m_depthView);
  allocator->destroyImage(m_depthImage);

  context->destroyComputeCmds(m_dispatchCmds);
  context->destroyGraphicsCmds(m_drawCmds);
  context->destroyComputeCmds(m_postProcessCmds);
  context->destroyGraphicsCmds(m_uiCmds);

  for (unsigned int i = 0; i < m_flightFrames; ++i) {
    context->device().destroyFence(m_flightFences[i]);
    context->device().destroySemaphore(m_imageSemaphores[i]);
    context->device().destroySemaphore(m_dispatchSemaphores[i]);
    context->device().destroySemaphore(m_drawSemaphores[i]);
    context->device().destroySemaphore(m_postProcessSemaphores[i]);
    context->device().destroySemaphore(m_uiSemaphores[i]);
  }

  for (const auto& view : m_views)
    context->device().destroyImageView(view);

  for (const auto& image : m_drawImages)
    allocator->destroyImage(image);

  for (const auto& view : m_drawViews)
    context->device().destroyImageView(view);

  context->device().destroySwapchainKHR(m_swapchain);
}

void Renderer::prepFrame(const VulkanContext * context, std::unordered_map<RID, unsigned long, RID::Hash>& resources) {
  vk::Fence& flightFence = m_flightFences[m_frameIndex];
  if (context->device().waitForFences(flightFence, true, 1000000000) != vk::Result::eSuccess)
    Log::runtime_error("Hung waiting for new frame");
  context->device().resetFences(flightFence);
}

void Renderer::dispatch(
  const VulkanContext * context,
  const ComputeCommand& command,
  const std::unordered_map<RID, unsigned long, RID::Hash>& resources
) {
  vk::CommandBuffer& cmd = m_preDraw ? m_dispatchCmds[m_frameIndex] : m_postProcessCmds[m_frameIndex];

  PipelineHandle * pipeline = reinterpret_cast<PipelineHandle *>(resources.at(command.pipeline));
  DescriptorSetHandle * set = reinterpret_cast<DescriptorSetHandle *>(resources.at(command.descriptor_set));

  cmd.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline->pipeline);
  cmd.bindDescriptorSets(
    vk::PipelineBindPoint::eCompute,
    pipeline->layout,
    0,
    set->set,
    nullptr
  );

  if (!command.push_constants.empty()) {
    cmd.pushConstants<unsigned char>(
      pipeline->layout,
      vk::ShaderStageFlagBits::eCompute,
      0,
      command.push_constants
    );
  }

  if (command.barrier) {
    vk::MemoryBarrier barrier{
      .srcAccessMask = vk::AccessFlagBits::eShaderWrite,
      .dstAccessMask  = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite
    };

    cmd.pipelineBarrier(
      vk::PipelineStageFlagBits::eComputeShader,
      vk::PipelineStageFlagBits::eComputeShader,
      {},
      barrier,
      nullptr,
      nullptr
    );
  }

  auto [group_x, group_y, group_z] = command.work_groups;
  cmd.dispatch(group_x, group_y, group_z);
}

void Renderer::beginDispatch(const VulkanContext * context, const std::set<unsigned long>& imageHandles) {
  m_preDraw = true;

  vk::CommandBuffer& cmd = m_dispatchCmds[m_frameIndex];
  cmd.reset();
  cmd.begin(vk::CommandBufferBeginInfo{});

  std::vector<vk::ImageMemoryBarrier> barriers;
  for (const auto& handle : imageHandles) {
    barriers.emplace_back(vk::ImageMemoryBarrier{
      .dstAccessMask  = vk::AccessFlagBits::eShaderWrite,
      .oldLayout      = vk::ImageLayout::eShaderReadOnlyOptimal,
      .newLayout      = vk::ImageLayout::eGeneral,
      .image          = reinterpret_cast<VkImage>(handle),
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    });
  }

  cmd.pipelineBarrier(
    vk::PipelineStageFlagBits::eTopOfPipe,
    vk::PipelineStageFlagBits::eComputeShader,
    {},
    nullptr,
    nullptr,
    barriers
  );
}

void Renderer::endDispatch(const VulkanContext * context, const std::set<unsigned long>& imageHandles) {
  vk::CommandBuffer& cmd = m_dispatchCmds[m_frameIndex];

  auto [graphicsIndex, graphicsQueue] = context->graphicsQueue();
  auto [computeIndex, computeQueue] = context->computeQueue();
  bool sameQueue = graphicsIndex == computeIndex;

  std::vector<vk::ImageMemoryBarrier> barriers;
  for (const auto& handle : imageHandles) {
    barriers.emplace_back(vk::ImageMemoryBarrier{
      .srcAccessMask        = vk::AccessFlagBits::eShaderWrite,
      .oldLayout            = vk::ImageLayout::eGeneral,
      .newLayout            = vk::ImageLayout::eShaderReadOnlyOptimal,
      .srcQueueFamilyIndex  = sameQueue ? vk::QueueFamilyIgnored : computeIndex,
      .dstQueueFamilyIndex  = sameQueue ? vk::QueueFamilyIgnored : graphicsIndex,
      .image                = reinterpret_cast<VkImage>(handle),
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    });
  }

  cmd.pipelineBarrier(
    vk::PipelineStageFlagBits::eComputeShader,
    vk::PipelineStageFlagBits::eBottomOfPipe,
    {},
    nullptr,
    nullptr,
    barriers
  );

  cmd.end();
}

unsigned int Renderer::draw(
  const VulkanContext * context,
  const std::set<unsigned long>& imageHandles,
  const std::unordered_map<RID, unsigned long, RID::Hash>& resources,
  const std::set<Object>& scene
) {
  vk::CommandBuffer& cmd = m_drawCmds[m_frameIndex];
  cmd.reset();
  cmd.begin(vk::CommandBufferBeginInfo{});

  auto [graphicsIndex, graphicsQueue] = context->graphicsQueue();
  auto [computeIndex, computeQueue] = context->computeQueue();
  bool sameQueue = graphicsIndex == computeIndex;

  if (!sameQueue) {
    std::vector<vk::ImageMemoryBarrier> barriers;
    for (const auto& handle : imageHandles) {
      barriers.emplace_back(vk::ImageMemoryBarrier{
        .dstAccessMask        = vk::AccessFlagBits::eShaderRead,
        .oldLayout            = vk::ImageLayout::eShaderReadOnlyOptimal,
        .newLayout            = vk::ImageLayout::eShaderReadOnlyOptimal,
        .srcQueueFamilyIndex  = computeIndex,
        .dstQueueFamilyIndex  = graphicsIndex,
        .image                = reinterpret_cast<VkImage>(handle),
        .subresourceRange = {
          .aspectMask = vk::ImageAspectFlagBits::eColor,
          .levelCount = 1,
          .layerCount = 1
        }
      });
    }
    cmd.pipelineBarrier(
      vk::PipelineStageFlagBits::eTopOfPipe,
      vk::PipelineStageFlagBits::eVertexShader,
      {},
      nullptr,
      nullptr,
      barriers
    );
  }

  auto [result, imgIndex] = context->device().acquireNextImageKHR(m_swapchain, 1000000000, m_imageSemaphores[m_frameIndex]);
  if (result != vk::Result::eSuccess)
    Log::runtime_error("Hung waiting for next render target");

  auto [drawImage, drawView] = drawTarget(imgIndex);
  vk::ImageMemoryBarrier barrier{
    .dstAccessMask  = vk::AccessFlagBits::eColorAttachmentWrite,
    .oldLayout      = vk::ImageLayout::eGeneral,
    .newLayout      = vk::ImageLayout::eColorAttachmentOptimal,
    .image          = drawImage,
    .subresourceRange = {
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .levelCount = 1,
      .layerCount = 1
    }
  };

  cmd.pipelineBarrier(
    vk::PipelineStageFlagBits::eTopOfPipe,
    vk::PipelineStageFlagBits::eColorAttachmentOutput,
    {},
    nullptr,
    nullptr,
    barrier
  );

  vk::RenderingAttachmentInfo colorAttachment{
    .imageView    = drawView,
    .imageLayout  = vk::ImageLayout::eColorAttachmentOptimal,
    .loadOp       = vk::AttachmentLoadOp::eClear,
    .storeOp      = vk::AttachmentStoreOp::eStore,
    .clearValue   = { m_clearColor }
  };

  vk::RenderingAttachmentInfo depthAttachment{
    .imageView    = m_depthView,
    .imageLayout  = vk::ImageLayout::eDepthAttachmentOptimal,
    .loadOp       = vk::AttachmentLoadOp::eClear,
    .storeOp      = vk::AttachmentStoreOp::eDontCare,
    .clearValue   = { .depthStencil = { 1, 0 } }
  };

  cmd.beginRendering(vk::RenderingInfo{
    .renderArea           = { .extent = m_extent },
    .layerCount           = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments    = &colorAttachment,
    .pDepthAttachment     = &depthAttachment
  });

  cmd.setViewport(0, vk::Viewport{
    .x        = 0.0,
    .y        = 0.0,
    .width    = static_cast<float>(m_extent.width),
    .height   = static_cast<float>(m_extent.height),
    .minDepth = 0.0,
    .maxDepth = 1.0
  });

  cmd.setScissor(0, vk::Rect2D{ .extent = m_extent });

  for (const auto& object : scene) {
    PipelineHandle * pipeline = reinterpret_cast<PipelineHandle *>(resources.at(object.m_pipeline));
    DescriptorSetHandle * set = reinterpret_cast<DescriptorSetHandle *>(resources.at(object.m_set));
    MeshHandle * mesh = reinterpret_cast<MeshHandle *>(resources.at(object.m_mesh));

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->pipeline);
    cmd.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics,
      pipeline->layout,
      0,
      set->set,
      nullptr
    );
    cmd.bindVertexBuffers(0, mesh->vertexBuffer, { 0 });
    cmd.bindIndexBuffer(mesh->indexBuffer, 0, vk::IndexType::eUint32);
    cmd.drawIndexed(mesh->indexCount, 1, 0, 0, 0);
  }

  cmd.endRendering();

  auto [renderImage, renderView] = renderTarget(imgIndex);

  std::array<vk::ImageMemoryBarrier, 2> transferBarriers{
    vk::ImageMemoryBarrier{
      .srcAccessMask  = vk::AccessFlagBits::eColorAttachmentWrite,
      .dstAccessMask  = vk::AccessFlagBits::eTransferRead,
      .oldLayout      = vk::ImageLayout::eColorAttachmentOptimal,
      .newLayout      = vk::ImageLayout::eTransferSrcOptimal,
      .image          = drawImage,
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    },
    vk::ImageMemoryBarrier{
      .dstAccessMask  = vk::AccessFlagBits::eTransferWrite,
      .oldLayout      = vk::ImageLayout::ePresentSrcKHR,
      .newLayout      = vk::ImageLayout::eTransferDstOptimal,
      .image          = renderImage,
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    }
  };

  cmd.pipelineBarrier(
    vk::PipelineStageFlagBits::eColorAttachmentOutput,
    vk::PipelineStageFlagBits::eTransfer,
    {},
    nullptr,
    nullptr,
    transferBarriers
  );

  cmd.copyImage(
    drawImage, vk::ImageLayout::eTransferSrcOptimal,
    renderImage, vk::ImageLayout::eTransferDstOptimal,
    vk::ImageCopy{
      .srcSubresource = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .layerCount = 1
      },
      .dstSubresource = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .layerCount = 1
      },
      .extent = { m_extent.width, m_extent.height, 1 }
    }
  );

  std::array<vk::ImageMemoryBarrier, 2> postProcessBarriers{
    vk::ImageMemoryBarrier{
      .srcAccessMask        = vk::AccessFlagBits::eTransferRead,
      .oldLayout            = vk::ImageLayout::eTransferSrcOptimal,
      .newLayout            = vk::ImageLayout::eGeneral,
      .srcQueueFamilyIndex  = sameQueue ? vk::QueueFamilyIgnored : graphicsIndex,
      .dstQueueFamilyIndex  = sameQueue ? vk::QueueFamilyIgnored : computeIndex,
      .image                = drawImage,
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    },
    vk::ImageMemoryBarrier{
      .srcAccessMask        = vk::AccessFlagBits::eTransferWrite,
      .oldLayout            = vk::ImageLayout::eTransferDstOptimal,
      .newLayout            = vk::ImageLayout::eGeneral,
      .srcQueueFamilyIndex  = sameQueue ? vk::QueueFamilyIgnored : graphicsIndex,
      .dstQueueFamilyIndex  = sameQueue ? vk::QueueFamilyIgnored : computeIndex,
      .image                = renderImage,
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    }
  };

  cmd.pipelineBarrier(
    vk::PipelineStageFlagBits::eTransfer,
    vk::PipelineStageFlagBits::eBottomOfPipe,
    {},
    nullptr,
    nullptr,
    postProcessBarriers
  );

  cmd.end();

  return imgIndex;
}

void Renderer::beginPostProcess(const VulkanContext * context, unsigned int imgIndex) {
  m_preDraw = false;

  vk::CommandBuffer& cmd = m_postProcessCmds[m_frameIndex];
  cmd.reset();
  cmd.begin(vk::CommandBufferBeginInfo{});

  auto [drawImage, drawView] = drawTarget(imgIndex);
  auto [renderImage, renderView] = renderTarget(imgIndex);

  auto [graphicsIndex, graphicsQueue] = context->graphicsQueue();
  auto [computeIndex, computeQueue] = context->computeQueue();

  if (graphicsIndex != computeIndex) {
    std::array<vk::ImageMemoryBarrier, 2> barriers{
      vk::ImageMemoryBarrier{
        .dstAccessMask        = vk::AccessFlagBits::eShaderRead,
        .oldLayout            = vk::ImageLayout::eGeneral,
        .newLayout            = vk::ImageLayout::eGeneral,
        .srcQueueFamilyIndex  = computeIndex,
        .dstQueueFamilyIndex  = graphicsIndex,
        .image                = drawImage,
        .subresourceRange = {
          .aspectMask = vk::ImageAspectFlagBits::eColor,
          .levelCount = 1,
          .layerCount = 1
        }
      },
      vk::ImageMemoryBarrier{
        .dstAccessMask        = vk::AccessFlagBits::eShaderWrite,
        .oldLayout            = vk::ImageLayout::eGeneral,
        .newLayout            = vk::ImageLayout::eGeneral,
        .srcQueueFamilyIndex  = computeIndex,
        .dstQueueFamilyIndex  = graphicsIndex,
        .image                = renderImage,
        .subresourceRange = {
          .aspectMask = vk::ImageAspectFlagBits::eColor,
          .levelCount = 1,
          .layerCount = 1
        }
      }
    };

    cmd.pipelineBarrier(
      vk::PipelineStageFlagBits::eTopOfPipe,
      vk::PipelineStageFlagBits::eComputeShader,
      {},
      nullptr,
      nullptr,
      barriers
    );
  }
}

void Renderer::endPostProcess(const VulkanContext * context, unsigned int imgIndex) {
  vk::CommandBuffer& cmd = m_postProcessCmds[m_frameIndex];

  auto [graphicsIndex, graphicsQueue] = context->graphicsQueue();
  auto [computeIndex, computeQueue] = context->computeQueue();
  bool sameQueue = graphicsIndex == computeIndex;

  auto [renderImage, renderView] = renderTarget(imgIndex);

  vk::ImageMemoryBarrier barrier{
    .srcAccessMask        = vk::AccessFlagBits::eShaderWrite,
    .oldLayout            = vk::ImageLayout::eGeneral,
    .newLayout            = vk::ImageLayout::eColorAttachmentOptimal,
    .srcQueueFamilyIndex  = sameQueue ? vk::QueueFamilyIgnored : computeIndex,
    .dstQueueFamilyIndex  = sameQueue ? vk::QueueFamilyIgnored : graphicsIndex,
    .image                = renderImage,
    .subresourceRange = {
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .levelCount = 1,
      .layerCount = 1
    }
  };

  cmd.pipelineBarrier(
    vk::PipelineStageFlagBits::eComputeShader,
    vk::PipelineStageFlagBits::eBottomOfPipe,
    {},
    nullptr,
    nullptr,
    barrier
  );

  cmd.end();
}

void Renderer::drawUI(
  const VulkanContext * context,
  unsigned int imgIndex,
  std::unordered_map<std::string, GUI>& guis
) {
  vk::CommandBuffer& cmd = m_uiCmds[m_frameIndex];
  cmd.reset();
  cmd.begin(vk::CommandBufferBeginInfo{});

  auto [graphicsIndex, graphicsQueue] = context->graphicsQueue();
  auto [computeIndex, computeQueue] = context->computeQueue();

  auto [renderImage, renderView] = renderTarget(imgIndex);

  if (graphicsIndex != computeIndex) {
    vk::ImageMemoryBarrier barrier{
      .dstAccessMask        = vk::AccessFlagBits::eColorAttachmentWrite,
      .oldLayout            = vk::ImageLayout::eColorAttachmentOptimal,
      .newLayout            = vk::ImageLayout::eColorAttachmentOptimal,
      .srcQueueFamilyIndex  = computeIndex,
      .dstQueueFamilyIndex  = graphicsIndex,
      .image                = renderImage,
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    };

    cmd.pipelineBarrier(
      vk::PipelineStageFlagBits::eTopOfPipe,
      vk::PipelineStageFlagBits::eColorAttachmentOutput,
      {},
      nullptr,
      nullptr,
      barrier
    );
  }

  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  for (auto& [label, gui] : guis)
    gui.render();

  ImGui::Render();
  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();

  ImDrawData * drawData = ImGui::GetDrawData();

  if (drawData && drawData->TotalVtxCount > 0) {
    vk::RenderingAttachmentInfo colorAttachment{
      .imageView    = renderView,
      .imageLayout  = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp       = vk::AttachmentLoadOp::eLoad,
      .storeOp      = vk::AttachmentStoreOp::eStore
    };

    cmd.beginRendering(vk::RenderingInfo{
      .renderArea           = { .extent = m_extent },
      .layerCount           = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments    = &colorAttachment
    });

    ImGui_ImplVulkan_RenderDrawData(drawData, cmd);

    cmd.endRendering();
  }

  auto [presentIndex, presentQueue] = context->presentQueue();
  bool sameQueue = graphicsIndex == presentIndex;

  vk::ImageMemoryBarrier barrier{
    .srcAccessMask        = vk::AccessFlagBits::eColorAttachmentWrite,
    .oldLayout            = vk::ImageLayout::eColorAttachmentOptimal,
    .newLayout            = vk::ImageLayout::ePresentSrcKHR,
    .srcQueueFamilyIndex  = sameQueue ? vk::QueueFamilyIgnored : graphicsIndex,
    .dstQueueFamilyIndex  = sameQueue ? vk::QueueFamilyIgnored : presentIndex,
    .image                = renderImage,
    .subresourceRange = {
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .levelCount = 1,
      .layerCount = 1
    }
  };

  cmd.pipelineBarrier(
    vk::PipelineStageFlagBits::eColorAttachmentOutput,
    vk::PipelineStageFlagBits::eBottomOfPipe,
    {},
    nullptr,
    nullptr,
    barrier
  );

  cmd.end();
}

void Renderer::submit(const VulkanContext * context, unsigned int imgIndex) {
  auto [graphicsIndex, graphicsQueue] = context->graphicsQueue();
  auto [computeIndex, computeQueue] = context->computeQueue();
  auto [presentIndex, presentQueue] = context->presentQueue();

  computeQueue.submit(vk::SubmitInfo{
    .commandBufferCount   = 1,
    .pCommandBuffers      = &m_dispatchCmds[m_frameIndex],
    .signalSemaphoreCount = 1,
    .pSignalSemaphores    = &m_dispatchSemaphores[m_frameIndex]
  });


  std::array<vk::Semaphore, 2> waitSemaphores = {
    m_dispatchSemaphores[m_frameIndex], m_imageSemaphores[m_frameIndex]
  };
  std::array<vk::PipelineStageFlags, 2> drawWaitStages = {
    vk::PipelineStageFlagBits::eVertexShader, vk::PipelineStageFlagBits::eColorAttachmentOutput
  };
  graphicsQueue.submit(vk::SubmitInfo{
    .waitSemaphoreCount   = 2,
    .pWaitSemaphores      = waitSemaphores.data(),
    .pWaitDstStageMask    = drawWaitStages.data(),
    .commandBufferCount   = 1,
    .pCommandBuffers      = &m_drawCmds[m_frameIndex],
    .signalSemaphoreCount = 1,
    .pSignalSemaphores    = &m_drawSemaphores[m_frameIndex]
  });

  vk::PipelineStageFlags postProcessWaitStage = vk::PipelineStageFlagBits::eComputeShader;
  computeQueue.submit(vk::SubmitInfo{
    .waitSemaphoreCount   = 1,
    .pWaitSemaphores      = &m_drawSemaphores[m_frameIndex],
    .pWaitDstStageMask    = &postProcessWaitStage,
    .commandBufferCount   = 1,
    .pCommandBuffers      = &m_postProcessCmds[m_frameIndex],
    .signalSemaphoreCount = 1,
    .pSignalSemaphores    = &m_postProcessSemaphores[m_frameIndex]
  });

  vk::PipelineStageFlags uiWaitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  graphicsQueue.submit(vk::SubmitInfo{
    .waitSemaphoreCount   = 1,
    .pWaitSemaphores      = &m_postProcessSemaphores[m_frameIndex],
    .pWaitDstStageMask    = &uiWaitStage,
    .commandBufferCount   = 1,
    .pCommandBuffers      = &m_uiCmds[m_frameIndex],
    .signalSemaphoreCount = 1,
    .pSignalSemaphores    = &m_uiSemaphores[m_frameIndex]
  }, m_flightFences[m_frameIndex]);

  if (presentQueue.presentKHR(vk::PresentInfoKHR{
    .waitSemaphoreCount = 1,
    .pWaitSemaphores    = &m_uiSemaphores[m_frameIndex],
    .swapchainCount     = 1,
    .pSwapchains        = &m_swapchain,
    .pImageIndices      = &imgIndex
  }) != vk::Result::eSuccess) {
    Log::runtime_error("Failed to present image");
  }

  m_frameIndex = (m_frameIndex + 1) % m_flightFrames;
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
