#pragma once

#include "src/include/linalg.hpp"

#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_beta.h>

#include <map>

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
      friend class MaterialManager;

      public:
        Builder() = default;
        Builder(const Builder&) = default;
        Builder(Builder&&) = default;

        ~Builder() = default;

        Builder& operator=(const Builder&) = default;
        Builder& operator=(Builder&&) = default;

        Builder& add_shader(ShaderStage, const std::string&);
        Builder& add_mutable(BufferProxy *);
        Builder& add_immutable(unsigned int, void *);
        Builder& add_texture(const std::string&);
        Builder& add_canvas();
        Builder& compute_space(unsigned int, unsigned int);

      private:
        std::map<vk::ShaderStageFlagBits, std::string> m_shaders;
        std::vector<BufferProxy *> m_mutables;
        std::vector<std::pair<unsigned int, void *>> m_immutables;
        std::vector<std::string> m_texturePaths;
        std::vector<std::tuple<unsigned int, unsigned int, unsigned int, std::vector<char>>> m_textures;
        unsigned int m_canvasCount = 0;
        std::pair<unsigned int, unsigned int> m_computeSpace = { 1, 1 };
    };

  private:
    class Iterator {
      using Output = std::tuple<
        const std::string&,
        const vk::raii::Pipeline&,
        const vk::raii::Pipeline&,
        const vk::raii::PipelineLayout&,
        const vk::raii::DescriptorSets&,
        const unsigned int&,
        const unsigned int&
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
    void transitionImages(vk::raii::CommandBuffer&, vk::raii::CommandBuffer&) const;

    void add(const std::string&, Builder&);
    void load(const Engine&, const std::map<std::string, std::vector<mat4>>&);
    void update(unsigned int, const std::map<std::string, std::vector<mat4>>&);

  private:
    std::map<std::string, Builder> m_builders;
    std::map<std::string, Material> m_materials;
    std::map<std::string, std::tuple<unsigned int, unsigned int, unsigned int, std::vector<char>>> m_textureMap;
};

class Material {
  using ShaderStages = std::pair<
    std::vector<vk::raii::ShaderModule>,
    std::vector<vk::PipelineShaderStageCreateInfo>
  >;

  using Output = std::tuple<
    const vk::raii::Pipeline&,
    const vk::raii::Pipeline&,
    const vk::raii::PipelineLayout&,
    const vk::raii::DescriptorSets&,
    const unsigned int&,
    const unsigned int&
  >;

  public:
    Material() = default;
    Material(const Material&) = delete;
    Material(Material&&) = default;

    ~Material() = default;

    Material& operator=(const Material&) = delete;
    Material& operator=(Material&&) = default;

    Output operator*() const;

    void transitionImages(vk::raii::CommandBuffer&, vk::raii::CommandBuffer&) const;

    void load(const Engine&, MaterialManager::Builder&, const std::vector<mat4>&);
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
    std::pair<unsigned int, unsigned int> m_computeSpace = { 1, 1 };

    vk::raii::PipelineLayout m_layout = nullptr;
    vk::raii::Pipeline m_pipeline = nullptr;
    vk::raii::Pipeline m_compute = nullptr;

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

    vk::raii::DeviceMemory m_immutableMemory = nullptr;
    std::vector<vk::raii::Buffer> m_immutableBuffers;

    vk::raii::Sampler m_sampler = nullptr;

    vk::raii::DeviceMemory m_textureMemory = nullptr;
    std::vector<vk::raii::Image> m_textures;
    std::vector<vk::raii::ImageView> m_textureViews;

    vk::raii::DeviceMemory m_canvasMemory = nullptr;
    std::vector<vk::raii::Image> m_canvases;
    std::vector<vk::raii::ImageView> m_canvasViews;
};

} // namespace ge