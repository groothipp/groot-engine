#pragma once

#include <cmath>
#include <compare>
#include <cstdint>
#include <functional>
#include <numbers>
#include <set>
#include <queue>
#include <string>
#include <unordered_map>

class GLFWwindow;

namespace vk {

class CommandBuffer;

} // namespace vk

namespace groot {

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

namespace groot::detail {

template <typename T>
struct Vec2 {
  T x = T{}, y = T{};

  inline Vec2() = default;
  inline explicit Vec2(T s) : x(s), y(s) {}
  inline Vec2(T a, T b) : x(a), y(b) {}
  inline Vec2(const Vec2&) = default;
  inline Vec2(Vec2&&) = default;

  template <typename K>
  inline Vec2(const Vec2<K>& vec) : x(static_cast<T>(vec.x)), y(static_cast<T>(vec.y)) {}

  inline ~Vec2() = default;

  inline Vec2& operator=(const Vec2&) = default;
  inline Vec2& operator=(Vec2&&) = default;

  inline T& operator[](unsigned int index) {
    switch (index) {
      case 0: return x;
      case 1: return y;
      default:
        Log::out_of_range("vec2 access out of range");
        return x;
    }
  }

  inline const T& operator[](unsigned int index) const {
    switch(index) {
      case 0:   return x;
      case 1:   return y;
      default:
        Log::out_of_range("vec2 access out of range");
        return x;
    }
  }

  inline std::partial_ordering operator<=>(const Vec2&) const = default;

  inline Vec2 operator+(const Vec2& rhs) const { return Vec2(x + rhs.x, y + rhs.y); }
  inline Vec2 operator-(const Vec2& rhs) const { return Vec2(x - rhs.x, y - rhs.y); }
  inline Vec2 operator-() const { return Vec2(-x, -y); }
  inline Vec2 operator*(const Vec2& rhs) const { return Vec2(x * rhs.x, y * rhs.y); }
  inline Vec2 operator*(T s) const { return Vec2(x * s, y * s); }
  inline Vec2 operator/(const Vec2& rhs) const { return Vec2( x / rhs.x, y / rhs.y ); }
  inline Vec2 operator/(T s) const { return Vec2(x / s, y / s); }

  inline T dot(const Vec2& vec) const { return x * vec.x + y * vec.y; }
  inline T mag() const { return std::sqrt(dot(*this)); }
  inline T mag_squared() const { return dot(*this); }
  inline Vec2 normalized() const { return *this / mag(); }
};

template <typename T>
struct Vec3 {
  T x{}, y{}, z{};

  inline Vec3() = default;
  inline explicit Vec3(T s) : x(s), y(s), z(s) {}
  inline Vec3(T a, T b, T c) : x(a), y(b), z(c) {}
  inline Vec3(const Vec2<T>& u, T s) : x(u.x), y(u.y), z(s) {}
  inline Vec3(const Vec3&) = default;
  inline Vec3(Vec3&&) = default;

  template <typename K>
  inline Vec3(const Vec3<K>& vec) : x(static_cast<T>(vec.x)), y(static_cast<T>(vec.y)), z(static_cast<T>(vec.z)) {}

  inline ~Vec3() = default;

  inline Vec3& operator=(const Vec3&) = default;
  inline Vec3& operator=(Vec3&&) = default;

  inline T& operator[](unsigned int index) {
    switch (index) {
      case 0: return x;
      case 1: return y;
      case 2: return z;
      default:
        Log::out_of_range("vec3 access out of range");
        return x;
    }
  }

  inline const T& operator[](unsigned int index) const {
    switch (index) {
      case 0: return x;
      case 1: return y;
      case 2: return z;
      default:
        Log::out_of_range("vec3 access out of range");
        return x;
    }
  }

  inline std::partial_ordering operator<=>(const Vec3&) const = default;

