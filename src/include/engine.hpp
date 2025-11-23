#pragma once

#include "src/include/enums.hpp"
#include "src/include/rid.hpp"
#include "src/include/structs.hpp"

#include <string>
#include <unordered_map>
#include <functional>
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
    void updateTimes();
};

} // namespace groot