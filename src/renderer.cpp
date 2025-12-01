#include "src/include/log.hpp"
#include "src/include/renderer.hpp"
#include "src/include/structs.hpp"
#include "src/include/vulkan_context.hpp"

namespace groot {

Renderer::Renderer(const VulkanContext * context, Settings& settings) {
  vk::SurfaceFormatKHR format = checkFormat(context, settings);
  vk::PresentModeKHR presentMode = choosePresentMode(context);
  vk::Extent2D extent = getExtent();

  vk::SwapchainCreateInfoKHR createInfo{
    .surface          = context->surface(),
    .minImageCount    = 0,
    .imageFormat      = format.format,
    .imageColorSpace  = format.colorSpace,
    .imageExtent      = extent,
    .imageArrayLayers = 1,
    .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
    .presentMode      = presentMode
  };

  m_swapchain = context->device().createSwapchainKHR(createInfo);
  m_images = context->device().getSwapchainImagesKHR(m_swapchain);

  for (const auto& image : m_images) {
    m_views.emplace_back(context->device().createImageView(vk::ImageViewCreateInfo{
      .image = image,
      .viewType = vk::ImageViewType::e2D,
      .format   = format.format,
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    }));
  }
}

vk::SurfaceFormatKHR Renderer::checkFormat(const VulkanContext * context, Settings& settings) const {
  std::vector<vk::SurfaceFormatKHR> formats = context->gpu().getSurfaceFormatsKHR(context->surface());
  for (const auto& format : formats) {
    if (format.format == static_cast<vk::Format>(settings.color_format) && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
      return format;
  }

  vk::SurfaceFormatKHR format = formats.front();
  Log::warn(std::format("chosen formats not compatible. using {}/{} instead",
    vk::to_string(format.format), vk::to_string(format.colorSpace)
  ));

  settings.color_format = static_cast<Format>(format.format);

  return format;
}

vk::PresentModeKHR Renderer::choosePresentMode(const VulkanContext * context) const {
  std::vector<vk::PresentModeKHR> modes = context->gpu().getSurfacePresentModesKHR(context->surface());
  for (const auto& mode : modes) {
    if (mode == vk::PresentModeKHR::eMailbox)
      return mode;
  }

  Log::warn("mailbox mode not available. using FIFO instead");
  return vk::PresentModeKHR::eFifo;
}

} // namespace groot