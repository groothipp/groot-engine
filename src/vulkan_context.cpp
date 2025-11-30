#include "src/include/log.hpp"
#include "vulkan/vulkan.hpp"
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
    .engineVersion      = VK_MAKE_API_VERSION(0, GROOT_VERSION_MAJOR, GROOT_VERSION_MINOR, GROOT_VERSION_PATCH),
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

    Log::generic("enabled portability enumeration");
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

  m_instance = vk::createInstance(createInfo);
  if (!m_instance)
    Log::runtime_error("failed to create instance");
}

VulkanContext::~VulkanContext() {
  m_device.destroyCommandPool(m_transferCmdPool);
  m_device.destroyCommandPool(m_computeCmdPool);
  m_device.destroy();
  m_instance.destroySurfaceKHR(m_surface);
  m_instance.destroy();
}

void VulkanContext::printInfo() const {
  vk::PhysicalDeviceProperties properties = m_gpu.getProperties();

  Log::generic(std::format("using Vulkan {}.{}.{} on {} ({})\n",
    VK_VERSION_MAJOR(properties.apiVersion),
    VK_VERSION_MINOR(properties.apiVersion),
    VK_VERSION_PATCH(properties.apiVersion),
    std::string(properties.deviceName),
    vk::to_string(properties.deviceType)
  ));
}

const vk::Instance& VulkanContext::instance() const {
  return m_instance;
}

const vk::PhysicalDevice& VulkanContext::gpu() const {
  return m_gpu;
}

const vk::Device& VulkanContext::device() const {
  return m_device;
}

bool VulkanContext::supportsTesselation() const {
  return m_gpu.getFeatures().tessellationShader;
}

bool VulkanContext::supportsNonSolidMesh() const {
  return m_gpu.getFeatures().fillModeNonSolid;
}

bool VulkanContext::supportsAnisotropy() const {
  return m_gpu.getFeatures().samplerAnisotropy;
}

vk::CommandBuffer VulkanContext::beginTransfer() const {
  vk::CommandBuffer cmdBuf = m_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{
    .commandPool        = m_transferCmdPool,
    .level              = vk::CommandBufferLevel::ePrimary,
    .commandBufferCount = 1
  })[0];

  cmdBuf.begin(vk::CommandBufferBeginInfo{});
  return cmdBuf;
}

void VulkanContext::endTransfer(const vk::CommandBuffer& cmdBuf) const {
  cmdBuf.end();

  vk::SubmitInfo submitInfo{
    .commandBufferCount = 1,
    .pCommandBuffers    = &cmdBuf
  };

  vk::Fence fence = m_device.createFence({});
  m_transferQueue.submit(submitInfo, fence);
  vk::Result res = m_device.waitForFences(fence, true, 1000000000);

  m_device.freeCommandBuffers(m_transferCmdPool, cmdBuf);
  m_device.destroyFence(fence);

  if (res != vk::Result::eSuccess)
    Log::runtime_error("hung waiting for transfer");
}

vk::CommandBuffer VulkanContext::beginDispatch() const {
  vk::CommandBuffer cmdBuf = m_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{
    .commandPool        = m_computeCmdPool,
    .level              = vk::CommandBufferLevel::ePrimary,
    .commandBufferCount = 1
  })[0];

  cmdBuf.begin(vk::CommandBufferBeginInfo{});
  return cmdBuf;
}

void VulkanContext::endDispatch(const vk::CommandBuffer& cmdBuf) const {
  cmdBuf.end();

  vk::SubmitInfo submitInfo{
    .commandBufferCount = 1,
    .pCommandBuffers    = &cmdBuf
  };

  vk::Fence fence = m_device.createFence({});
  m_computeQueue.submit(submitInfo, fence);
  vk::Result res = m_device.waitForFences(fence, true, 1000000000);

  m_device.freeCommandBuffers(m_computeCmdPool, cmdBuf);
  m_device.destroyFence(fence);

  if (res != vk::Result::eSuccess)
    Log::runtime_error("hung waiting for compute dispatch");
}

