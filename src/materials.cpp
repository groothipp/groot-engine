#include "src/include/allocator.hpp"
#include "src/include/engine.hpp"
#include "src/include/materials.hpp"
#include "src/include/parsers.hpp"

namespace ge {

MaterialManager::Builder& MaterialManager::Builder::add_shader(ShaderStage stage, const std::string& path) {
  m_shaders[static_cast<vk::ShaderStageFlagBits>(stage)] = path;
  return *this;
}

MaterialManager::Iterator::Iterator(
  const MaterialManager * m, const std::map<std::string, Material>::const_iterator& itr
) : m_manager(m), m_iterator(itr)
{}

bool MaterialManager::Iterator::operator!=(const Iterator& rhs) const {
  return m_iterator != rhs.m_iterator;
}

MaterialManager::Iterator::Output MaterialManager::Iterator::operator*() const {
  const auto& [pipeline, layout, sets] = *m_iterator->second;

  return { m_iterator->first, pipeline, layout, sets };
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

bool MaterialManager::exists(const std::string& tag) const {
  return m_builders.contains(tag) || m_materials.contains(tag);
}

void MaterialManager::add(const std::string& tag, const Builder& builder) {
  m_builders[tag] = builder;
}

void MaterialManager::load(const Engine& engine, const std::map<std::string, std::vector<mat4>>& transforms) {
  for (const auto& [material, matrices] : transforms)
    m_materials.emplace(material, std::move(Material(engine, m_builders[material], matrices)));
}

void MaterialManager::updateTransforms(unsigned int frameIndex, const std::map<std::string, std::vector<mat4>>& transforms) {
  for (const auto& [material, matrices] : transforms)
    m_materials.at(material).updateTransforms(frameIndex, matrices);
}

Material::Material(const Engine& engine, const MaterialManager::Builder& builder, const std::vector<mat4>& matrices) {
  createLayout(engine);
  createPipeline(engine, builder);
  createDescriptors(engine, matrices);
  createSets(engine);
}

Material::Output Material::operator*() const {
  return { m_pipeline, m_layout, m_sets };
}

void Material::updateTransforms(unsigned int frameIndex, const std::vector<mat4>& matrices) {
  memcpy(reinterpret_cast<char *>(m_map) + m_offsets[frameIndex], matrices.data(), sizeof(mat4) * matrices.size());
}

Material::ShaderStages Material::getShaderStages(const Engine& engine, const MaterialManager::Builder& builder) const {
  std::vector<vk::raii::ShaderModule> modules;
  std::vector<vk::PipelineShaderStageCreateInfo> infos;

  for (const auto& [stage, path] : builder.m_shaders) {
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

void Material::createLayout(const Engine& engine) {
  vk::DescriptorSetLayoutBinding binding{
    .binding          = 0,
    .descriptorType   = vk::DescriptorType::eStorageBuffer,
    .descriptorCount  = 1,
    .stageFlags       = vk::ShaderStageFlagBits::eVertex
  };

  m_setLayout = engine.m_context.device().createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo{
    .bindingCount = 1,
    .pBindings    = &binding
  });

  vk::PushConstantRange range{
    .stageFlags = vk::ShaderStageFlagBits::eAll,
    .size       = sizeof(EngineData)
  };

  m_layout = engine.m_context.device().createPipelineLayout(vk::PipelineLayoutCreateInfo{
    .setLayoutCount         = 1,
    .pSetLayouts            = &*m_setLayout,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges    = &range
  });
}

void Material::createPipeline(const Engine& engine, const MaterialManager::Builder& builder) {
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

  m_pipeline = engine.m_context.device().createGraphicsPipeline(nullptr, vk::GraphicsPipelineCreateInfo{
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
    .layout               = m_layout
  });
}

void Material::createDescriptors(const Engine& engine, const std::vector<mat4>& matrices) {
  std::vector<vk::BufferCreateInfo> bufferInfos;
  for (unsigned int i = 0; i < engine.m_settings.buffer_mode; ++i) {
    bufferInfos.emplace_back(vk::BufferCreateInfo{
      .size   = sizeof(mat4) * matrices.size(),
      .usage  = vk::BufferUsageFlagBits::eStorageBuffer,
    });
  }

  auto [tmp_mem, tmp_bufs, tmp_offs, allocationSize] = Allocator::bufferPool(engine, bufferInfos,
    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
  );
  m_memory = std::move(tmp_mem);
  m_buffers = std::move(tmp_bufs);
  m_offsets = std::move(tmp_offs);

  m_map = m_memory.mapMemory(0, allocationSize);

  for (unsigned int i = 0; i < engine.m_settings.buffer_mode; ++i)
    updateTransforms(i, matrices);
}

void Material::createSets(const Engine& engine) {
  auto [tmp_pool, tmp_sets] = Allocator::descriptorPool(engine, m_setLayout,
    engine.m_settings.buffer_mode,
    0,
    0,
    0
  );
  m_pool = std::move(tmp_pool);
  m_sets = std::move(tmp_sets);

  std::vector<vk::WriteDescriptorSet> writes;
  for (unsigned int i = 0; i < engine.m_settings.buffer_mode; ++i) {
    vk::DescriptorBufferInfo bufferInfo{
      .buffer = m_buffers[i],
      .range  = vk::WholeSize
    };

    writes.emplace_back(vk::WriteDescriptorSet{
      .dstSet           = m_sets[i],
      .dstBinding       = 0,
      .descriptorCount  = 1,
      .descriptorType   = vk::DescriptorType::eStorageBuffer,
      .pBufferInfo      = &bufferInfo
    });
  }

  engine.m_context.device().updateDescriptorSets(writes, nullptr);
}

} // namespace ge