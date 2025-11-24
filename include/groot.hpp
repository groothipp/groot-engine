#pragma once

#include <functional>
#include <string>
#include <unordered_map>

class GLFWwindow;

namespace groot {

class Allocator;
class ShaderCompiler;
class VulkanContext;

enum ShaderType {
  Vertex,
  Fragment,
  TesselationControl,
  TesselationEvaluation,
  Compute
};

enum ResourceType {
  Invalid,
  Shader,
  Pipeline,
  DescriptorSet,
  UniformBuffer,
  StorageBuffer,
  Image
};

enum class Format {
  undefined           = 0,
  r8_unorm            = 9,
  r8_snorm            = 10,
  r8_uint             = 13,
  r8_sint             = 14,
  rg8_unorm           = 16,
  rg8_snorm           = 17,
  rg8_uint            = 20,
  rg8_sint            = 21,
  rgba8_unorm         = 37,
  rgba8_snorm         = 38,
  rgba8_uint          = 41,
  rgba8_sint          = 42,
  rgba8_srgb          = 43,
  bgra8_unorm         = 44,
  bgra8_srgb          = 50,
  r16_unorm           = 70,
  r16_snorm           = 71,
  r16_uint            = 74,
  r16_sint            = 75,
  r16_sfloat          = 76,
  rg16_unorm          = 77,
  rg16_snorm          = 78,
  rg16_uint           = 81,
  rg16_sint           = 82,
  rg16_sfloat         = 83,
  rgba16_unorm        = 91,
  rgba16_snorm        = 92,
  rgba16_uint         = 95,
  rgba16_sint         = 96,
  rgba16_sfloat       = 97,
  r32_uint            = 98,
  r32_sint            = 99,
  r32_sfloat          = 100,
  rg32_uint           = 101,
  rg32_sint           = 102,
  rg32_sfloat         = 103,
  rgb32_uint          = 104,
  rgb32_sint          = 105,
  rgb32_sfloat        = 106,
  rgba32_uint         = 107,
  rgba32_sint         = 108,
  rgba32_sfloat       = 109,
  d16_unorm           = 124,
  d32_sfloat          = 126,
  d24_unorm_s8_uint   = 129,
  d32_sfloat_s8_uint  = 130,
  bc1_rgb_unorm       = 131,
  bc1_rgb_srgb        = 132,
  bc1_rgba_unorm      = 133,
  bc1_rgba_srgb       = 134,
  bc2_unorm           = 135,
  bc2_srgb            = 136,
  bc3_unorm           = 137,
  bc3_srgb            = 138,
  bc4_unorm           = 139,
  bc4_snorm           = 140,
  bc5_unorm           = 141,
  bc5_snorm           = 142,
  bc6h_ufloat         = 143,
  bc6h_sfloat         = 144,
  bc7_unorm           = 145,
  bc7_srgb            = 146
};

enum InputRate {
  VertexRate,
  InstanceRate
};

enum MeshType {
  Solid,
  Wireframe,
  Vertices
};

enum CullMode {
  None,
  Front,
  Back
};

enum DrawDirection {
  CounterClockwise,
  Clockwise
};

struct Settings {
  std::string application_name = "Groot Engine Application";
  unsigned int application_version = 1;
  std::pair<unsigned int, unsigned int> window_size = std::make_pair(1280, 720);
  std::string window_title = "Groot Engine Application";
  unsigned int gpu_index = 0;
  double time_step = 1.0 / 60.0;
  Format color_format = Format::rgba8_srgb;
  Format depth_format = Format::d32_sfloat;
};

class RID {
  friend class Engine;

  unsigned long m_id = ~(0x0);
  ResourceType m_type = ResourceType::Invalid;

  struct Hash {
    std::size_t operator()(const RID&) const;
  };

  public:
    RID() = default;
    RID(const RID&) = default;
    RID(RID&&) = default;

    ~RID() = default;

    RID& operator=(const RID&) = default;
    RID& operator=(RID&&) = default;

    bool operator==(const RID&) const;
    const unsigned long& operator*() const;

    bool is_valid() const;

  private:
    explicit RID(unsigned long, ResourceType);

    void invalidate();
};

struct GraphicsPipelineShaders {
  RID vertex = RID();
  RID fragment = RID();
  RID tesselation_control = RID();
  RID tesselation_evaluation = RID();
};

struct VertexBinding {
  unsigned int binding = 0;
  unsigned int stride = 0;
  InputRate input_rate = InputRate::VertexRate;
};

struct VertexAttribute {
  unsigned int location = 0;
  unsigned int binding = 0;
  Format format = Format::undefined;
  unsigned int offset = 0;
};

struct GraphicsPipelineSettings {
  std::vector<VertexBinding> vertex_bindings = {};
  std::vector<VertexAttribute> vertex_attributes = {};
  MeshType mesh_type = MeshType::Solid;
  CullMode cull_mode = CullMode::Back;
  DrawDirection draw_direction = DrawDirection::CounterClockwise;
  bool enable_depth = true;
  bool enable_blend = true;
};

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

class Log {
  public:
    Log() = delete;
    Log(const Log&) = delete;
    Log(Log&&) = delete;

    ~Log() = default;

    Log& operator=(const Log&) = delete;
    Log& operator=(Log&&) = delete;

    static void generic(const std::string&);
    static void warn(const std::string&);
    static void runtime_error(const std::string&);
    static void out_of_range(const std::string&);
};

} // namespace groot