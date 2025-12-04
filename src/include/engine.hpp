#pragma once

#include "src/include/enums.hpp"
#include "src/include/rid.hpp"
#include "src/include/structs.hpp"

#include <string>
#include <unordered_map>
#include <functional>
#include <queue>
#include <set>
#include <vector>

class GLFWwindow;

namespace groot {

class Allocator;
class VulkanContext;
class ShaderCompiler;
class Renderer;
class Object;

class alignas(64) Engine {
  Settings m_settings;
  GLFWwindow * m_window = nullptr;
  VulkanContext * m_context = nullptr;
  Allocator * m_allocator = nullptr;
  ShaderCompiler * m_compiler = nullptr;
  Renderer * m_renderer = nullptr;

  unsigned long m_nextRID = 0;
  std::unordered_map<RID, unsigned long, RID::Hash> m_resources;
  std::set<RID> m_busySamplers;
  std::set<unsigned long> m_storageTextures;

  std::queue<ComputeCommand> m_computeCmds;

  std::set<RID> m_scene;
  mat4 m_cameraView = mat4::view(vec3(0.0f, 0.0f, -2.0f), vec3(0.0f), vec3(0.0f, -1.0f, 0.0f));
  mat4 m_cameraProjection = mat4::identity();

  double m_frameTime = 0.0;
  double m_time = 0.0;
  double m_accumulator = 0.0;

  public:
    explicit Engine(const Settings& settings = Settings{});
    Engine(const Engine&) = delete;
    Engine(Engine&&) = delete;

    ~Engine();

    Engine& operator=(const Engine&) = delete;
    Engine& operator=(Engine&&) = delete;

    mat4 camera_view() const;
    mat4 camera_projection() const;
    std::pair<unsigned int, unsigned int> viewport_dims() const;

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

    RID create_sampler(const SamplerSettings&);
    void destroy_sampler(RID&);

    RID create_storage_image(unsigned int, unsigned int, Format format = Format::rgba16_unorm);
    RID create_texture(const std::string&, const RID&);
    RID create_storage_texture(unsigned int, unsigned int, const RID&, Format format = Format::rgba16_unorm);
    void destroy_image(RID&);

    RID compile_shader(ShaderType type, const std::string&);
    void destroy_shader(RID&);

    RID create_descriptor_set(const std::vector<RID>&);
    void destroy_descriptor_set(RID&);

    RID create_compute_pipeline(const RID&, const RID&);
    RID create_graphics_pipeline(const GraphicsPipelineShaders&, const RID&, const GraphicsPipelineSettings&);
    void destroy_pipeline(RID&);

    RID load_mesh(const std::string&);
    void destroy_mesh(RID&);

    void compute_command(const ComputeCommand&);
    void dispatch();

    void add_to_scene(Object&);
    void remove_from_scene(Object&);

  private:
    void updateTimes();
    std::pair<unsigned int, void *> read_buffer_raw(const RID&) const;
    void write_buffer_raw(const RID&, std::size_t, const void *) const;
};

} // namespace groot