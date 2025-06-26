#pragma once

#include "src/include/materials.hpp"
#include "src/include/objects.hpp"
#include "src/include/renderer.hpp"
#include "src/include/vkcontext.hpp"

#include <GLFW/glfw3.h>

#include <chrono>
#include <string>

#define ge_make_version(major, minor, patch) (unsigned int)( \
  ( (major) << 24 ) | \
  ( (minor) << 16 ) | \
  (patch) \
)

namespace ge {

enum BufferMode {
  SingleBuffer = 1,
  DoubleBuffer = 2,
  TripleBuffer = 3
};

struct Settings {
  unsigned int gpu_index = 0;
  unsigned int window_width = 1280;
  unsigned int window_height = 720;
  std::string window_title = "Groot Engine";
  std::string application_name = "Groot Engine Application";
  unsigned int application_version = ge_make_version(1, 0, 0);
  BufferMode buffer_mode = TripleBuffer;
  vk::Format format = vk::Format::eB8G8R8A8Srgb;
  vk::Format depth_format = vk::Format::eD32Sfloat;
  vk::ColorSpaceKHR color_space = vk::ColorSpaceKHR::eSrgbNonlinear;
  vk::PresentModeKHR present_mode = vk::PresentModeKHR::eMailbox;
  vk::Extent2D extent = vk::Extent2D{ 1280, 720 };
  std::array<float, 4> background_color = { 0.0f, 0.0f, 0.0f, 1.0f };
  double time_step = 1 / 60.0f;
};

class Engine {
  friend class Allocator;
  friend class Material;
  friend class MaterialManager;
  friend class Object;
  friend class ObjectManager;
  friend class Renderer;
  friend class VulkanContext;

  public:
    Engine();
    Engine(const Settings&);
    Engine(Settings&&);
    Engine(Engine&) = delete;
    Engine(Engine&&) = delete;

    ~Engine();

    Engine& operator = (Engine&) = delete;
    Engine& operator = (Engine&&) = delete;

    void add_material(std::string, MaterialManager::Builder&);
    transform add_object(std::string, std::string, const Transform& t = Transform());
    void run();

    template <typename Func>
    requires std::invocable<Func>
    inline void run(Func&& code) {
      load();

      std::chrono::time_point start = std::chrono::high_resolution_clock::now();
      std::chrono::time_point now = start;

      while (!shouldClose()) {
        updateTimes();
        pollEvents();

        while (m_accumulator >= m_settings.time_step) {
          code();
          m_accumulator -= m_settings.time_step;
        }

        m_renderer.render(*this);
        batchUpdates();
      }

      m_context.device().waitIdle();
    }

    template <typename Func>
    requires std::invocable<Func, double>
    inline void run(Func&& code) {
      load();

      std::chrono::time_point start = std::chrono::high_resolution_clock::now();
      std::chrono::time_point now = start;

      while (!shouldClose()) {
        updateTimes();
        pollEvents();

        while (m_accumulator >= m_settings.time_step) {
          code(m_settings.time_step);
          m_accumulator -= m_settings.time_step;
        }

        m_renderer.render(*this);
        batchUpdates();
      }

      m_context.device().waitIdle();
    }

  private:
    bool shouldClose() const;
    void pollEvents() const;
    vk::raii::CommandBuffers getCmds(QueueFamilyType, unsigned int) const;

    void initialize();
    void createWindow();
    void createSurface();
    void createCommandPools();
    void load();
    void updateTimes();
    void batchUpdates();

  private:
    Settings m_settings;
    double m_frameTime = 0.0;
    double m_currTime = 0.0;
    double m_accumulator = 0.0;

    VulkanContext m_context;
    MaterialManager m_materials;
    ObjectManager m_objects;

    GLFWwindow * m_window = nullptr;
    vk::raii::SurfaceKHR m_surface = nullptr;

    std::map<QueueFamilyType, vk::raii::CommandPool> m_commandPools;

    Renderer m_renderer;
};

} // namespace ge