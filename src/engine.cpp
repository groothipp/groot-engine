#include "src/include/allocator.hpp"
#include "src/include/engine.hpp"
#include "src/include/log.hpp"
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

  std::vector<const char *> requiredExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
  };

  m_context->chooseGPU(m_settings.gpu_index, requiredExtensions);
  m_context->createDevice(requiredExtensions);
  m_context->printInfo();

  m_allocator = new Allocator(m_context, m_context->gpu().getProperties().apiVersion);

  m_compiler = new ShaderCompiler();
}

Engine::~Engine() {
  for (auto& [rid, handle] : m_resources) {
    switch (rid.m_type) {
      case ResourceType::Buffer:
        m_allocator->destroyBuffer(reinterpret_cast<VkBuffer>(handle));
        break;
      case ResourceType::Shader:
        m_context->device().destroyShaderModule(reinterpret_cast<VkShaderModule>(handle));
        break;
      default: break;
    }
  }

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

  RID rid = RID(m_nextRID++, ResourceType::Buffer);
  m_resources[rid] = reinterpret_cast<unsigned long>(static_cast<VkBuffer>(buffer));

  return rid;
}

RID Engine::create_storage_buffer(unsigned int size) {
  vk::Buffer buffer = m_allocator->allocateBuffer(vk::BufferCreateInfo{
    .size         = size,
    .usage        = vk::BufferUsageFlagBits::eStorageBuffer,
    .sharingMode  = vk::SharingMode::eExclusive
  });

  RID rid = RID(m_nextRID++, ResourceType::Buffer);
  m_resources[rid] = reinterpret_cast<unsigned long>(static_cast<VkBuffer>(buffer));

  return rid;
}

void Engine::update_buffer(const RID& rid, std::size_t size, void * data) const {
  if (!rid.is_valid()) {
    Log::warn("tried to update buffer with an invalid RID");
    return;
  }

  if (rid.m_type != ResourceType::Buffer) {
    Log::warn("tried to update buffer of a non-buffer resource");
    return;
  }

  vk::Buffer buffer = reinterpret_cast<VkBuffer>(m_resources.at(rid));

  void * map = m_allocator->mapBuffer(buffer);
  std::memcpy(map, data, size);
  m_allocator->unmapBuffer(buffer);
}

void Engine::destroy_buffer(RID& rid) {
  if (!rid.is_valid()) {
    Log::warn("tried to destroy a buffer with an invalid RID");
    return;
  }

  if (rid.m_type != ResourceType::Buffer) {
    Log::warn("tried to destroy buffer of a non-buffer resource");
    return;
  }

  vk::Buffer buffer = reinterpret_cast<VkBuffer>(m_resources.at(rid));
  m_allocator->destroyBuffer(buffer);
  m_resources.erase(rid);

  rid.invalidate();
}

RID Engine::compile_shader(ShaderType type, const std::string& path) {
  std::vector<unsigned int> code = m_compiler->compileShader(type, path);
  if (code.empty()) return RID();

  vk::ShaderModuleCreateInfo createInfo{
    .codeSize = code.size() * sizeof(unsigned int),
    .pCode = code.data()
  };

  vk::ShaderModule module = m_context->device().createShaderModule(createInfo);

  Log::generic(std::format("compiled {}", path));

  RID rid = RID(m_nextRID++, ResourceType::Shader);
  m_resources[rid] = reinterpret_cast<unsigned long>(static_cast<VkShaderModule>(module));

  return rid;
}

void Engine::destroy_shader(RID& rid) {
  if (!rid.is_valid()) {
    Log::warn("tried to destroy shader with an invalid RID");
    return;
  }

  if (rid.m_type != ResourceType::Shader) {
    Log::warn("tried to destroy shader of a non-shader resource");
    return;
  }

  vk::ShaderModule module = reinterpret_cast<VkShaderModule>(m_resources.at(rid));
  m_context->device().destroyShaderModule(module);
  m_resources.erase(rid);

  rid.invalidate();
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