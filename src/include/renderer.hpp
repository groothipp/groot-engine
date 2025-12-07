#pragma once

#include "src/include/structs.hpp"

#include <vulkan/vulkan.hpp>

#include <set>
#include <unordered_map>
#include <vector>

class GLFWwindow;

namespace groot {

class Allocator;
class VulkanContext;
class Object;

class Renderer {
  vk::Extent2D m_extent;
  vk::SurfaceFormatKHR m_colorFormat;
  vk::Format m_depthFormat;
  vk::PresentModeKHR m_presentMode;
  vk::ClearColorValue m_clearColor{};

  vk::SwapchainKHR m_swapchain = nullptr;
  std::vector<vk::Image> m_images;
  std::vector<vk::ImageView> m_views;
  vk::Image m_depthImage = nullptr;
  vk::ImageView m_depthView = nullptr;

  std::vector<vk::CommandBuffer> m_cmds;
  std::vector<vk::Fence> m_fences;
  std::vector<vk::Semaphore> m_imageSemaphores;
  std::vector<vk::Semaphore> m_renderSemaphores;

  unsigned int m_flightFrames = 0;
  unsigned int m_frameIndex = 0;

  public:
    Renderer(GLFWwindow *, const VulkanContext *, Allocator *, Settings&);
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    ~Renderer() = default;

    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    vk::Format depthFormat() const;
    std::pair<unsigned int, unsigned int> extent() const;
    unsigned int prepFrame(const vk::Device&) const;
    const vk::CommandBuffer& renderBuffer() const;

    void render(
      const VulkanContext *,
      const std::set<Object>&,
      const std::unordered_map<RID, unsigned long, RID::Hash>&,
      unsigned int
    );
    void destroy(const VulkanContext *, Allocator *);

  private:
    vk::SurfaceFormatKHR checkFormat(const VulkanContext *, Settings&) const;
    vk::Format getDepthFormat(const VulkanContext *) const;
    vk::PresentModeKHR checkPresentMode(const VulkanContext *, Settings&) const;
    vk::Extent2D getExtent(GLFWwindow *, const VulkanContext *) const;
};

} // namespace groot