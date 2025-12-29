#pragma once

#include "src/include/enums.hpp"
#include "src/include/gui.hpp"
#include "src/include/rid.hpp"
#include "src/include/structs.hpp"

#include <string>
#include <unordered_map>
#include <functional>
#include <set>
#include <vector>

class GLFWwindow;

namespace groot {

class Allocator;
class InputManager;
class Object;
class Renderer;
class ShaderCompiler;
class VulkanContext;

class alignas(64) Engine {
  Settings m_settings;
  GLFWwindow * m_window = nullptr;
  VulkanContext * m_context = nullptr;
  Allocator * m_allocator = nullptr;
  ShaderCompiler * m_compiler = nullptr;
  Renderer * m_renderer = nullptr;
  InputManager * m_inputManager = nullptr;

  unsigned long m_nextRID = 1;
  std::unordered_map<RID, unsigned long, RID::Hash> m_resources;
  std::set<RID> m_busySamplers;
  std::set<unsigned long> m_storageTextures;

  ImageHandle * m_drawOutput = nullptr;
  ImageHandle * m_renderTarget = nullptr;

  std::set<Object> m_scene;
  vec3 m_cameraEye = vec3(0.0f, 0.0f, 2.0f);
  vec3 m_cameraTarget = vec3(0.0f);

  std::unordered_map<std::string, GUI> m_guis;

  double m_frameTime = 0.0;
  double m_time = 0.0;

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
    void close_window() const;
    bool is_pressed(Key) const;
    bool is_pressed(MouseButton) const;
    bool just_pressed(Key) const;
    bool just_pressed(MouseButton) const;
    bool just_released(Key) const;
    bool just_released(MouseButton) const;
    vec2 mouse_pos() const;
    std::tuple<vec3, vec3, vec3> camera_basis() const;
    void capture_cursor() const;
    void release_cursor() const;

    RID render_target();
    void translate_camera(const vec3&);
    void rotate_camera(float, float);
    void run(std::function<void(double)> pre_draw = [](double){}, std::function<void(double)> post_draw = [](double){});

    RID create_uniform_buffer(unsigned int);
    RID create_storage_buffer(unsigned int);
    void destroy_buffer(RID&);

    template <typename T>
    inline std::vector<T> read_buffer(const RID& rid) const {
      auto [size, data] = readBufferRaw(rid);
      if (size == 0) return {};

      std::vector<T> out(static_cast<T *>(data), static_cast<T *>(data) + (size / sizeof(T)));
      delete [] static_cast<char *>(data);

      return out;
    }

    template <typename T>
    inline T read_buffer(const RID& rid, const T& error) const {
      auto [size, data] = readBufferRaw(rid);
      if (size == 0) return error;

      T out = *static_cast<T *>(data);
      delete [] static_cast<char *>(data);

      return out;
    }

    template <typename T>
    inline void write_buffer(const RID& rid, const std::vector<T>& data) const {
      writeBufferRaw(rid, sizeof(T) * data.size(), data.data());
    }

    template <typename T>
    inline void write_buffer(const RID& rid, const T& data) const {
      writeBufferRaw(rid, sizeof(T), &data);
    }

    RID create_sampler(const SamplerSettings&);
    void destroy_sampler(RID&);

    RID create_storage_image(unsigned int, unsigned int, ImageType type = ImageType::two_dim, Format format = Format::rgba16_unorm);
    RID create_texture(const std::string&, const RID&);
    RID create_storage_texture(unsigned int, unsigned int, const RID&, ImageType type = ImageType::two_dim, Format format = Format::rgba16_unorm);
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

    void dispatch(const ComputeCommand&);

    void add_to_scene(Object&);
    void remove_from_scene(Object&);

    void add_gui(const std::string&, GUI&&);
    void toggle_gui(const std::string&);

    template<typename T>
    inline std::unique_ptr<T>& get_gui_element(const std::string& gui, const std::string& element) {
      return m_guis.at(gui).get_element<T>(element);
    }

  private:
    void updateTimes();
    std::pair<unsigned int, void *> readBufferRaw(const RID&) const;
    void writeBufferRaw(const RID&, std::size_t, const void *) const;
};

} // namespace groot