  inline Vec3 operator+(const Vec3& rhs) const { return Vec3(x + rhs.x, y + rhs.y, z + rhs.z); }
  inline Vec3 operator-(const Vec3& rhs) const { return Vec3(x - rhs.x, y - rhs.y, z - rhs.z); }
  inline Vec3 operator-() const { return Vec3(-x, -y, -z); }
  inline Vec3 operator*(const Vec3& rhs) const { return Vec3(rhs.x * x, rhs.y * y, rhs.z * z); }
  inline Vec3 operator*(T rhs) const { return Vec3(rhs * x, rhs * y, rhs * z); }
  inline Vec3 operator/(const Vec3& rhs) const { return Vec3(x / rhs.x, y / rhs.y, z / rhs.z); }
  inline Vec3 operator/(T rhs) const { return Vec3(x / rhs, y / rhs, z / rhs); }

  inline T dot(const Vec3& vec) const { return x * vec.x + y * vec.y + z * vec.z; }
  inline Vec3 cross(const Vec3& vec) const { return Vec3(y * vec.z - z * vec.y, z * vec.x - x * vec.z, x * vec.y - y * vec.x); }
  inline T mag() const { return std::sqrt(dot(*this)); }
  inline T mag_squared() const { return dot(*this); }
  inline Vec3 normalized() const { return *this / mag(); }
};

template<typename T>
struct Vec4 {
  T x{}, y{}, z{}, w{};

  inline Vec4() = default;
  inline Vec4(T s) : x(s), y(s), z(s), w(s) {}
  inline Vec4(T a, T b, T c, T d) : x(a), y(b), z(c), w(d) {}
  inline Vec4(const Vec2<T>& vec, T a, T b) : x(vec.x), y(vec.y), z(a), w(b) {}
  inline Vec4(const Vec3<T>& vec, T s) : x(vec.x), y(vec.y), z(vec.z), w(s) {}
  inline Vec4(const Vec4&) = default;
  inline Vec4(Vec4&&) = default;

  template <typename K>
  inline Vec4(const Vec4<K>& vec) : x(static_cast<T>(vec.x)), y(static_cast<T>(vec.y)), z(static_cast<T>(vec.z)), w(static_cast<T>(vec.w)) {}

  inline ~Vec4() = default;

  inline Vec4& operator=(const Vec4&) = default;
  inline Vec4& operator=(Vec4&&) = default;

  inline T& operator[](unsigned int index) {
    switch (index) {
      case 0: return x;
      case 1: return y;
      case 2: return z;
      case 3: return w;
      default:
        Log::out_of_range("vec3 access out of range");
        return x;
    }
  }

  inline const T& operator[](unsigned int index) const {
    switch (index) {
      case 0: return x;
      case 1: return y;
      case 2: return z;
      case 3: return w;
      default:
        Log::out_of_range("vec3 access out of range");
        return x;
    }
  }

  inline std::partial_ordering operator<=>(const Vec4&) const = default;

