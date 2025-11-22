#pragma once

#include <vulkan/vulkan_raii.hpp>

class GLFWwindow;

namespace groot {

class VulkanContext {
  vk::raii::Instance m_instance = nullptr;
  vk::raii::SurfaceKHR m_surface = nullptr;
  vk::raii::PhysicalDevice m_gpu = nullptr;
  vk::raii::Device m_device = nullptr;

  unsigned int m_queueFamilyIndices = 0;
  vk::raii::Queue m_graphicsQueue = nullptr;
  vk::raii::Queue m_presentQueue = nullptr;
  vk::raii::Queue m_transferQueue = nullptr;
  vk::raii::Queue m_computeQueue = nullptr;

  public:
    VulkanContext(const std::string&, const unsigned int&);
    VulkanContext(const VulkanContext&) = delete;
    VulkanContext(VulkanContext&&) = delete;

    ~VulkanContext() = default;

    VulkanContext& operator=(const VulkanContext&) = delete;
    VulkanContext& operator=(VulkanContext&&) = delete;

    void printInfo() const;
    const vk::raii::Instance& instance() const;
    const vk::raii::PhysicalDevice& gpu() const;
    const vk::raii::Device& device() const;

    void createSurface(GLFWwindow *);
    void chooseGPU(const unsigned int&, const std::vector<const char *>&);
    void createDevice(std::vector<const char *>&);

  private:
    unsigned int getQueueFamilyIndices() const;
    std::vector<vk::DeviceQueueCreateInfo> getQueueCreateInfos(const float&) const;
};

} // namespace groot