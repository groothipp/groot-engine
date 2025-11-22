#include "src/include/allocator.hpp"
#include "src/include/engine.hpp"
#include "src/include/log.hpp"
#include "src/include/rid.hpp"
#include "src/include/vulkan_context.hpp"

#include <GLFW/glfw3.h>

#include <chrono>

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

void Engine::run(std::function<void(double)> code) {
  while (!glfwWindowShouldClose(m_window)) {
    updateTimes();
    glfwPollEvents();

    while (m_accumulator >= m_settings.time_step) {
      code(m_settings.time_step);
      m_accumulator -= m_settings.time_step;
    }
  }
}

RID Engine::create_uniform_buffer(unsigned int size) {
  vk::Buffer buffer = m_allocator->allocateBuffer(vk::BufferCreateInfo{
    .size         = size,
    .usage        = vk::BufferUsageFlagBits::eUniformBuffer,
    .sharingMode  = vk::SharingMode::eExclusive
  });

  m_resources[m_nextRID] = reinterpret_cast<unsigned long>(static_cast<VkBuffer>(buffer));
  return RID(m_nextRID++);
}

RID Engine::create_storage_buffer(unsigned int size) {
  vk::Buffer buffer = m_allocator->allocateBuffer(vk::BufferCreateInfo{
    .size         = size,
    .usage        = vk::BufferUsageFlagBits::eStorageBuffer,
    .sharingMode  = vk::SharingMode::eExclusive
  });

  m_resources[m_nextRID] = reinterpret_cast<unsigned long>(static_cast<VkBuffer>(buffer));
  return RID(m_nextRID++);
}

void Engine::destroy_buffer(RID& rid) {
  if (!rid.is_valid()) {
    Log::warn("tried to destroy a buffer with an invalid RID");
    return;
  }

  vk::Buffer buffer = reinterpret_cast<VkBuffer>(m_resources.at(*rid));

  m_allocator->destroyBuffer(buffer);
  m_resources.erase(*rid);

  rid.invalidate();
}

void Engine::update_buffer(const RID& rid, std::size_t size, void * data) const {
  if (!rid.is_valid()) {
    Log::warn("tried to update buffer with an invalid RID");
    return;
  }

  vk::Buffer buffer = reinterpret_cast<VkBuffer>(m_resources.at(*rid));

  void * map = m_allocator->mapBuffer(buffer);
  std::memcpy(map, data, size);
  m_allocator->unmapBuffer(buffer);
}

void Engine::updateTimes() {
  static std::chrono::time_point start = std::chrono::high_resolution_clock::now();
  std::chrono::time_point now = std::chrono::high_resolution_clock::now();

  double time = std::chrono::duration(now - start).count();

  m_frameTime = std::min(time - m_time, 0.25);
  m_time = time;
  m_accumulator += m_frameTime;
}

} // namespace groot