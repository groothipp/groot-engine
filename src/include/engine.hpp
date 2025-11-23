#pragma once

#include "src/include/rid.hpp"
#include "src/include/shader_compiler.hpp"
#include "vulkan/vulkan.hpp"

#include <string>
#include <unordered_map>
#include <functional>
#include <vulkan/vulkan.hpp>

class GLFWwindow;

namespace groot {

class Allocator;
class VulkanContext;
class ShaderCompiler;

struct Settings {
  std::string application_name = "Groot Engine Application";
  unsigned int application_version = 1;
  std::pair<unsigned int, unsigned int> window_size = std::make_pair(1280, 720);
  std::string window_title = "Groot Engine Application";
  unsigned int gpu_index = 0;
  double time_step = 1.0 / 60.0;
  vk::Format color_format = vk::Format::eB8G8R8A8Srgb;
  vk::Format depth_format = vk::Format::eD32Sfloat;
};

struct GraphicsPipelineShaders {
  RID vertex = RID();
  RID fragment = RID();
  RID tesselation_control = RID();
  RID tesselation_evaluation = RID();
};

struct GraphicsPipelineSettings {
  std::vector<vk::VertexInputBindingDescription> vertex_bindings = {};
  std::vector<vk::VertexInputAttributeDescription> vertex_attributes = {};
  vk::PolygonMode polygon_mode = vk::PolygonMode::eFill;
  vk::CullModeFlags cull_mode = vk::CullModeFlagBits::eBack;
  vk::FrontFace front_face = vk::FrontFace::eCounterClockwise;
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
    void update_buffer(const RID&, std::size_t, void *) const;
    void destroy_buffer(RID&);

    RID compile_shader(ShaderType type, const std::string&);
    void destroy_shader(RID&);

    RID create_descriptor_set(const std::vector<RID>&);
    void destroy_descriptor_set(RID&);

    RID create_compute_pipeline(const RID&, const RID&);
    RID create_graphics_pipeline(const GraphicsPipelineShaders&, const RID&, const GraphicsPipelineSettings&);
    void destroy_pipeline(RID&);

  private:
    std::vector<vk::PipelineShaderStageCreateInfo> getShaderStages(const GraphicsPipelineShaders&) const;

    void updateTimes();
};

} // namespace groot