void VulkanContext::createSurface(GLFWwindow * window) {
  VkSurfaceKHR rawSurface = nullptr;
  if (glfwCreateWindowSurface(m_instance, window, nullptr, &rawSurface) != VK_SUCCESS)
    Log::runtime_error("failed to create surface");

  m_surface = rawSurface;
}

void VulkanContext::chooseGPU(const unsigned int& gpuIndex, const std::vector<const char *>& requiredExtensions) {
  auto gpus = m_instance.enumeratePhysicalDevices();
  if (gpuIndex >= gpus.size())
    Log::out_of_range("GPU index out of range");

  std::set<std::string> availableExtensions;
  for (const auto& extension : gpus[gpuIndex].enumerateDeviceExtensionProperties())
    availableExtensions.emplace(extension.extensionName);

  std::string extensionErrors = "";
  for (const auto& extension : requiredExtensions) {
    if (!availableExtensions.contains(extension))
      extensionErrors += std::format("\n\t{}", std::string(extension));
  }
  if (extensionErrors != "")
    Log::runtime_error(std::format("GPU does not support the following extensions:{}", extensionErrors));

  if (!gpus[gpuIndex].getFeatures().tessellationShader)
    Log::warn("GPU does not support tesselation shaders");

  if (!gpus[gpuIndex].getFeatures().fillModeNonSolid)
    Log::warn("GPU does not support non-solid mesh types");

  if (!gpus[gpuIndex].getFeatures().samplerAnisotropy)
    Log::warn("GPU does not support anisotropic filtering");

  m_gpu = gpus[gpuIndex];
  m_queueFamilyIndices = getQueueFamilyIndices();
}

void VulkanContext::createDevice(std::vector<const char *>& extensions) {
  float queuePriority = 1.0f;
  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = getQueueCreateInfos(queuePriority);

  for (const auto& extension : m_gpu.enumerateDeviceExtensionProperties()) {
    if (strcmp(extension.extensionName, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME) != 0) continue;

    extensions.emplace_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    Log::generic("enabled portability subset");
    break;
  }

  vk::PhysicalDeviceFeatures features{
    .tessellationShader = m_gpu.getFeatures().tessellationShader,
    .fillModeNonSolid   = m_gpu.getFeatures().fillModeNonSolid,
    .samplerAnisotropy  = m_gpu.getFeatures().samplerAnisotropy
  };

  vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature{
    .dynamicRendering = true
  };

  vk::DeviceCreateInfo deviceCreateInfo{
    .pNext                    = &dynamicRenderingFeature,
    .queueCreateInfoCount     = static_cast<unsigned int>(queueCreateInfos.size()),
    .pQueueCreateInfos        = queueCreateInfos.data(),
    .enabledExtensionCount    = static_cast<unsigned int>(extensions.size()),
    .ppEnabledExtensionNames  = extensions.data(),
    .pEnabledFeatures         = &features
  };

  m_device = m_gpu.createDevice(deviceCreateInfo);
  if (!m_device)
    Log::runtime_error("failed to create device");

  m_graphicsQueue = m_device.getQueue((m_queueFamilyIndices >> GRAPHICS_SHIFT) & 0xFF, 0);
  m_presentQueue = m_device.getQueue((m_queueFamilyIndices >> PRESENT_SHIFT) & 0xFF, 0);
  m_computeQueue = m_device.getQueue((m_queueFamilyIndices >> COMPUTE_SHIFT) & 0xFF, 0);
  m_transferQueue = m_device.getQueue((m_queueFamilyIndices >> TRANSFER_SHIFT) & 0xFF, 0);
}

void VulkanContext::createCommandPools() {
  m_transferCmdPool = m_device.createCommandPool(vk::CommandPoolCreateInfo{
    .flags            = vk::CommandPoolCreateFlagBits::eTransient,
    .queueFamilyIndex = (m_queueFamilyIndices >> TRANSFER_SHIFT) & 0xFF
  });

  m_computeCmdPool = m_device.createCommandPool(vk::CommandPoolCreateInfo{
    .flags            = vk::CommandPoolCreateFlagBits::eTransient,
    .queueFamilyIndex = (m_queueFamilyIndices >> COMPUTE_SHIFT) & 0xFF
  });
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