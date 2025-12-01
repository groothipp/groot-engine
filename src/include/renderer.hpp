#pragma once

#include "src/include/structs.hpp"
#include "src/include/vulkan_context.hpp"

#include <vulkan/vulkan.hpp>

#include <vector>

namespace groot {

class VulkanContext;

class Renderer {
  vk::SwapchainKHR m_swapchain = nullptr;
  std::vector<vk::Image> m_images;
  std::vector<vk::ImageView> m_views;

  public:
    Renderer(const VulkanContext *, Settings&);
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    ~Renderer();

    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

  private:
    vk::SurfaceFormatKHR checkFormat(const VulkanContext *, Settings&) const;
    vk::Format checkDepthFormat(vk::Format) const;
    vk::PresentModeKHR choosePresentMode(const VulkanContext *) const;
    vk::Extent2D getExtent() const;
};

} // namespace groot