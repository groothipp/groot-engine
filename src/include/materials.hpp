#pragma once

#include "src/include/linalg.hpp"

#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_beta.h>

#include <map>

#define all_stages  vk::ShaderStageFlagBits::eVertex                  | \
                    vk::ShaderStageFlagBits::eGeometry                | \
                    vk::ShaderStageFlagBits::eTessellationControl     | \
                    vk::ShaderStageFlagBits::eTessellationEvaluation  | \
                    vk::ShaderStageFlagBits::eFragment                | \
                    vk::ShaderStageFlagBits::eCompute

namespace ge {

class BufferProxy;
class Engine;
class Material;

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
  public:
    class Builder {
      friend class Material;

      public:
        Builder() = default;
        Builder(const Builder&) = default;
        Builder(Builder&&) = default;

        ~Builder() = default;

        Builder& operator=(const Builder&) = default;
        Builder& operator=(Builder&&) = default;

        Builder& add_shader(ShaderStage, const std::string&);
        Builder& add_mutable(BufferProxy *);
        // Builder& add_immutable(unsigned int, void *);
        // Builder& add_texture();
        // Builder& add_canvas();

      private:
        std::map<vk::ShaderStageFlagBits, std::string> m_shaders;
        std::vector<BufferProxy *> m_mutables;
    };

  private:
    class Iterator {
      using Output = std::tuple<
        const std::string&,
        const vk::raii::Pipeline&,
        const vk::raii::PipelineLayout&,
        const vk::raii::DescriptorSets&
      >;

      public:
        Iterator(const MaterialManager *, const std::map<std::string, Material>::const_iterator&);
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
        std::map<std::string, Material>::const_iterator m_iterator;
    };

  public:
    MaterialManager() = default;
    MaterialManager(const MaterialManager&) = delete;
    MaterialManager(MaterialManager&&) = delete;

    ~MaterialManager() = default;

    MaterialManager& operator=(const MaterialManager&) = delete;
    MaterialManager& operator=(MaterialManager&&) = delete;

    Iterator begin() const;
    Iterator end() const;

    bool exists(const std::string&) const;

    void add(const std::string&, const Builder&);
    void load(const Engine&, const std::map<std::string, std::vector<mat4>>&);
    void update(unsigned int, const std::map<std::string, std::vector<mat4>>&);

  private:
    std::map<std::string, Builder> m_builders;
    std::map<std::string, Material> m_materials;
};

class Material {
  using ShaderStages = std::pair<
    std::vector<vk::raii::ShaderModule>,
    std::vector<vk::PipelineShaderStageCreateInfo>
  >;

  using Output = std::tuple<
    const vk::raii::Pipeline&,
    const vk::raii::PipelineLayout&,
    const vk::raii::DescriptorSets&
  >;

  public:
    Material() = default;
    Material(const Material&) = delete;
    Material(Material&&) = default;

    ~Material() = default;

    Material& operator=(const Material&) = delete;
    Material& operator=(Material&&) = default;

    Output operator*() const;

    void load(const Engine&, const MaterialManager::Builder&, const std::vector<mat4>&);
    void batchMutable(void *, unsigned int, const std::vector<unsigned int>&);
    void updateTransforms(unsigned int, const std::vector<mat4>&);
    void updateMutables(unsigned int);

  private:
    ShaderStages getShaderStages(const Engine&, const MaterialManager::Builder&) const;

    void createLayout(const Engine&, const MaterialManager::Builder&);
    void createPipeline(const Engine&, const MaterialManager::Builder&);
    void createDescriptors(const Engine&, const std::vector<mat4>&, const MaterialManager::Builder&);
    void createSets(const Engine&, const MaterialManager::Builder&);

  private:
    std::map<void *, std::pair<unsigned int, std::vector<unsigned int>>> m_mutableUpdates;

    vk::raii::PipelineLayout m_layout = nullptr;
    vk::raii::Pipeline m_pipeline = nullptr;

    vk::raii::DescriptorSetLayout m_setLayout = nullptr;
    vk::raii::DescriptorPool m_pool = nullptr;
    vk::raii::DescriptorSets m_sets = nullptr;

    vk::raii::DeviceMemory m_transformMemory = nullptr;
    std::vector<vk::raii::Buffer> m_transformBuffers;
    std::vector<unsigned int> m_transformOffsets;
    void * m_transformMap = nullptr;

    vk::raii::DeviceMemory m_mutableMemory = nullptr;
    std::vector<vk::raii::Buffer> m_mutableBuffers;
    void * m_mutableMap = nullptr;
};

} // namespace ge