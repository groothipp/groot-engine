#pragma once

#include "src/include/enums.hpp"
#include "src/include/linalg.hpp"
#include "src/include/rid.hpp"

#include <vulkan/vulkan.hpp>

namespace groot {

struct VkBufferHash {
  std::size_t operator()(VkBuffer) const;
};

struct VkImageHash {
  std::size_t operator()(VkImage) const;
};

struct Settings {
  std::string application_name = "Groot Engine Application";
  unsigned int application_version = 1;
  std::pair<unsigned int, unsigned int> window_size = std::make_pair(1280, 720);
  std::string window_title = "Groot Engine Application";
  unsigned int gpu_index = 0;
  Format color_format = Format::bgra8_srgb;
  ColorSpace color_space = ColorSpace::srgb_nonlinear;
  RenderMode render_mode = RenderMode::TripleBuffer;
  float fov = 70.0f;
  unsigned int flight_frames = 3;
  vec4 background_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
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

struct DescriptorSetHandle {
  vk::DescriptorSetLayout layout = nullptr;
  vk::DescriptorPool pool = nullptr;
  vk::DescriptorSet set = nullptr;
};

struct PipelineHandle {
  vk::PipelineLayout layout = nullptr;
  vk::Pipeline pipeline = nullptr;
};

struct ImageHandle {
  vk::Image image = nullptr;
  vk::ImageView view = nullptr;
  RID sampler = RID();
};

struct SamplerSettings {
  Filter mag_filter = Filter::Linear;
  Filter min_filter = Filter::Linear;
  SampleMode mode_u = SampleMode::Repeat;
  SampleMode mode_v = SampleMode::Repeat;
  SampleMode mode_w = SampleMode::Repeat;
  bool anisotropic_filtering = true;
};

struct ComputeCommand {
  RID pipeline = RID();
  RID descriptor_set = RID();
  std::vector<unsigned char> push_constants;
  std::tuple<unsigned int, unsigned int, unsigned int> work_groups = { 1, 1, 1 };
  bool barrier = false;
};

struct Vertex {
  vec3 position;
  vec2 uv;
  vec3 normal;

  struct Hash {
    std::size_t operator()(const Vertex&) const;
  };

  bool operator==(const Vertex&) const;

  static vk::VertexInputBindingDescription binding();
  static std::array<vk::VertexInputAttributeDescription, 3> attributes();
};

struct MeshHandle {
  vk::Buffer vertexBuffer = nullptr;
  vk::Buffer indexBuffer  = nullptr;
  unsigned int indexCount = 0;
};

struct Transform {
  vec3 position = vec3(0.0f);
  vec3 rotation = vec3(0.0f);
  vec3 scale = vec3(1.0f);

  mat4 matrix() const;
};

} // namespace groot