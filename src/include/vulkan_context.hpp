#pragma once

#include "src/include/structs.hpp"

#include <vulkan/vulkan.hpp>

class GLFWwindow;

namespace groot {

class VulkanContext {
  vk::Instance m_instance = nullptr;
  vk::SurfaceKHR m_surface = nullptr;
  vk::PhysicalDevice m_gpu = nullptr;
  vk::Device m_device = nullptr;

  unsigned int m_queueFamilyIndices = 0;
  vk::Queue m_graphicsQueue = nullptr;
  vk::Queue m_presentQueue = nullptr;
  vk::Queue m_transferQueue = nullptr;
  vk::Queue m_computeQueue = nullptr;

  vk::CommandPool m_transferCmdPool = nullptr;
  vk::CommandPool m_computeCmdPool = nullptr;
  vk::CommandPool m_graphicsCmdPool = nullptr;

  public:
    VulkanContext(const std::string&, const unsigned int&);
    VulkanContext(const VulkanContext&) = delete;
    VulkanContext(VulkanContext&&) = delete;

    ~VulkanContext();

    VulkanContext& operator=(const VulkanContext&) = delete;
    VulkanContext& operator=(VulkanContext&&) = delete;

    void printInfo() const;
    const vk::Instance& instance() const;
    const vk::PhysicalDevice& gpu() const;
    const vk::Device& device() const;
    const vk::SurfaceKHR& surface() const;
    bool supportsTesselation() const;
    bool supportsNonSolidMesh() const;
    bool supportsAnisotropy() const;
    vk::CommandBuffer beginTransfer() const;
    void endTransfer(const vk::CommandBuffer&) const;
    vk::CommandBuffer beginDispatch() const;
    void endDispatch(const vk::CommandBuffer&) const;
    std::vector<vk::CommandBuffer> createRenderBuffers(unsigned int) const;
    std::vector<vk::Fence> createFlightFences(unsigned int) const;
    std::vector<vk::Semaphore> createRenderSemaphores(unsigned int) const;
    void destroyRenderBuffers(std::vector<vk::CommandBuffer>&) const;
    void submitRender(const RenderInfo&) const;

    void createSurface(GLFWwindow *);
    void chooseGPU(const unsigned int&, const std::vector<const char *>&);
    void createDevice(std::vector<const char *>&);
    void createCommandPools();

  private:
    unsigned int getQueueFamilyIndices() const;
    std::vector<vk::DeviceQueueCreateInfo> getQueueCreateInfos(const float&) const;
};

} // namespace groot