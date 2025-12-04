#include "src/include/allocator.hpp"
#include "src/include/log.hpp"
#include "src/include/vulkan_context.hpp"

namespace groot {

Allocator::Allocator(const VulkanContext * context, unsigned int apiVersion) {
  VmaAllocatorCreateInfo createInfo{
    .physicalDevice   = context->gpu(),
    .device           = context->device(),
    .instance         = context->instance(),
    .vulkanApiVersion = apiVersion
  };

  if (vmaCreateAllocator(&createInfo, &m_allocator) != VK_SUCCESS)
    Log::runtime_error("failed to create allocator");
}

Allocator::~Allocator() {
  for (auto [buffer, allocation] : m_buffers)
    vmaDestroyBuffer(m_allocator, buffer, allocation);

  for (auto [image, allocation] : m_images)
    vmaDestroyImage(m_allocator, image, allocation);

  if (m_allocator)
    vmaDestroyAllocator(m_allocator);
}

vk::Buffer Allocator::allocateBuffer(const vk::BufferCreateInfo& bufferCreateInfo, VmaMemoryUsage usage, VmaAllocationCreateFlags flags) {
  VmaAllocationCreateInfo allocationCreateInfo{
    .flags = flags,
    .usage = usage
  };

  VkBuffer buffer = nullptr;
  VmaAllocation allocation = nullptr;
  if (vmaCreateBuffer(
    m_allocator,
    reinterpret_cast<const VkBufferCreateInfo *>(&bufferCreateInfo),
    &allocationCreateInfo,
    &buffer,
    &allocation,
    nullptr
  ) != VK_SUCCESS) {
    Log::runtime_error("failed to create buffer");
  }

  m_buffers[buffer] = allocation;
  return buffer;
}

void * Allocator::mapBuffer(const vk::Buffer& buffer) {
  VmaAllocation alloc = m_buffers.at(buffer);

  void * map = nullptr;
  if (vmaMapMemory(m_allocator, alloc, &map) != VK_SUCCESS)
    Log::runtime_error("failed to map buffer memory");

  return map;
}

void Allocator::unmapBuffer(const vk::Buffer& buffer) {
  VmaAllocation alloc = m_buffers.at(buffer);
  vmaUnmapMemory(m_allocator, alloc);
}

void Allocator::destroyBuffer(const vk::Buffer& buffer) {
  VmaAllocation alloc = m_buffers.at(buffer);
  vmaDestroyBuffer(m_allocator, buffer, alloc);
  m_buffers.erase(buffer);
}

unsigned int Allocator::bufferSize(const vk::Buffer& buffer) const {
  VmaAllocation allocation = m_buffers.at(buffer);

  VmaAllocationInfo info;
  vmaGetAllocationInfo(m_allocator, allocation, &info);

  return info.size;
}

vk::Image Allocator::allocateImage(const vk::ImageCreateInfo& createInfo, VmaMemoryUsage memoryUsage) {
  VmaAllocationCreateInfo allocationCreateInfo{
    .usage          = memoryUsage,
    .requiredFlags  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
  };

  VkImage image;
  VmaAllocation allocation;

  if (vmaCreateImage(
    m_allocator,
    reinterpret_cast<const VkImageCreateInfo *>(&createInfo),
    &allocationCreateInfo,
    &image,
    &allocation,
    nullptr
  ) != VK_SUCCESS) {
    Log::runtime_error("failed to allocate image");
  }

  m_images[image] = allocation;
  return image;
}

void Allocator::destroyImage(const vk::Image& image) {
  VmaAllocation alloc = m_images[image];
  vmaDestroyImage(m_allocator, image, alloc);
  m_images.erase(image);
}

} // namespace groot