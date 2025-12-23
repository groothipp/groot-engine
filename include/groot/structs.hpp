#pragma once

#include "enums.hpp"
#include "linalg.hpp"
#include "rid.hpp"

#include <string>
#include <vector>

namespace groot {

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

struct Transform {
  vec3 position = vec3(0.0f);
  vec3 rotation = vec3(0.0f);
  vec3 scale = vec3(1.0f);

  mat4 matrix() const;
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
  SampleMode mode_w = SampleMode::Repeat;
  bool anisotropic_filtering = true;
};

struct ComputeCommand {
  RID pipeline = RID();
  RID descriptor_set = RID();
  std::vector<uint8_t> push_constants;
  bool barrier = false;
  std::tuple<unsigned int, unsigned int, unsigned int> work_groups = { 1, 1, 1 };
};

} // namespace groot