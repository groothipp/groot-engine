#include "src/include/engine.hpp"

#include <chrono>
#include <future>

namespace ge {

Engine::Engine() {
  initialize();
}

Engine::Engine(const Settings& settings) : m_settings(settings) {
  initialize();
}

Engine::Engine(Settings&& settings) : m_settings(settings) {
  initialize();
}

Engine::~Engine() {
  glfwDestroyWindow(m_window);
  glfwTerminate();
}

void Engine::add_material(std::string tag, MaterialManager::Builder& builder) {
  if (m_materials.exists(tag))
    throw std::runtime_error("groot-engine: material '" + tag + "' already exists");
  m_materials.add(tag, builder);
}

transform Engine::add_object(std::string material, std::string path, const Transform& transform) {
  if (!m_materials.exists(material))
    throw std::runtime_error("groot-engine: material '" + material + "' does not exist");
  return m_objects.add(*this, material, path, transform);
}

void Engine::run() {
  run([](){});
}

bool Engine::shouldClose() const {
  return glfwWindowShouldClose(m_window);
}

void Engine::pollEvents() const {
  glfwPollEvents();
}

vk::raii::CommandBuffers Engine::getCmds(QueueFamilyType type, unsigned int count) const {
  return vk::raii::CommandBuffers(m_context.device(), vk::CommandBufferAllocateInfo{
    .commandPool        = m_commandPools.at(type),
    .level              = vk::CommandBufferLevel::ePrimary,
    .commandBufferCount = count
  });
}

void Engine::initialize() {
  createWindow();
  m_context.initialize(*this);
  createSurface();
  m_context.initialize(*this);

  createCommandPools();

  m_renderer.initialize(*this);
}

void Engine::createWindow() {
  if (glfwInit() != GLFW_TRUE)
    throw std::runtime_error("groot-engine: failed to initialize glfw");

  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  m_window = glfwCreateWindow(
    m_settings.window_width,
    m_settings.window_height,
    m_settings.window_title.c_str(),
    nullptr,
    nullptr
  );

  if (m_window == nullptr)
    throw std::runtime_error("groot-engine: failed to create window");
}

void Engine::createSurface() {
  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(*m_context.instance(), m_window, nullptr, &surface) != VK_SUCCESS)
    throw std::runtime_error("groot-engine: failed to create window surface");

  m_surface = vk::raii::SurfaceKHR(m_context.instance(), surface);
  if (*m_surface == nullptr)
    throw std::runtime_error("groot-engine: failed to create window surface");
}

void Engine::createCommandPools() {
  m_commandPools.emplace(QueueFamilyType::Main, m_context.device().createCommandPool(vk::CommandPoolCreateInfo{
    .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
    .queueFamilyIndex = m_context.queueFamily(QueueFamilyType::Main).index
  }));

  m_commandPools.emplace(QueueFamilyType::Compute, m_context.device().createCommandPool(vk::CommandPoolCreateInfo{
    .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
    .queueFamilyIndex = m_context.queueFamily(QueueFamilyType::Compute).index
  }));

  m_commandPools.emplace(QueueFamilyType::Transfer, m_context.device().createCommandPool(vk::CommandPoolCreateInfo{
    .flags            = vk::CommandPoolCreateFlagBits::eTransient,
    .queueFamilyIndex = m_context.queueFamily(QueueFamilyType::Transfer).index
  }));
}

void Engine::load() {
  std::future materialThread = std::async(std::launch::async,
    [this](){ this->m_materials.load(*this, this->m_objects.transforms()); }
  );

  std::future objectThread = std::async(std::launch::async,
    [this]() { this->m_objects.load(*this); }
  );

  materialThread.get();
  objectThread.get();
}

void Engine::updateTimes() {
  static std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  std::chrono::time_point now = std::chrono::high_resolution_clock::now();
  double time = std::chrono::duration(now - start).count();

  m_frameTime = std::min(time - m_currTime, 0.25);
  m_currTime = time;
  m_accumulator += m_frameTime;
}

void Engine::batchUpdates() {
  m_objects.update(m_frameTime);
  m_materials.update(m_renderer.frameIndex(), m_objects.transforms());
}

} // namespace ge