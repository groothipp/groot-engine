#include "src/include/allocator.hpp"
#include "src/include/engine.hpp"
#include "src/include/materials.hpp"
#include "src/include/parsers.hpp"

namespace ge {

MaterialManager::Builder& MaterialManager::Builder::add_shader(ShaderStage stage, std::string path) {
  m_shaders[static_cast<vk::ShaderStageFlagBits>(stage)] = path;
  return *this;
}

MaterialManager::Iterator::Iterator(
  const MaterialManager * manager,
  const std::map<std::string, unsigned int>::const_iterator& iterator
) : m_manager(manager), m_iterator(iterator) {}

bool MaterialManager::Iterator::operator!=(const Iterator& rhs) const {
  return m_iterator != rhs.m_iterator;
}

MaterialManager::Iterator::Output MaterialManager::Iterator::operator*() const {
  auto& [tag, material] = *m_iterator;
  return Output(
    tag,
    m_manager->m_pipelines[material],
    m_manager->m_layouts[material],
    m_manager->m_sets[m_manager->m_frameIndex * m_manager->m_materials.size() + material]
  );
}

MaterialManager::Iterator& MaterialManager::Iterator::operator++() {
  ++m_iterator;
  return *this;
}

MaterialManager::Iterator MaterialManager::begin() const {
  return Iterator(this, m_materials.begin());
}

MaterialManager::Iterator MaterialManager::end() const {
  return Iterator(this, m_materials.end());
}

bool MaterialManager::exists(std::string tag) const {
  return m_materials.contains(tag);
}

void MaterialManager::add(const std::string& tag, const Builder& builder) {
  m_materials.emplace(tag, static_cast<unsigned int>(m_builders.size()));
  m_builders.emplace_back(builder);
}

void MaterialManager::add(const std::string& tag, Builder&& builder) {
  m_materials.emplace(tag, static_cast<unsigned int>(m_builders.size()));
  m_builders.emplace_back(std::move(builder));
}

void MaterialManager::load(const Engine& engine, const std::map<std::string, std::vector<mat4>>& transforms) {
  if (transforms.size() != m_materials.size())
    throw std::runtime_error("groot-engine: missing transform data");

  for (auto& [tag, material] : m_materials) {
    createLayout(engine);
    createPipeline(engine, m_builders[material]);
  }

  createDescriptors(engine, transforms);

  unsigned int storageCount = engine.m_settings.buffer_mode * transforms.size();
  unsigned int uniformCount, imageCount, samplerCount;
  uniformCount = imageCount = samplerCount = 0;

  auto [tmp_setPool, tmp_sets] = Allocator::descriptorPool(engine, m_setLayouts,
    storageCount,
    uniformCount,
    imageCount,
    samplerCount
  );
  m_setPool = std::move(tmp_setPool);
  m_sets = std::move(tmp_sets);

  updateSets(engine);
}

void MaterialManager::updateTransforms(const std::map<std::string, std::vector<mat4>>& transforms) {
  for (const auto& [material, matrices] : transforms) {
    if (!m_materials.contains(material)) continue;

    unsigned int index = m_frameIndex * m_materials.size() + m_materials.at(material);

    memcpy(
      reinterpret_cast<char *>(m_transformMap) + m_transformOffsets[index],
      matrices.data(),
      sizeof(mat4) * matrices.size()
    );
  }
}

void MaterialManager::updateFrameIndex(const Engine& engine) {
  m_frameIndex = engine.m_renderer.frameIndex();
}

MaterialManager::ShaderStages MaterialManager::getShaderStages(const Engine& engine, const Builder& builder) const {
  std::vector<vk::raii::ShaderModule> modules;
  std::vector<vk::PipelineShaderStageCreateInfo> infos;

  for (auto [stage, path] : builder.m_shaders) {
    std::vector<char> code = SPVParser::parse(path);

    modules.emplace_back(engine.m_context.device().createShaderModule(vk::ShaderModuleCreateInfo{
      .codeSize = static_cast<unsigned int>(code.size()),
      .pCode    = reinterpret_cast<unsigned int *>(code.data())
    }));

    infos.emplace_back(vk::PipelineShaderStageCreateInfo{
      .stage  = stage,
      .module = modules.back(),
      .pName  = "main"
    });
  }

  return { std::move(modules), std::move(infos) };
}

void MaterialManager::createLayout(const Engine& engine) {
  vk::DescriptorSetLayoutBinding transformBinding{
    .binding          = 0,
    .descriptorType   = vk::DescriptorType::eStorageBuffer,
    .descriptorCount  = 1,
    .stageFlags       = vk::ShaderStageFlagBits::eVertex
  };

  m_setLayouts.emplace_back(engine.m_context.device().createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo{
    .bindingCount = 1,
    .pBindings    = &transformBinding
  }));

