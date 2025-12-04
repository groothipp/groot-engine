#include "src/include/log.hpp"
#include "src/include/renderer.hpp"
#include "src/include/structs.hpp"
#include "src/include/vulkan_context.hpp"

#include <GLFW/glfw3.h>

#include <set>

namespace groot {

Renderer::Renderer(GLFWwindow * window, const VulkanContext * context, Settings& settings) {
  m_colorFormat = checkFormat(context, settings);
  m_depthFormat = getDepthFormat(context);
  m_presentMode = checkPresentMode(context, settings);
  m_extent = getExtent(window, context);

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
      .image = image,
      .viewType = vk::ImageViewType::e2D,
      .format   = m_colorFormat.format,
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    }));
  }
}

std::pair<unsigned int, unsigned int> Renderer::extent() const {
  return std::make_pair(m_extent.width, m_extent.height);
}

vk::Format Renderer::depthFormat() const {
  return m_depthFormat;
}

void Renderer::destroy(const vk::Device& device) {
  for (const auto& view : m_views)
    device.destroyImageView(view);
  device.destroySwapchainKHR(m_swapchain);
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
  std::set<vk::Format> formats;
  for (const auto& format : context->gpu().getSurfaceFormatsKHR(context->surface())) {
    formats.emplace(format.format);
  }

  if (formats.contains(vk::Format::eD32Sfloat))
    return vk::Format::eD32Sfloat;

  if (formats.contains(vk::Format::eD24UnormS8Uint))
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