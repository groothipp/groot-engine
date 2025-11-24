#pragma once

#include "src/include/enums.hpp"
#include "src/include/rid.hpp"
#include "src/include/structs.hpp"

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <vulkan/vulkan.hpp>

class GLFWwindow;

namespace groot {

class Allocator;
class VulkanContext;
class ShaderCompiler;

class Engine {
  Settings m_settings = Settings{};
  GLFWwindow * m_window = nullptr;
  VulkanContext * m_context = nullptr;
  Allocator * m_allocator = nullptr;
  ShaderCompiler * m_compiler = nullptr;

  double m_frameTime = 0.0;
  double m_time = 0.0;
  double m_accumulator = 0.0;

  unsigned long m_nextRID = 0;
  std::unordered_map<RID, unsigned long, RID::Hash> m_resources;

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

    template <typename T>
    inline std::vector<T> read_buffer(const RID& rid) const {
      auto [size, data] = read_buffer_raw(rid);
      if (size == 0) return {};

      std::vector<T> out(static_cast<T *>(data), static_cast<T *>(data) + (size / sizeof(T)));
      delete [] static_cast<char *>(data);

      return out;
    }

    template <typename T>
    inline T read_buffer(const RID& rid, const T& error) const {
      auto [size, data] = read_buffer_raw(rid);
      if (size == 0) return error;

      T out = *static_cast<T *>(data);
      delete [] static_cast<char *>(data);

      return out;
    }

    template <typename T>
    inline void write_buffer(const RID& rid, const std::vector<T>& data) const {
      write_buffer_raw(rid, sizeof(T) * data.size(), data.data());
    }

    template <typename T>
    inline void write_buffer(const RID& rid, const T& data) const {
      write_buffer_raw(rid, sizeof(T), &data);
    }

    RID create_storage_image(unsigned int, unsigned int, Format);
    void destroy_image(RID&);

    RID compile_shader(ShaderType type, const std::string&);
    void destroy_shader(RID&);

    RID create_descriptor_set(const std::vector<RID>&);
    void destroy_descriptor_set(RID&);

    RID create_compute_pipeline(const RID&, const RID&);
    RID create_graphics_pipeline(const GraphicsPipelineShaders&, const RID&, const GraphicsPipelineSettings&);
    void destroy_pipeline(RID&);

  private:
    void updateTimes();
    std::pair<unsigned int, void *> read_buffer_raw(const RID&) const;
    void write_buffer_raw(const RID&, std::size_t, const void *) const;
};

} // namespace groot