  vk::PushConstantRange range{
    .stageFlags = all_stages,
    .size       = sizeof(EngineData)
  };

  m_layouts.emplace_back(engine.m_context.device().createPipelineLayout(vk::PipelineLayoutCreateInfo{
    .setLayoutCount         = 1,
    .pSetLayouts            = &*m_setLayouts.back(),
    .pushConstantRangeCount = 1,
    .pPushConstantRanges    = &range
  }));
}

void MaterialManager::createPipeline(const Engine& engine, const Builder& builder) {
  auto [modules, ci_stages] = getShaderStages(engine, builder);

  vk::DynamicState dynStates[2] = {
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor
  };

  vk::PipelineDynamicStateCreateInfo ci_dynState{
    .dynamicStateCount  = 2,
    .pDynamicStates     = dynStates
  };

  vk::PipelineViewportStateCreateInfo ci_viewport{
    .viewportCount  = 1,
    .scissorCount   = 1
  };

  auto vertBinding = Vertex::binding();
  auto vertAttribs = Vertex::attributes();

  vk::PipelineVertexInputStateCreateInfo ci_input{
    .vertexBindingDescriptionCount    = 1,
    .pVertexBindingDescriptions       = &vertBinding,
    .vertexAttributeDescriptionCount  = static_cast<unsigned int>(vertAttribs.size()),
    .pVertexAttributeDescriptions     = vertAttribs.data()
  };

  vk::PipelineInputAssemblyStateCreateInfo ci_assembly{
    .topology               = vk::PrimitiveTopology::eTriangleList,
    .primitiveRestartEnable = false
  };

  vk::PipelineRasterizationStateCreateInfo ci_rasterizer{
    .depthClampEnable         = false,
    .rasterizerDiscardEnable  = false,
    .polygonMode              = vk::PolygonMode::eFill,
    .cullMode                 = vk::CullModeFlagBits::eBack,
    .frontFace                = vk::FrontFace::eCounterClockwise,
    .depthBiasEnable          = false,
    .lineWidth                = 1.0f
  };

  vk::PipelineMultisampleStateCreateInfo ci_multisample{
    .rasterizationSamples = vk::SampleCountFlagBits::e1,
    .sampleShadingEnable  = false
  };

  vk::PipelineDepthStencilStateCreateInfo ci_depth{
    .depthTestEnable        = true,
    .depthWriteEnable       = true,
    .depthCompareOp         = vk::CompareOp::eLess,
    .depthBoundsTestEnable  = false,
    .stencilTestEnable      = false
  };

  vk::PipelineColorBlendAttachmentState colorAttachment{
    .blendEnable          = true,
    .srcColorBlendFactor  = vk::BlendFactor::eSrcAlpha,
    .dstColorBlendFactor  = vk::BlendFactor::eOneMinusSrcAlpha,
    .colorBlendOp         = vk::BlendOp::eAdd,
    .srcAlphaBlendFactor  = vk::BlendFactor::eOne,
    .dstAlphaBlendFactor  = vk::BlendFactor::eOneMinusSrcAlpha,
    .alphaBlendOp         = vk::BlendOp::eAdd,
    .colorWriteMask       = vk::ColorComponentFlagBits::eR |
                            vk::ColorComponentFlagBits::eG |
                            vk::ColorComponentFlagBits::eB |
                            vk::ColorComponentFlagBits::eA
  };

  vk::PipelineColorBlendStateCreateInfo ci_blend{
    .logicOpEnable    = false,
    .attachmentCount  = 1,
    .pAttachments     = &colorAttachment
  };

  vk::PipelineRenderingCreateInfo ci_rendering{
    .colorAttachmentCount     = 1,
    .pColorAttachmentFormats  = &engine.m_settings.format,
    .depthAttachmentFormat    = engine.m_settings.depth_format
  };

  m_pipelines.emplace_back(engine.m_context.device().createGraphicsPipeline(nullptr, vk::GraphicsPipelineCreateInfo{
    .pNext                = &ci_rendering,
    .stageCount           = static_cast<unsigned int>(ci_stages.size()),
    .pStages              = ci_stages.data(),
    .pVertexInputState    = &ci_input,
    .pInputAssemblyState  = &ci_assembly,
    .pViewportState       = &ci_viewport,
    .pRasterizationState  = &ci_rasterizer,
    .pMultisampleState    = &ci_multisample,
    .pDepthStencilState   = &ci_depth,
    .pColorBlendState     = &ci_blend,
    .pDynamicState        = &ci_dynState,
    .layout               = m_layouts.back()
  }));
}

