#pragma once

#include "src/include/linalg.hpp"

#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_beta.h>

#include <map>
#include <string>

#define all_stages  vk::ShaderStageFlagBits::eVertex                  | \
                    vk::ShaderStageFlagBits::eGeometry                | \
                    vk::ShaderStageFlagBits::eTessellationControl     | \
                    vk::ShaderStageFlagBits::eTessellationEvaluation  | \
                    vk::ShaderStageFlagBits::eFragment                | \
                    vk::ShaderStageFlagBits::eCompute

namespace ge {

class Engine;

enum ShaderStage {
  VertexShader = static_cast<unsigned int>(vk::ShaderStageFlagBits::eVertex),
  GeometryShader = static_cast<unsigned int>(vk::ShaderStageFlagBits::eGeometry),
  TesselationControlShader = static_cast<unsigned int>(vk::ShaderStageFlagBits::eTessellationControl),
  TesselationEvaluationShader = static_cast<unsigned int>(vk::ShaderStageFlagBits::eTessellationEvaluation),
  FragmentShader = static_cast<unsigned int>(vk::ShaderStageFlagBits::eFragment),
  ComputeShader = static_cast<unsigned int>(vk::ShaderStageFlagBits::eCompute)
};

struct EngineData {
  mat4 view = mat4::identity();
  mat4 projection = mat4::identity();
};

class MaterialManager {
  using ShaderStages = std::pair<
    std::vector<vk::raii::ShaderModule>,
    std::vector<vk::PipelineShaderStageCreateInfo>
  >;

  public:
    class Builder {
      friend class MaterialManager;

      public:
        Builder() = default;
        Builder(const Builder&) = default;
        Builder(Builder&&) = default;

        ~Builder() = default;

        Builder& operator=(const Builder&) = default;
        Builder& operator=(Builder&&) = default;

        Builder& add_shader(ShaderStage, std::string);

      private:
        std::map<vk::ShaderStageFlagBits, std::string> m_shaders;
    };

  private:
    class Iterator {
      using Output = std::tuple<
        const std::string&,
        const vk::raii::Pipeline&,
        const vk::raii::PipelineLayout&,
        const vk::raii::DescriptorSet&
      >;

      public:
        Iterator(const MaterialManager *, const std::map<std::string, unsigned int>::const_iterator&);
        Iterator(const Iterator&) = default;
        Iterator(Iterator&&) = default;

        ~Iterator() = default;

        Iterator& operator=(const Iterator&) = default;
        Iterator& operator=(Iterator&&) = default;

        bool operator!=(const Iterator&) const;

        Output operator*() const;

        Iterator& operator++();

      private:
        const MaterialManager * m_manager;
        std::map<std::string, unsigned int>::const_iterator m_iterator;
    };

  public:
    MaterialManager() = default;
    MaterialManager(MaterialManager&) = delete;
    MaterialManager(MaterialManager&&) = delete;

    ~MaterialManager() = default;

    MaterialManager& operator=(MaterialManager&) = delete;
    MaterialManager& operator=(MaterialManager&&) = delete;

    Iterator begin() const;
    Iterator end() const;

    bool exists(std::string) const;

    void add(const std::string&, const Builder&);
    void add(const std::string&, Builder&&);
    void load(const Engine&, const std::map<std::string, std::vector<mat4>>&);
    void updateTransforms(const std::map<std::string, std::vector<mat4>>&);
    void updateFrameIndex(const Engine&);

  private:
    ShaderStages getShaderStages(const Engine&, const Builder&) const;

    void createLayout(const Engine&);
    void createPipeline(const Engine&, const Builder&);
    void createDescriptors(const Engine&, const std::map<std::string, std::vector<mat4>>&);
    void updateSets(const Engine&);
    void updateTransforms(const std::vector<mat4>&, unsigned int);

  private:
    std::map<std::string, unsigned int> m_materials;
    std::vector<Builder> m_builders;
    unsigned int m_frameIndex = 0;

    std::vector<vk::raii::DescriptorSetLayout> m_setLayouts;
    std::vector<vk::raii::PipelineLayout> m_layouts;
    std::vector<vk::raii::Pipeline> m_pipelines;

    vk::raii::DescriptorPool m_setPool = nullptr;
    vk::raii::DescriptorSets m_sets = nullptr;

    vk::raii::DeviceMemory m_transformMemory = nullptr;
    std::vector<vk::raii::Buffer> m_transformBuffers;
    std::vector<unsigned int> m_transformOffsets;
    void * m_transformMap = nullptr;
};

} // namespace ge