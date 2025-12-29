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
class GUI;

class Renderer {
  vk::Extent2D m_extent;
  vk::SurfaceFormatKHR m_colorFormat;
  vk::Format m_depthFormat;
  vk::PresentModeKHR m_presentMode;
  vk::ClearColorValue m_clearColor{};

  vk::SwapchainKHR m_swapchain = nullptr;
  std::vector<vk::Image> m_images;
  std::vector<vk::ImageView> m_views;
  std::vector<vk::Image> m_drawImages;
  std::vector<vk::ImageView> m_drawViews;
  vk::Image m_depthImage = nullptr;
  vk::ImageView m_depthView = nullptr;;

  vk::DescriptorPool m_guiDescriptorPool = nullptr;

  std::vector<vk::CommandBuffer> m_dispatchCmds;
  std::vector<vk::CommandBuffer> m_drawCmds;
  std::vector<vk::CommandBuffer> m_postProcessCmds;
  std::vector<vk::CommandBuffer> m_uiCmds;

  std::vector<vk::Fence> m_flightFences;
  std::vector<vk::Semaphore> m_imageSemaphores;
  std::vector<vk::Semaphore> m_dispatchSemaphores;
  std::vector<vk::Semaphore> m_drawSemaphores;
  std::vector<vk::Semaphore> m_postProcessSemaphores;
  std::vector<vk::Semaphore> m_uiSemaphores;

  std::vector<std::vector<RID>> m_postProcessResources;

  unsigned int m_flightFrames = 0;
  unsigned int m_frameIndex = 0;
  bool m_preDraw = false;

  public:
    Renderer(GLFWwindow *, const VulkanContext *, Allocator *, Settings&);
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    ~Renderer() = default;

    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    vk::Format depthFormat() const;
    std::pair<unsigned int, unsigned int> extent() const;
    std::pair<const vk::Image&, const vk::ImageView&> renderTarget(unsigned int) const;
    std::pair<const vk::Image&, const vk::ImageView&> drawTarget(unsigned int) const;
    unsigned int frameIndex() const;

    void destroy(const VulkanContext *, Allocator *);

    void prepFrame(const VulkanContext *, std::unordered_map<RID, unsigned long, RID::Hash>&);
    void dispatch(const VulkanContext *, const ComputeCommand&, const std::unordered_map<RID, unsigned long, RID::Hash>&);
    void beginDispatch(const VulkanContext *, const std::set<unsigned long>&);
    void endDispatch(const VulkanContext *, const std::set<unsigned long>&);
    unsigned int draw(const VulkanContext *, const std::set<unsigned long>&, const std::unordered_map<RID, unsigned long, RID::Hash>&, const std::set<Object>&);
    void beginPostProcess(const VulkanContext *, unsigned int);
    void endPostProcess(const VulkanContext *, unsigned int);
    void drawUI(const VulkanContext *, unsigned int, std::unordered_map<std::string, GUI>&);
    void submit(const VulkanContext *, unsigned int);

  private:
    vk::SurfaceFormatKHR checkFormat(const VulkanContext *, Settings&) const;
    vk::Format getDepthFormat(const VulkanContext *) const;
    vk::PresentModeKHR checkPresentMode(const VulkanContext *, Settings&) const;
    vk::Extent2D getExtent(GLFWwindow *, const VulkanContext *) const;
};

} // namespace groot
