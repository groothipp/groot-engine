#pragma once

#include "src/include/enums.hpp"
#include "src/include/rid.hpp"

#include <vulkan/vulkan_raii.hpp>

#include <cstdint>

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
  double time_step = 1.0 / 60.0;
  Format color_format = Format::rgba8_srgb;
  Format depth_format = Format::d32_sfloat;
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