void MaterialManager::createDescriptors(const Engine& engine, const std::map<std::string, std::vector<mat4>>& transforms) {
  std::vector<vk::BufferCreateInfo> transformInfos;
  for (unsigned int i = 0; i < engine.m_settings.buffer_mode; ++i) {
    for (const auto& [material, matrices] : transforms) {
      transformInfos.emplace_back(vk::BufferCreateInfo{
        .size         = static_cast<unsigned int>(sizeof(mat4) * matrices.size()),
        .usage        = vk::BufferUsageFlagBits::eStorageBuffer,
        .sharingMode  = vk::SharingMode::eExclusive
      });
    }
  }

  auto [tmp_transMem, tmp_transBufs, tmp_transOffs, transSize] = Allocator::bufferPool(engine, transformInfos,
    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
  );
  m_transformMemory = std::move(tmp_transMem);
  m_transformBuffers = std::move(tmp_transBufs);
  m_transformOffsets = std::move(tmp_transOffs);

  m_transformMap = m_transformMemory.mapMemory(0, transSize);
  for (unsigned int i = 0; i < engine.m_settings.buffer_mode; ++i) {
    unsigned int j = 0;
    for (const auto& [material, matrices] : transforms)
      updateTransforms(matrices, i * transforms.size() + j++);
  }
}

void MaterialManager::updateSets(const Engine& engine) {
  std::vector<vk::WriteDescriptorSet> writes;

  unsigned int i = 0;
  for (const auto& buffer : m_transformBuffers) {
    vk::DescriptorBufferInfo info{
      .buffer = *buffer,
      .range  = vk::WholeSize
    };

    writes.emplace_back(vk::WriteDescriptorSet{
      .dstSet           = m_sets[i++],
      .dstBinding       = 0,
      .descriptorCount  = 1,
      .descriptorType   = vk::DescriptorType::eStorageBuffer,
      .pBufferInfo      = &info
    });
  }

  engine.m_context.device().updateDescriptorSets(writes, nullptr);
}

void MaterialManager::updateTransforms(const std::vector<mat4>& transforms, unsigned int index) {
  memcpy(
    reinterpret_cast<char *>(m_transformMap) + m_transformOffsets[index],
    transforms.data(),
    sizeof(mat4) * transforms.size()
  );
}

} // namespace ge