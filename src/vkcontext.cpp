#include "src/include/engine.hpp"
#include "vulkan/vulkan_core.h"
#include "src/include/vkcontext.hpp"

#include <set>

namespace ge {

const vk::raii::Instance& VulkanContext::instance() const {
  return m_instance;
}

const vk::raii::PhysicalDevice& VulkanContext::gpu() const {
  return m_gpu;
}

const vk::raii::Device& VulkanContext::device() const {
  return m_device;
}

const QueueFamily& VulkanContext::queueFamily(QueueFamilyType type) const {
  if (m_queueFamilies.contains(type)) return m_queueFamilies.at(type);

  if (type == QueueFamilyType::Compute || type == QueueFamilyType::Transfer)
    type = QueueFamilyType::Async;

  if (m_queueFamilies.contains(type)) return m_queueFamilies.at(type);

  return m_queueFamilies.at(QueueFamilyType::Main);
}

void VulkanContext::initialize(const Engine& engine) {
  if (*engine.m_surface == nullptr) {
    createInstance(engine);
    return;
  }

  if (*m_instance == nullptr)
    throw std::runtime_error("groot-engine: tried to initialize vulkan objects without establishing vulkan instance");

  std::vector<const char *> extensions{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
  };

  chooseGPU(engine, extensions);
  createDevice(engine, extensions);

  for (auto& [type, family] : m_queueFamilies)
    family.queue = m_device.getQueue(family.index, 0);
}

void VulkanContext::createInstance(const Engine& engine) {
  vk::ApplicationInfo appInfo{
    .pApplicationName   = engine.m_settings.application_name.c_str(),
    .applicationVersion = engine.m_settings.application_version,
    .pEngineName        = "Groot Engine",
    .engineVersion      = ge_engine_version,
    .apiVersion         = ge_vulkan_version
  };

  const char * validationLayer = "VK_LAYER_KHRONOS_validation";

  unsigned int extensionCount = 0;
  const char ** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
  if (glfwExtensions == nullptr)
    throw std::runtime_error("groot-engine: failed to get glfw extensions");

  std::vector<const char *> extensions(glfwExtensions, glfwExtensions + extensionCount);

  vk::InstanceCreateFlags flags;
  for (const auto& extension : vk::enumerateInstanceExtensionProperties()) {
    if (std::string(extension.extensionName) != VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) continue;

    extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;

    break;
  }

  m_instance = vk::raii::Context().createInstance(vk::InstanceCreateInfo{
    .flags                    = flags,
    .pApplicationInfo         = &appInfo,
    .enabledLayerCount        = 1,
    .ppEnabledLayerNames      = &validationLayer,
    .enabledExtensionCount    = static_cast<unsigned int>(extensions.size()),
    .ppEnabledExtensionNames  = extensions.data()
  });
}

void VulkanContext::chooseGPU(const Engine& engine, const std::vector<const char *>& extensions) {
  std::vector<vk::raii::PhysicalDevice> gpus = m_instance.enumeratePhysicalDevices();
  if (engine.m_settings.gpu_index > gpus.size() - 1)
    throw std::runtime_error("groot-engine: gpu not found");

  m_gpu = gpus[engine.m_settings.gpu_index];
  getQueueFamilies(engine);

  bool supported = true;
  std::vector<std::string> reasons;

  if (!m_queueFamilies.contains(QueueFamilyType::Main)) {
    reasons.emplace_back("\t- main queue family missing");
    supported = false;
  }

  std::set<std::string> availableExtensions;
  for (const auto& extension : m_gpu.enumerateDeviceExtensionProperties())
    availableExtensions.emplace(extension.extensionName);

  for (const auto& extension : extensions) {
    if (!availableExtensions.contains(extension)) {
      reasons.emplace_back("\t- " + std::string(extension) + " extension not supported");
      supported = false;
    }
  }

  vk::PhysicalDeviceFeatures features = m_gpu.getFeatures();
  if (!features.samplerAnisotropy) {
    reasons.emplace_back("\t- sampler anisotropy feature not supported");
    supported = false;
  }

  if (!features.multiDrawIndirect) {
    reasons.emplace_back("\t- multi draw indirect feature not supported");
    supported = false;
  }

  if (!supported) {
    std::string error = "groot-engine: gpu " + std::to_string(engine.m_settings.gpu_index) + " not suitable:\n";
    for (const auto& reason : reasons)
      error = error + reason + '\n';

    throw std::runtime_error(error);
  }
}

void VulkanContext::getQueueFamilies(const Engine& engine) {
  unsigned int index = 0;
  for (auto& property : m_gpu.getQueueFamilyProperties()) {
    if (property.queueFlags & vk::QueueFlagBits::eSparseBinding) {
      ++index;
      continue;
    }

    if ((property.queueFlags & static_cast<vk::QueueFlagBits>(QueueFamilyType::Main))
          && !m_queueFamilies.contains(QueueFamilyType::Main)
          && m_gpu.getSurfaceSupportKHR(index, engine.m_surface)
    ) {
      m_queueFamilies.emplace(QueueFamilyType::Main, QueueFamily{ .index = index++ });
      continue;
    }

    if ((property.queueFlags & static_cast<vk::QueueFlagBits>(QueueFamilyType::Async))
          && !m_queueFamilies.contains(QueueFamilyType::Async)
    ) {
      m_queueFamilies.emplace(QueueFamilyType::Async, QueueFamily{ .index = index++ });
      continue;
    }

    if ((property.queueFlags & static_cast<vk::QueueFlagBits>(QueueFamilyType::Compute))
          && !m_queueFamilies.contains(QueueFamilyType::Compute)
    ) {
      m_queueFamilies.emplace(QueueFamilyType::Compute, QueueFamily{ .index = index++ });
      continue;
    }

    if ((property.queueFlags & static_cast<vk::QueueFlagBits>(QueueFamilyType::Transfer))
          && m_queueFamilies.contains(QueueFamilyType::Transfer)
    ) {
      m_queueFamilies.emplace(QueueFamilyType::Transfer, QueueFamily{ .index = index++ });
      continue;
    }

    ++index;
  }
}

void VulkanContext::createDevice(const Engine& engine, std::vector<const char *>& extensions) {
  std::vector<vk::DeviceQueueCreateInfo> ci_queues;
  float priority = 1.0f;
  for (auto& [type, family] : m_queueFamilies) {
    ci_queues.emplace_back(vk::DeviceQueueCreateInfo{
      .queueFamilyIndex = family.index,
      .queueCount       = 1,
      .pQueuePriorities = &priority
    });
  }

  for (const auto& extension : m_gpu.enumerateDeviceExtensionProperties()) {
    if (std::string(extension.extensionName) != VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME) continue;

    extensions.emplace_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    break;
  }

  vk::PhysicalDeviceFeatures features{
    .multiDrawIndirect  = true,
    .samplerAnisotropy  = true
  };

  vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynRender{
    .dynamicRendering = vk::True
  };

  vk::DeviceCreateInfo ci_device{
    .pNext                    = &dynRender,
    .queueCreateInfoCount     = static_cast<unsigned int>(ci_queues.size()),
    .pQueueCreateInfos        = ci_queues.data(),
    .enabledExtensionCount    = static_cast<unsigned int>(extensions.size()),
    .ppEnabledExtensionNames  = extensions.data(),
    .pEnabledFeatures         = &features
  };

  m_device = m_gpu.createDevice(ci_device);
}

} // namespace ge