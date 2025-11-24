#pragma once

#include "src/include/structs.hpp"
#include "vulkan/vulkan_core.h"

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

#include <unordered_map>

namespace groot {

class VulkanContext;

class Allocator {
  VmaAllocator m_allocator = nullptr;
  std::unordered_map<VkBuffer, VmaAllocation, VkBufferHash> m_buffers;
  std::unordered_map<VkImage, VmaAllocation, VkImageHash> m_images;

  public:
    explicit Allocator(const VulkanContext *, unsigned int);
    Allocator(const Allocator&) = delete;
    Allocator(Allocator&&) = delete;

    ~Allocator();

    Allocator& operator=(const Allocator&) = delete;
    Allocator& operator=(Allocator&&) = delete;

    vk::Buffer allocateBuffer(const vk::BufferCreateInfo&, VmaMemoryUsage memoryusage = VMA_MEMORY_USAGE_AUTO, VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    void * mapBuffer(const vk::Buffer&);
    void unmapBuffer(const vk::Buffer&);
    void destroyBuffer(const vk::Buffer&);

    vk::Image allocateImage(const vk::ImageCreateInfo&, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);
    void destroyImage(const vk::Image&);
};

} // namespace groot