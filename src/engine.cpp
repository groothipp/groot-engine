#include "src/include/allocator.hpp"
#include "src/include/engine.hpp"
#include "src/include/log.hpp"
#include "src/include/rid.hpp"
#include "src/include/vulkan_context.hpp"

#include <GLFW/glfw3.h>

namespace groot {

Engine::Engine(Settings settings) : m_settings(settings) {
  if (!glfwInit())
    Log::runtime_error("failed to initialize GLFW");

  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  auto [width, height] = m_settings.window_size;
  m_window = glfwCreateWindow(width, height, m_settings.window_title.c_str(), nullptr, nullptr);
  if (m_window == nullptr)
    Log::runtime_error("failed to create GLFW window");

  m_context = new VulkanContext(m_settings.application_name, m_settings.application_version);

  m_context->createSurface(m_window);

  std::vector<const char *> requiredExtensions = {};
  m_context->chooseGPU(m_settings.gpu_index, requiredExtensions);
  m_context->createDevice(requiredExtensions);

  m_context->printInfo();

  m_allocator = new Allocator(m_context, m_context->gpu().getProperties().apiVersion);
}

Engine::~Engine() {
  delete m_allocator;
  delete m_context;

  glfwDestroyWindow(m_window);
  glfwTerminate();
}

void Engine::run() {
  while (!glfwWindowShouldClose(m_window)) {
    glfwPollEvents();
  }
}

RID Engine::create_uniform_buffer(unsigned int size) {
  vk::Buffer buffer = m_allocator->allocateBuffer(vk::BufferCreateInfo{
    .size         = size,
    .usage        = vk::BufferUsageFlagBits::eUniformBuffer,
    .sharingMode  = vk::SharingMode::eExclusive
  });

  m_buffers[m_nextRID] = reinterpret_cast<unsigned long>(static_cast<VkBuffer>(buffer));
  return RID(m_nextRID++);
}

RID Engine::create_storage_buffer(unsigned int size) {
  vk::Buffer buffer = m_allocator->allocateBuffer(vk::BufferCreateInfo{
    .size         = size,
    .usage        = vk::BufferUsageFlagBits::eStorageBuffer,
    .sharingMode  = vk::SharingMode::eExclusive
  });

  m_buffers[m_nextRID] = reinterpret_cast<unsigned long>(static_cast<VkBuffer>(buffer));
  return RID(m_nextRID++);
}

} // namespace groot