  inline Vec4 operator+(const Vec4& rhs) const { return Vec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
  inline Vec4 operator-(const Vec4& rhs) const { return Vec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
  inline Vec4 operator-() const { return Vec4(-x, -y, -z, -w); }
  inline Vec4 operator*(const Vec4& rhs) const { return Vec4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
  inline Vec4 operator*(T rhs) const { return Vec4(x * rhs, y * rhs, z * rhs, w * rhs); }
  inline Vec4 operator/(const Vec4& rhs) const { return Vec4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }
  inline Vec4 operator/(T rhs) const { return Vec4(x / rhs, y / rhs, z / rhs, w / rhs); }

  inline T dot(const Vec4& vec) const { return x * vec.x + y * vec.y + z * vec.z + w * vec.w; }
  inline T mag() const { return std::sqrt(dot(*this)); }
  inline T mag_squared() const { return dot(*this); }
  inline Vec4 normalized() const { return *this / mag(); }
};

} // namespace groot::detail

namespace groot {

using vec2  = detail::Vec2<float>;
using ivec2 = detail::Vec2<int>;
using uvec2 = detail::Vec2<unsigned int>;
using vec3  = detail::Vec3<float>;
using ivec3 = detail::Vec3<int>;
using uvec3 = detail::Vec3<unsigned int>;
using vec4  = detail::Vec4<float>;
using ivec4 = detail::Vec4<int>;
using uvec4 = detail::Vec4<unsigned int>;

class Allocator;
class Renderer;
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
  Sampler,
  Image,
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

enum SampleMode {
  Repeat,
  MirroredRepeat,
  ClampToEdge,
  ClampToBorder
};

enum Filter {
  Nearest,
  Linear
};

enum ColorSpace {
  srgb_nonlinear        = 0,
  display_p3_nonlinear  = 1000104001,
  extended_srgb_linear  = 1000104002
};

enum RenderMode {
  NoSync,
  TripleBuffer,
  VSync
};

struct Settings {
  std::string application_name = "Groot Engine Application";
  unsigned int application_version = 1;
  std::pair<unsigned int, unsigned int> window_size = std::make_pair(1280, 720);
  std::string window_title = "Groot Engine Application";
  unsigned int gpu_index = 0;
  double time_step = 1.0 / 60.0;
  Format color_format = Format::bgra8_srgb;
  ColorSpace color_space = ColorSpace::srgb_nonlinear;
  RenderMode render_mode = RenderMode::TripleBuffer;
  float fov = 70.0f;
  unsigned int flight_frames = 3;
  vec4 background_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
};

class mat2 {
  vec2 m_col1, m_col2;

  public:
    mat2() = default;
    explicit mat2(float s);
    mat2(const vec2&, const vec2&);
    mat2(const mat2&) = default;
    mat2(mat2&&) = default;

    ~mat2() = default;

    mat2& operator=(const mat2&) = default;
    mat2& operator=(mat2&&) = default;

    vec2& operator[](unsigned int);
    const vec2& operator[](unsigned int) const;

    std::partial_ordering operator<=>(const mat2&) const = default;

    mat2 operator+(const mat2&) const;
    mat2 operator-(const mat2&) const;
    mat2 operator-() const;
    mat2 operator*(const mat2&) const;
    vec2 operator*(const vec2&) const;
    mat2 operator*(float) const;
    mat2 operator/(float) const;

    mat2 inverse() const;
    mat2 transpose() const;
    float determinant() const;
    float trace() const;

    static mat2 identity();
    static mat2 rotation(float);
    static mat2 scale(float, float);
};

class mat3 {
  vec3 m_col1, m_col2, m_col3;

  public:
    mat3() = default;
    explicit mat3(float);
    mat3(const vec3&, const vec3&, const vec3&);
    explicit mat3(const mat2&, float s = 0);
    mat3(const mat3&) = default;
    mat3(mat3&&) = default;

    ~mat3() = default;

    mat3& operator=(const mat3&) = default;
    mat3& operator=(mat3&&) = default;

    vec3& operator[](unsigned int);
    const vec3& operator[](unsigned int) const;

    std::partial_ordering operator<=>(const mat3&) const = default;

    mat3 operator+(const mat3&) const;
    mat3 operator-(const mat3&) const;
    mat3 operator-() const;
    mat3 operator*(const mat3&) const;
    vec3 operator*(const vec3&) const;
    mat3 operator*(float) const;
    mat3 operator/(float) const;

    mat3 inverse() const;
    mat3 transpose() const;
    float determinant() const;
    float trace() const;

    static mat3 identity();
    static mat3 rotation_x(float);
    static mat3 rotation_y(float);
    static mat3 rotation_z(float);
    static mat3 rotation(const vec3&, float);
    static mat3 euler_rotation(float, float, float);
    static mat3 scale(float, float, float);
};

class mat4 {
  vec4 m_col1, m_col2, m_col3, m_col4;

  public:
    mat4() = default;
    explicit mat4(float);
    mat4(const vec4&, const vec4&, const vec4&, const vec4&);
    explicit mat4(const mat2&, float s = 0.0f);
    explicit mat4(const mat3&, float s = 0.0f);
    mat4(const mat4&) = default;
    mat4(mat4&&) = default;

    ~mat4() = default;

    mat4& operator=(const mat4&) = default;
    mat4& operator=(mat4&&) = default;

    vec4& operator[](unsigned int);
    const vec4& operator[](unsigned int) const;

    std::partial_ordering operator<=>(const mat4&) const = default;

    mat4 operator+(const mat4&) const;
    mat4 operator-(const mat4&) const;
    mat4 operator-() const;
    mat4 operator*(const mat4&) const;
    vec4 operator*(const vec4&) const;
    mat4 operator*(float) const;
    mat4 operator/(float) const;

    mat4 inverse() const;
    mat4 transpose() const;
    float determinant() const;
    float trace() const;

    static mat4 identity();
    static mat4 translation(const vec3&);
    static mat4 rotation(const vec3&);
    static mat4 scale(float, float, float);
    static mat4 view(const vec3&, const vec3&, const vec3&);
    static mat4 perspective_projection(float, float, float, float);

  private:
    mat3 getMinorMatrix(unsigned int, unsigned int) const;
};

struct Transform {
  vec3 position = vec3(0.0f);
  vec3 rotation = vec3(0.0f);
  vec3 scale = vec3(1.0f);

  mat4 matrix() const;
};

class RID {
  friend class Engine;

  unsigned long m_id = ~(0x0);
  ResourceType m_type = ResourceType::Invalid;

  public:
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
    bool operator<(const RID&) const;
    bool operator>(const RID&) const;
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

struct GraphicsPipelineSettings {
  MeshType mesh_type = MeshType::Solid;
  CullMode cull_mode = CullMode::Back;
  DrawDirection draw_direction = DrawDirection::CounterClockwise;
  bool enable_depth_test = true;
  bool enable_depth_write = true;
  bool enable_blend = true;
};

struct SamplerSettings {
  Filter mag_filter = Filter::Linear;
  Filter min_filter = Filter::Linear;
  SampleMode mode_u = SampleMode::Repeat;
  SampleMode mode_v = SampleMode::Repeat;
  bool anisotropic_filtering = true;
};

struct ComputeCommand {
  RID pipeline = RID();
  RID descriptor_set = RID();
  std::vector<uint8_t> push_constants;
  bool barrier = false;
  std::tuple<unsigned int, unsigned int, unsigned int> work_groups = { 1, 1, 1 };
};

class alignas(64) Object {
  friend class Engine;
  friend class Renderer;

  RID m_id;
  RID m_mesh;
  RID m_pipeline;
  RID m_set;

  public:
    Object() = default;
    Object(const Object&);
    Object(Object&&) = default;

    ~Object() = default;

    Object& operator=(const Object&);
    Object& operator=(Object&&) = default;

    bool operator<(const Object&) const;

    bool is_in_scene() const;

    void set_mesh(const RID&);
    void set_pipeline(const RID&);
    void set_descriptor_set(const RID&);
};

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

  std::set<Object> m_scene;
  mat4 m_cameraView = mat4::view(vec3(0.0f, 0.0f, 2.0f), vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
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
    void transitionImagesCompute() const;
    void transitionImagesGraphics(const vk::CommandBuffer&) const;
};

inline float radians(float deg) {
  return std::numbers::pi_v<float> / 180.0 * deg;
}

} // namespace groot

inline groot::vec2 operator*(float lhs, const groot::vec2& rhs) {
  return rhs * lhs;
}

inline groot::ivec2 operator*(int lhs, const groot::ivec2& rhs) {
  return rhs * lhs;
}

inline groot::uvec2 operator*(unsigned int lhs, const groot::uvec2& rhs) {
  return rhs * lhs;
}

inline groot::vec3 operator*(float lhs, const groot::vec3& rhs) {
  return rhs * lhs;
}

inline groot::ivec3 operator*(int lhs, const groot::ivec3& rhs) {
  return rhs * lhs;
}

inline groot::ivec3 operator*(unsigned int lhs, const groot::uvec3& rhs) {
  return rhs * lhs;
}

inline groot::vec4 operator*(float lhs, const groot::vec4& rhs) {
  return rhs * lhs;
}

inline groot::ivec4 operator*(int lhs, const groot::ivec4& rhs) {
  return rhs * lhs;
}

inline groot::uvec4 operator*(unsigned int lhs, const groot::uvec4& rhs) {
  return rhs * lhs;
}

inline groot::mat2 operator*(float lhs, const groot::mat2& rhs) {
  return rhs * lhs;
}

inline groot::mat3 operator*(float lhs, const groot::mat3& rhs) {
  return rhs * lhs;
}

inline groot::mat4 operator*(float lhs, const groot::mat4& rhs) {
  return rhs * lhs;
}