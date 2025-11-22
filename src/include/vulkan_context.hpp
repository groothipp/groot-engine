#pragma once

#include <vulkan/vulkan.hpp>

class GLFWwindow;

namespace groot {

class VulkanContext {
  vk::Instance m_instance = nullptr;
  vk::SurfaceKHR m_surface = nullptr;
  vk::PhysicalDevice m_gpu = nullptr;
  vk::Device m_device = nullptr;

  bool m_tesselationSupport = false;

  unsigned int m_queueFamilyIndices = 0;
  vk::Queue m_graphicsQueue = nullptr;
  vk::Queue m_presentQueue = nullptr;
  vk::Queue m_transferQueue = nullptr;
  vk::Queue m_computeQueue = nullptr;

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
    const bool& supportsTesselation() const;

    void createSurface(GLFWwindow *);
    void chooseGPU(const unsigned int&, const std::vector<const char *>&);
    void createDevice(std::vector<const char *>&);

  private:
    unsigned int getQueueFamilyIndices() const;
    std::vector<vk::DeviceQueueCreateInfo> getQueueCreateInfos(const float&) const;
};

} // namespace groot