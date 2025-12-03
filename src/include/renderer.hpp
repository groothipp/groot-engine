#pragma once

#include "src/include/structs.hpp"

#include <vulkan/vulkan.hpp>

#include <vector>

class GLFWwindow;

namespace groot {

class VulkanContext;

class Renderer {
  vk::SwapchainKHR m_swapchain = nullptr;
  std::vector<vk::Image> m_images;
  std::vector<vk::ImageView> m_views;

  vk::Extent2D m_extent;
  vk::SurfaceFormatKHR m_colorFormat;
  vk::Format m_depthFormat;
  vk::PresentModeKHR m_presentMode;

  public:
    Renderer(GLFWwindow *, const VulkanContext *, Settings&);
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    ~Renderer() = default;

    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    vk::Format depthFormat() const;

    void destroy(const vk::Device&);

  private:
    vk::SurfaceFormatKHR checkFormat(const VulkanContext *, Settings&) const;
    vk::Format getDepthFormat(const VulkanContext *) const;
    vk::PresentModeKHR checkPresentMode(const VulkanContext *, Settings&) const;
    vk::Extent2D getExtent(GLFWwindow *, const VulkanContext *) const;
};

} // namespace groot