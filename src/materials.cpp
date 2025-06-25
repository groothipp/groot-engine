#include "src/include/allocator.hpp"
#include "src/include/buffer.hpp"
#include "src/include/engine.hpp"
#include "src/include/materials.hpp"
#include "src/include/parsers.hpp"

namespace ge {

MaterialManager::Builder& MaterialManager::Builder::add_shader(ShaderStage stage, const std::string& path) {
  m_shaders[static_cast<vk::ShaderStageFlagBits>(stage)] = path;
  return *this;
}

MaterialManager::Builder& MaterialManager::Builder::add_mutable(BufferProxy * buffer) {
  m_mutables.emplace_back(buffer);
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
  return m_materials.contains(tag);
}

void MaterialManager::add(const std::string& tag, const Builder& builder) {
  m_builders[tag] = builder;
  m_materials[tag] = Material();
}

void MaterialManager::load(const Engine& engine, const std::map<std::string, std::vector<mat4>>& transforms) {
  for (const auto& [material, matrices] : transforms)
    m_materials.at(material).load(engine, m_builders.at(material), matrices);
}

void MaterialManager::update(unsigned int frameIndex, const std::map<std::string, std::vector<mat4>>& transforms) {
  for (const auto& [material, matrices] : transforms)
    m_materials.at(material).updateTransforms(frameIndex, matrices);

  for (auto& [tag, material] : m_materials)
    material.updateMutables(frameIndex);
}

Material::Output Material::operator*() const {
  return { m_pipeline, m_layout, m_sets };
}

void Material::load(const Engine& engine, const MaterialManager::Builder& builder, const std::vector<mat4>& matrices) {
  createLayout(engine, builder);
  createPipeline(engine, builder);
  createDescriptors(engine, matrices, builder);
  createSets(engine, builder);
}

void Material::batchMutable(void * data, unsigned int size, const std::vector<unsigned int>& offsets) {
  m_mutableUpdates.emplace(data, std::make_pair(size, offsets));
}

void Material::updateTransforms(unsigned int frameIndex, const std::vector<mat4>& matrices) {
  memcpy(reinterpret_cast<char *>(m_transformMap) + m_transformOffsets[frameIndex], matrices.data(), sizeof(mat4) * matrices.size());
}

void Material::updateMutables(unsigned int frameIndex) {
  for (auto& [data, info] : m_mutableUpdates) {
    auto& [size, offsets] = info;
    memcpy(reinterpret_cast<char *>(m_mutableMap) + offsets[frameIndex], data, size);
  }
  m_mutableUpdates.clear();
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

void Material::createLayout(const Engine& engine, const MaterialManager::Builder& builder) {
  std::vector<vk::DescriptorSetLayoutBinding> bindings = {vk::DescriptorSetLayoutBinding{
    .binding          = 0,
    .descriptorType   = vk::DescriptorType::eStorageBuffer,
    .descriptorCount  = 1,
    .stageFlags       = vk::ShaderStageFlagBits::eVertex
  }};

  unsigned int binding = 1;
  for (const auto * buffer : builder.m_mutables) {
    bindings.emplace_back(vk::DescriptorSetLayoutBinding{
      .binding          = binding++,
      .descriptorType   = vk::DescriptorType::eStorageBuffer,
      .descriptorCount  = 1,
      .stageFlags       = vk::ShaderStageFlagBits::eAll
    });
  }

  m_setLayout = engine.m_context.device().createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo{
    .bindingCount = static_cast<unsigned int>(bindings.size()),
    .pBindings    = bindings.data()
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

void Material::createDescriptors(const Engine& engine, const std::vector<mat4>& matrices, const MaterialManager::Builder& builder) {
  std::vector<vk::BufferCreateInfo> transformInfos;
  std::vector<vk::BufferCreateInfo> mutableInfos;

  for (unsigned int i = 0; i < engine.m_settings.buffer_mode; ++i) {
    transformInfos.emplace_back(vk::BufferCreateInfo{
      .size   = sizeof(mat4) * matrices.size(),
      .usage  = vk::BufferUsageFlagBits::eStorageBuffer,
    });

    for (auto * buffer : builder.m_mutables) {
      mutableInfos.emplace_back(vk::BufferCreateInfo{
        .size   = buffer->m_size,
        .usage  = vk::BufferUsageFlagBits::eStorageBuffer
      });
      if (i == 0) buffer->m_material = this;
    }
  }

  auto [tmp_transformMem, tmp_transformBufs, tmp_transformOffs, transformSize] = Allocator::bufferPool(engine, transformInfos,
    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
  );
  m_transformMemory = std::move(tmp_transformMem);
  m_transformBuffers = std::move(tmp_transformBufs);
  m_transformOffsets = std::move(tmp_transformOffs);

  m_transformMap = m_transformMemory.mapMemory(0, transformSize);

  auto [tmp_mutableMem, tmp_mutableBufs, mutableOffs, mutableSize] = Allocator::bufferPool(engine, mutableInfos,
    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
  );
  m_mutableMemory = std::move(tmp_mutableMem);
  m_mutableBuffers = std::move(tmp_mutableBufs);

  m_mutableMap = m_mutableMemory.mapMemory(0, mutableSize);

  for (unsigned int i = 0; i < engine.m_settings.buffer_mode; ++i) {
    updateTransforms(i, matrices);

    unsigned int j = 0;
    for (auto * buffer : builder.m_mutables) {
      buffer->m_offsets.emplace_back(mutableOffs[i * builder.m_mutables.size() + j]);
      buffer->initialize(reinterpret_cast<char *>(m_mutableMap) + mutableOffs[i * builder.m_mutables.size() + j]);
      ++j;
    }
  }

}

void Material::createSets(const Engine& engine, const MaterialManager::Builder& builder) {
  unsigned int mutableCount = engine.m_settings.buffer_mode * (1 + builder.m_mutables.size());

  auto [tmp_pool, tmp_sets] = Allocator::descriptorPool(engine, m_setLayout,
    mutableCount,
    0,
    0,
    0
  );
  m_pool = std::move(tmp_pool);
  m_sets = std::move(tmp_sets);

  std::vector<vk::DescriptorBufferInfo> bufferInfos;
  bufferInfos.reserve(mutableCount);

  std::vector<vk::WriteDescriptorSet> writes;
  writes.reserve(mutableCount);

  for (unsigned int i = 0; i < engine.m_settings.buffer_mode; ++i) {
    bufferInfos.emplace_back(vk::DescriptorBufferInfo{
      .buffer = *m_transformBuffers[i],
      .offset = 0,
      .range  = vk::WholeSize
    });

    writes.emplace_back(vk::WriteDescriptorSet{
      .dstSet           = m_sets[i],
      .dstBinding       = 0,
      .descriptorCount  = 1,
      .descriptorType   = vk::DescriptorType::eStorageBuffer,
      .pBufferInfo      = &bufferInfos.back()
    });

    unsigned int binding = 1;
    for (auto * buffer : builder.m_mutables) {
      bufferInfos.emplace_back(vk::DescriptorBufferInfo{
        .buffer = *m_mutableBuffers[i * builder.m_mutables.size() + binding - 1],
        .offset = 0,
        .range  = vk::WholeSize
      });

      writes.emplace_back(vk::WriteDescriptorSet{
        .dstSet           = m_sets[i],
        .dstBinding       = binding++,
        .descriptorCount  = 1,
        .descriptorType   = vk::DescriptorType::eStorageBuffer,
        .pBufferInfo      = &bufferInfos.back()
      });
    }
  }

  engine.m_context.device().updateDescriptorSets(writes, nullptr);
}

} // namespace ge