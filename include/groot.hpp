#pragma once

#include <functional>
#include <string>
#include <unordered_map>

class GLFWwindow;

namespace groot {

class Allocator;
class VulkanContext;

struct Settings {
  std::string application_name = "Groot Engine Application";
  unsigned int application_version = 1;
  std::pair<unsigned int, unsigned int> window_size = std::make_pair(1280, 720);
  std::string window_title = "Groot Engine Application";
  unsigned int gpu_index = 0;
  double time_step = 1.0 / 60.0;
};

class RID {
  friend class Engine;

  unsigned long m_id = ~(0x0);

  public:
    RID() = default;
    RID(const RID&) = default;
    RID(RID&&) = default;

    ~RID() = default;

    RID& operator=(const RID&) = default;
    RID& operator=(RID&&) = default;

    const unsigned long& operator*() const;

    bool is_valid() const;

  private:
    explicit RID(unsigned long);

    void invalidate();
};

class Engine {
  Settings m_settings;
  GLFWwindow * m_window = nullptr;
  VulkanContext * m_context = nullptr;
  Allocator * m_allocator = nullptr;

  double m_frameTime = 0.0;
  double m_time = 0.0;
  double m_accumulator = 0.0;

  unsigned long m_nextRID = 0;
  std::unordered_map<unsigned long, unsigned long> m_buffers;

  public:
    explicit Engine(Settings settings = Settings{});
    Engine(const Engine&) = delete;
    Engine(Engine&&) = delete;

    ~Engine();

    Engine& operator=(const Engine&) = delete;
    Engine& operator=(Engine&&) = delete;

    void run(std::function<void(double)> code = [](double){});

    RID create_uniform_buffer(unsigned int);
    RID create_storage_buffer(unsigned int);
    void destroy_buffer(RID&);

    void update_buffer(const RID&, std::size_t, void *) const;

  private:
    void updateTimes();
};

} // namespace groot