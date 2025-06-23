#pragma once

#include "src/include/vkcontext.hpp"

#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_beta.h>

#include <map>
#include <tuple>
#include <vector>

namespace ge {

class Allocator {
  using BufferOutput = std::tuple<
    vk::raii::DeviceMemory,
    std::vector<vk::raii::Buffer>,
    std::vector<unsigned int>,
    unsigned int
  >;

  using CommandOutput = std::map<QueueFamilyType, vk::raii::CommandPool>;

  using DepthOutput = std::tuple<
    vk::raii::DeviceMemory,
    vk::raii::Image,
    vk::raii::ImageView
  >;

  using FenceOutput = std::vector<vk::raii::Fence>;
  using SemaphoreOutput = std::vector<vk::raii::Semaphore>;

  using DescriptorOutput = std::pair<
    vk::raii::DescriptorPool,
    vk::raii::DescriptorSets
  >;

  public:
    Allocator() = delete;
    Allocator(Allocator&) = delete;
    Allocator(Allocator&&) = delete;

    ~Allocator() = default;

    Allocator& operator=(Allocator&) = delete;
    Allocator& operator=(Allocator&&) = delete;

    static BufferOutput bufferPool(const Engine&, const std::vector<vk::BufferCreateInfo>&, vk::MemoryPropertyFlags);
    static CommandOutput commandPools(const Engine&);
    static DepthOutput depthResources(const Engine&);
    static FenceOutput fences(const Engine&, unsigned int, bool signaled = false);
    static SemaphoreOutput semaphores(const Engine&, unsigned int);
    static DescriptorOutput descriptorPool(const Engine&, const std::vector<vk::raii::DescriptorSetLayout>&,
      unsigned int storageCount,
      unsigned int uniformCount,
      unsigned int imageCount,
      unsigned int samplerCount
    );

  private:
    static vk::raii::DeviceMemory allocate(
      const Engine&,
      const unsigned int&,
      const unsigned int&,
      const vk::MemoryPropertyFlags&
    );
};

} // namespace ge