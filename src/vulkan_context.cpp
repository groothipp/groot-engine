#include "src/include/log.hpp"
#include "src/include/vulkan_context.hpp"

#include <vulkan/vulkan_beta.h>
#include <GLFW/glfw3.h>

#include <set>

#define GRAPHICS_SHIFT  0
#define PRESENT_SHIFT   8
#define COMPUTE_SHIFT   16
#define TRANSFER_SHIFT  24

namespace groot {

VulkanContext::VulkanContext(const std::string& applicationName, const unsigned int& applicationVersion) {
  vk::ApplicationInfo applicationInfo {
    .pApplicationName   = applicationName.c_str(),
    .applicationVersion = applicationVersion,
    .pEngineName        = "Groot Engine",
    .engineVersion      = VK_MAKE_API_VERSION(0, 0, 14, 0),
    .apiVersion         = VK_MAKE_API_VERSION(0, 1, 4, 328)
  };

  unsigned int extensionCount = 0;
  const char ** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
  if (glfwExtensions == nullptr)
    Log::runtime_error("glfw is not initialized");

  std::vector<const char *> extensions(glfwExtensions, glfwExtensions + extensionCount);

  vk::InstanceCreateFlags flags;
  for (const auto& extension : vk::enumerateInstanceExtensionProperties()) {
    if (std::strcmp(extension.extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) != 0) continue;

    extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;

    break;
  }

  const char * validationLayer = "VK_LAYER_KHRONOS_validation";

  vk::InstanceCreateInfo createInfo {
    .flags                    = flags,
    .pApplicationInfo         = &applicationInfo,
    .enabledLayerCount        = 1,
    .ppEnabledLayerNames      = &validationLayer,
    .enabledExtensionCount    = static_cast<unsigned int>(extensions.size()),
    .ppEnabledExtensionNames  = extensions.data()
  };

  m_instance = vk::raii::Context().createInstance(createInfo);
  if (!*m_instance)
    Log::runtime_error("failed to create vulkan instance");
}

void VulkanContext::printInfo() const {
  vk::PhysicalDeviceProperties properties = m_gpu.getProperties();

  unsigned int variant = (properties.apiVersion >> 29) & 0b111;
  unsigned int major = (properties.apiVersion >> 22) & 0b1111111;
  unsigned int minor = (properties.apiVersion >> 12) & 0b1111111111;
  unsigned int patch = properties.apiVersion & 0xFFF;
  std::string versionString =
    std::to_string(variant) + '.' + std::to_string(major)
    + '.' + std::to_string(minor) + '.' + std::to_string(patch);

  Log::generic(
    "using Vulkan " + versionString
    + " on " + std::string(properties.deviceName)
    + " (" + vk::to_string(properties.deviceType) + ')'
  );
}

const vk::raii::Instance& VulkanContext::instance() const {
  return m_instance;
}

const vk::raii::PhysicalDevice& VulkanContext::gpu() const {
  return m_gpu;
}

const vk::raii::Device& VulkanContext::device() const {
  return m_device;
}

void VulkanContext::createSurface(GLFWwindow * window) {
  vk::raii::SurfaceKHR::CType rawSurface = nullptr;
  if (glfwCreateWindowSurface(*m_instance, window, nullptr, &rawSurface) != VK_SUCCESS)
    Log::runtime_error("failed to create surface");

  m_surface = vk::raii::SurfaceKHR(m_instance, rawSurface);
  if (!*m_surface)
    Log::runtime_error("failed to initialize raii surface");
}

void VulkanContext::chooseGPU(const unsigned int& gpuIndex, const std::vector<const char *>& requiredExtensions) {
  auto gpus = m_instance.enumeratePhysicalDevices();
  if (gpuIndex >= gpus.size())
    Log::out_of_range("GPU index out of range");

  std::set<std::string> availableExtensions;
  for (const auto& extension : gpus[gpuIndex].enumerateDeviceExtensionProperties())
    availableExtensions.emplace(extension.extensionName);

  for (const auto& extension : requiredExtensions) {
    if (!availableExtensions.contains(extension))
      Log::runtime_error("GPU does not support the required extensions");
  }

  m_gpu = gpus[gpuIndex];
  m_queueFamilyIndices = getQueueFamilyIndices();
}

void VulkanContext::createDevice(std::vector<const char *>& extensions) {
  float queuePriority = 1.0f;
  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = getQueueCreateInfos(queuePriority);

  for (const auto& extension : m_gpu.enumerateDeviceExtensionProperties()) {
    if (strcmp(extension.extensionName, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME) == 0) {
      extensions.emplace_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
      break;
    }
  }

  vk::PhysicalDeviceFeatures features{};

  vk::DeviceCreateInfo deviceCreateInfo{
    .pNext                    = nullptr,
    .queueCreateInfoCount     = static_cast<unsigned int>(queueCreateInfos.size()),
    .pQueueCreateInfos        = queueCreateInfos.data(),
    .enabledExtensionCount    = static_cast<unsigned int>(extensions.size()),
    .ppEnabledExtensionNames  = extensions.data(),
    .pEnabledFeatures         = &features
  };

  m_device = m_gpu.createDevice(deviceCreateInfo);
  if (!*m_device)
    Log::runtime_error("failed to create device");

  m_graphicsQueue = m_device.getQueue((m_queueFamilyIndices >> GRAPHICS_SHIFT) & 0xFF, 0);
  m_presentQueue = m_device.getQueue((m_queueFamilyIndices >> PRESENT_SHIFT) & 0xFF, 0);
  m_computeQueue = m_device.getQueue((m_queueFamilyIndices >> COMPUTE_SHIFT) & 0xFF, 0);
  m_transferQueue = m_device.getQueue((m_queueFamilyIndices >> TRANSFER_SHIFT) & 0xFF, 0);
}

unsigned int VulkanContext::getQueueFamilyIndices() const {
  unsigned int queueFamilyIndex = 0, queueFamilyIndices = 0xFFFFFFFF;
  for (const auto& queueFamily : m_gpu.getQueueFamilyProperties()) {
    if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
      if (((queueFamilyIndices >> GRAPHICS_SHIFT) & 0xFF) == 0xFF) {
        queueFamilyIndices &= ~(0xFF << GRAPHICS_SHIFT);
        queueFamilyIndices |= queueFamilyIndex << GRAPHICS_SHIFT;

        queueFamilyIndices &= ~(0xFF << COMPUTE_SHIFT);
        queueFamilyIndices |= queueFamilyIndex << COMPUTE_SHIFT;

        queueFamilyIndices &= ~(0xFF << TRANSFER_SHIFT);
        queueFamilyIndices |= queueFamilyIndex << TRANSFER_SHIFT;
      }
    }
    else if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
      queueFamilyIndices &= ~(0xFF << COMPUTE_SHIFT);
      queueFamilyIndices |= queueFamilyIndex << COMPUTE_SHIFT;
    }
    else if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
      queueFamilyIndices &= ~(0xFF << TRANSFER_SHIFT);
      queueFamilyIndices |= queueFamilyIndex << TRANSFER_SHIFT;
    }

    if (m_gpu.getSurfaceSupportKHR(queueFamilyIndex, m_surface) &&
        ((queueFamilyIndices >> PRESENT_SHIFT) & 0xFF) == 0xFF) {
      queueFamilyIndices &= ~(0xFF << PRESENT_SHIFT);
      queueFamilyIndices |= queueFamilyIndex << PRESENT_SHIFT;
    }

    ++queueFamilyIndex;
  }

  return queueFamilyIndices;
}

std::vector<vk::DeviceQueueCreateInfo> VulkanContext::getQueueCreateInfos(const float& queuePriority) const {
  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<unsigned int> usedIndices;

  for (unsigned int i = 0; i < 4; ++i) {
    unsigned int index = (m_queueFamilyIndices >> 8 * i) & 0xFF;
    if (usedIndices.contains(index)) continue;

    queueCreateInfos.emplace_back(vk::DeviceQueueCreateInfo{
      .queueFamilyIndex = index,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority
    });

    usedIndices.emplace(index);
  }

  return queueCreateInfos;
}

} // namespace groot