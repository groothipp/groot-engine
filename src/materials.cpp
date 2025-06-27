#include "src/include/allocator.hpp"
#include "src/include/buffer.hpp"
#include "src/include/engine.hpp"
#include "src/include/materials.hpp"
#include "src/include/parsers.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace ge {

MaterialManager::Builder& MaterialManager::Builder::add_shader(ShaderStage stage, const std::string& path) {
  m_shaders[static_cast<vk::ShaderStageFlagBits>(stage)] = path;
  return *this;
}

MaterialManager::Builder& MaterialManager::Builder::add_mutable(BufferProxy * buffer) {
  m_mutables.emplace_back(buffer);
  return *this;
}

MaterialManager::Builder& MaterialManager::Builder::add_immutable(unsigned int size, void * data) {
  m_immutables.emplace_back(std::make_pair(size, data));
  return *this;
}

MaterialManager::Builder& MaterialManager::Builder::add_texture(const std::string& path) {
  m_texturePaths.emplace_back(path);
  return *this;
}

MaterialManager::Builder& MaterialManager::Builder::add_canvas() {
  ++m_canvasCount;
  return *this;
}

MaterialManager::Builder& MaterialManager::Builder::compute_space(unsigned int x, unsigned int y) {
  m_computeSpace = { x, y };
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
  const auto& [pipeline, compute, layout, sets, x, y] = *m_iterator->second;

  return { m_iterator->first, pipeline, compute, layout, sets, x, y };
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

void MaterialManager::transitionImages(vk::raii::CommandBuffer& computeCmd, vk::raii::CommandBuffer& renderCmd) const {
  for (auto& [tag, material] : m_materials)
    material.transitionImages(computeCmd, renderCmd);
}

void MaterialManager::add(const std::string& tag, Builder& builder) {
  if (m_materials.contains(tag))
    throw std::runtime_error("groot-engine: tried to add material with tag that already exists");

  for (const auto& path : builder.m_texturePaths) {
    if (!m_textureMap.contains(path))
      m_textureMap.emplace(path, PNGParser::parse(path));
    builder.m_textures.emplace_back(m_textureMap.at(path));
  }

  m_builders[tag] = builder;
  m_materials[tag] = Material();
}

void MaterialManager::load(const Engine& engine, const std::map<std::string, std::vector<mat4>>& transforms) {
  for (const auto& [material, matrices] : transforms)
    m_materials.at(material).load(engine, m_builders.at(material), matrices);
  m_builders.clear();
  m_textureMap.clear();
}

void MaterialManager::update(unsigned int frameIndex, const std::map<std::string, std::vector<mat4>>& transforms) {
  for (const auto& [material, matrices] : transforms)
    m_materials.at(material).updateTransforms(frameIndex, matrices);

  for (auto& [tag, material] : m_materials)
    material.updateMutables(frameIndex);
}

Material::Output Material::operator*() const {
  return { m_pipeline, m_compute, m_layout, m_sets, m_computeSpace.first, m_computeSpace.second };
}

void Material::transitionImages(vk::raii::CommandBuffer& computeCmd, vk::raii::CommandBuffer& renderCmd) const {
  if (m_canvases.empty()) return;

  std::vector<vk::ImageMemoryBarrier> computeBarriers;
  std::vector<vk::ImageMemoryBarrier> fragmentBarriers;

  for (auto& image : m_canvases) {
    computeBarriers.emplace_back(vk::ImageMemoryBarrier{
      .dstAccessMask    = vk::AccessFlagBits::eShaderWrite,
      .newLayout        = vk::ImageLayout::eGeneral,
      .image            = image,
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    });

    fragmentBarriers.emplace_back(vk::ImageMemoryBarrier{
      .srcAccessMask    = vk::AccessFlagBits::eShaderWrite,
      .dstAccessMask    = vk::AccessFlagBits::eShaderRead,
      .oldLayout        = vk::ImageLayout::eGeneral,
      .newLayout        = vk::ImageLayout::eShaderReadOnlyOptimal,
      .image            = image,
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    });
  }

  computeCmd.pipelineBarrier(
    vk::PipelineStageFlagBits::eTopOfPipe,
    vk::PipelineStageFlagBits::eComputeShader,
    vk::DependencyFlags(),
    nullptr,
    nullptr,
    computeBarriers
  );

  renderCmd.pipelineBarrier(
    vk::PipelineStageFlagBits::eComputeShader,
    vk::PipelineStageFlagBits::eFragmentShader,
    vk::DependencyFlags(),
    nullptr,
    nullptr,
    fragmentBarriers
  );
}

void Material::load(const Engine& engine, MaterialManager::Builder& builder, const std::vector<mat4>& matrices) {
  createLayout(engine, builder);
  m_computeSpace = builder.m_computeSpace;

  if (builder.m_shaders.contains(vk::ShaderStageFlagBits::eCompute)) {
    auto code = SPVParser::parse(builder.m_shaders.at(vk::ShaderStageFlagBits::eCompute));
    builder.m_shaders.erase(vk::ShaderStageFlagBits::eCompute);

    vk::raii::ShaderModule computeModule = engine.m_context.device().createShaderModule(vk::ShaderModuleCreateInfo{
      .codeSize = static_cast<unsigned int>(code.size()),
      .pCode    = reinterpret_cast<unsigned int *>(code.data())
    });

    vk::PipelineShaderStageCreateInfo computeStage{
      .stage  = vk::ShaderStageFlagBits::eCompute,
      .module = computeModule,
      .pName  = "main"
    };

    m_compute = engine.m_context.device().createComputePipeline(nullptr, vk::ComputePipelineCreateInfo{
      .stage  = computeStage,
      .layout = m_layout
    });
  }

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
  if (!builder.m_textures.empty()) {
    bindings.emplace_back(vk::DescriptorSetLayoutBinding{
      .binding          = binding++,
      .descriptorType   = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount  = static_cast<unsigned int>(builder.m_textures.size()),
      .stageFlags       = vk::ShaderStageFlagBits::eFragment
    });
  }

  if (builder.m_canvasCount != 0) {
    bindings.emplace_back(vk::DescriptorSetLayoutBinding{
      .binding          = binding++,
      .descriptorType   = vk::DescriptorType::eStorageImage,
      .descriptorCount  = builder.m_canvasCount,
      .stageFlags       = vk::ShaderStageFlagBits::eCompute
    });

    bindings.emplace_back(vk::DescriptorSetLayoutBinding{
      .binding          = binding++,
      .descriptorType   = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount  = builder.m_canvasCount,
      .stageFlags       = vk::ShaderStageFlagBits::eFragment
    });
  }

  for (const auto& immutable : builder.m_immutables) {
    bindings.emplace_back(vk::DescriptorSetLayoutBinding{
      .binding          = binding++,
      .descriptorType   = vk::DescriptorType::eStorageBuffer,
      .descriptorCount  = 1,
      .stageFlags       = vk::ShaderStageFlagBits::eAll
    });
  }

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
  std::vector<vk::BufferCreateInfo> immutableTransferInfos;
  std::vector<vk::BufferCreateInfo> immutableInfos;
  std::vector<vk::BufferCreateInfo> imageTransferInfos;
  std::vector<std::pair<unsigned int, unsigned int>> imageInfos;
  std::vector<std::pair<unsigned int, unsigned int>> canvasInfos;

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

  for (const auto& [size, data] : builder.m_immutables) {
    immutableTransferInfos.emplace_back(vk::BufferCreateInfo{
      .size   = size,
      .usage  = vk::BufferUsageFlagBits::eTransferSrc
    });

    immutableInfos.emplace_back(vk::BufferCreateInfo{
      .size   = size,
      .usage  = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst
    });
  }

  for (const auto& [width, height, size, data] : builder.m_textures) {
    imageTransferInfos.emplace_back(vk::BufferCreateInfo{
      .size   = size,
      .usage  = vk::BufferUsageFlagBits::eTransferSrc
    });

    imageInfos.emplace_back(std::make_pair(width, height));
  }

  for (unsigned int i = 0; i < builder.m_canvasCount; ++i) {
    auto [x, y] = m_computeSpace;
    canvasInfos.emplace_back(std::make_pair(8 * x, 8 * y));
  }

  auto [tmp_transformMem, tmp_transformBufs, tmp_transformOffs, transformSize] = Allocator::bufferPool(engine, transformInfos,
    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
  );
  m_transformMemory = std::move(tmp_transformMem);
  m_transformBuffers = std::move(tmp_transformBufs);
  m_transformOffsets = std::move(tmp_transformOffs);

  m_transformMap = m_transformMemory.mapMemory(0, transformSize);

  std::vector<unsigned int> mutableOffs;
  if (!mutableInfos.empty()) {
    auto [tmp_mutableMem, tmp_mutableBufs, tmp_mutableOffs, mutableSize] = Allocator::bufferPool(engine, mutableInfos,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );
    m_mutableMemory = std::move(tmp_mutableMem);
    m_mutableBuffers = std::move(tmp_mutableBufs);
    mutableOffs = std::move(tmp_mutableOffs);

    m_mutableMap = m_mutableMemory.mapMemory(0, mutableSize);
  }

  for (unsigned int i = 0; i < engine.m_settings.buffer_mode; ++i) {
    updateTransforms(i, matrices);

    unsigned int j = 0;
    for (auto * buffer : builder.m_mutables) {
      buffer->m_offsets.emplace_back(mutableOffs[i * builder.m_mutables.size() + j]);
      buffer->initialize(reinterpret_cast<char *>(m_mutableMap) + mutableOffs[i * builder.m_mutables.size() + j]);
      ++j;
    }
  }

  vk::raii::DeviceMemory immutableTMem = nullptr;
  std::vector<vk::raii::Buffer> immutableTBufs;
  if (!immutableInfos.empty()) {
    auto [tmp_immutableTMem, tmp_immutableTBufs, immutableTOffs, immutableTSize] = Allocator::bufferPool(engine, immutableTransferInfos,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );
    immutableTMem = std::move(tmp_immutableTMem);
    immutableTBufs = std::move(tmp_immutableTBufs);

    void * immutableTMap = immutableTMem.mapMemory(0, immutableTSize);
    for (unsigned int i = 0; i < immutableTOffs.size(); ++i) {
      auto& [size, data] = builder.m_immutables[i];
      memcpy(reinterpret_cast<char *>(immutableTMap) + immutableTOffs[i], data, size);
    }
    immutableTMem.unmapMemory();

    auto [tmp_immutableMem, tmp_immutableBufs, immutableOffs, immutableSize] = Allocator::bufferPool(engine, immutableInfos,
      vk::MemoryPropertyFlagBits::eDeviceLocal
    );
    m_immutableMemory = std::move(tmp_immutableMem);
    m_immutableBuffers = std::move(tmp_immutableBufs);
  }

  vk::raii::DeviceMemory imageTMem = nullptr;
  std::vector<vk::raii::Buffer> imageTBufs;
  if (!imageTransferInfos.empty()) {
    auto [tmp_imageTMem, tmp_imageTBufs, imageTOffs, imageTSize] = Allocator::bufferPool(engine, imageTransferInfos,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );
    imageTMem = std::move(tmp_imageTMem);
    imageTBufs = std::move(tmp_imageTBufs);

    void * imageTMap = imageTMem.mapMemory(0, imageTSize);
    unsigned int i = 0;
    for (auto& [width, height, size, data] : builder.m_textures)
      memcpy(reinterpret_cast<char *>(imageTMap) + imageTOffs[i++], data.data(), size);
    imageTMem.unmapMemory();

    auto [tmp_imageMem, tmp_images, tmp_views, imageOffs, imageSize] = Allocator::imagePool(engine, imageInfos,
      vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::Format::eR8G8B8A8Srgb
    );
    m_textureMemory = std::move(tmp_imageMem);
    m_textures = std::move(tmp_images);
    m_textureViews = std::move(tmp_views);
  }

  if (!canvasInfos.empty()) {
    auto [tmp_canvasMem, tmp_canvasImgs, tmp_canvasViews, _, __] = Allocator::imagePool(engine, canvasInfos,
      vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::Format::eR8G8B8A8Unorm
    );
    m_canvasMemory = std::move(tmp_canvasMem);
    m_canvases = std::move(tmp_canvasImgs);
    m_canvasViews = std::move(tmp_canvasViews);
  }

  vk::raii::CommandBuffer transferCmd = std::move(engine.getCmds(QueueFamilyType::Transfer, 1)[0]);
  transferCmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

  for (unsigned int i = 0; i < m_immutableBuffers.size(); ++i) {
    auto& [size, data] = builder.m_immutables[i];
    transferCmd.copyBuffer(immutableTBufs[i], m_immutableBuffers[i], vk::BufferCopy{ .size = size });
  }

  for (unsigned int i = 0; i < m_textures.size(); ++i) {
    auto& [width, height, size, data] = builder.m_textures[i];

    vk::ImageMemoryBarrier barrier{
      .dstAccessMask    = vk::AccessFlagBits::eTransferWrite,
      .newLayout        = vk::ImageLayout::eTransferDstOptimal,
      .image            = m_textures[i],
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    };

    transferCmd.pipelineBarrier(
      vk::PipelineStageFlagBits::eTopOfPipe,
      vk::PipelineStageFlagBits::eTransfer,
      vk::DependencyFlags(),
      nullptr,
      nullptr,
      barrier
    );

    transferCmd.copyBufferToImage(imageTBufs[i], m_textures[i], vk::ImageLayout::eTransferDstOptimal, vk::BufferImageCopy{
      .imageSubresource = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .layerCount = 1
      },
      .imageExtent = {
        .width  = width,
        .height = height,
        .depth  = 1
      }
    });

    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    transferCmd.pipelineBarrier(
      vk::PipelineStageFlagBits::eTransfer,
      vk::PipelineStageFlagBits::eFragmentShader,
      vk::DependencyFlags(),
      nullptr,
      nullptr,
      barrier
    );
  }

  transferCmd.end();

  vk::raii::Fence fence = std::move(Allocator::fences(engine, 1)[0]);

  engine.m_context.queueFamily(QueueFamilyType::Transfer).queue.submit(vk::SubmitInfo{
    .commandBufferCount = 1,
    .pCommandBuffers    = &*transferCmd
  }, fence);

  if (engine.m_context.device().waitForFences(*fence, true, ge_timeout) != vk::Result::eSuccess)
    throw std::runtime_error("groot-engine: hung waiting for immutable transfer");
}

void Material::createSets(const Engine& engine, const MaterialManager::Builder& builder) {
  unsigned int storageCount =
    engine.m_settings.buffer_mode * (1 + builder.m_mutables.size() + builder.m_immutables.size());
  unsigned int imageCount = engine.m_settings.buffer_mode * builder.m_canvasCount;
  unsigned int samplerCount = engine.m_settings.buffer_mode * (builder.m_textures.size() + builder.m_canvasCount);

  auto [tmp_pool, tmp_sets] = Allocator::descriptorPool(engine, m_setLayout,
    storageCount,
    imageCount,
    samplerCount
  );
  m_pool = std::move(tmp_pool);
  m_sets = std::move(tmp_sets);

  std::vector<vk::DescriptorBufferInfo> bufferInfos;
  bufferInfos.reserve(storageCount);

  std::vector<std::vector<vk::DescriptorImageInfo>> imageInfos;
  imageInfos.resize(engine.m_settings.buffer_mode);

  std::vector<std::vector<vk::DescriptorImageInfo>> canvasStorageInfos;
  canvasStorageInfos.resize(engine.m_settings.buffer_mode);

  std::vector<std::vector<vk::DescriptorImageInfo>> canvasSamplerInfos;
  canvasSamplerInfos.resize(engine.m_settings.buffer_mode);

  for (unsigned int i = 0; i < engine.m_settings.buffer_mode; ++i) {
    imageInfos[i].reserve(builder.m_textures.size());
    canvasStorageInfos[i].reserve(builder.m_canvasCount);
    canvasStorageInfos[i].reserve(builder.m_canvasCount);
  }

  std::vector<vk::WriteDescriptorSet> writes;
  writes.reserve(storageCount + 3 * engine.m_settings.buffer_mode);

  m_sampler = Allocator::sampler(engine);

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
    for (auto& view : m_textureViews) {
      imageInfos[i].emplace_back(vk::DescriptorImageInfo{
        .sampler      = m_sampler,
        .imageView    = view,
        .imageLayout  = vk::ImageLayout::eShaderReadOnlyOptimal
      });
    }
    if (!builder.m_textures.empty()) {
      writes.emplace_back(vk::WriteDescriptorSet{
        .dstSet           = m_sets[i],
        .dstBinding       = binding++,
        .descriptorCount  = static_cast<unsigned int>(builder.m_textures.size()),
        .descriptorType   = vk::DescriptorType::eCombinedImageSampler,
        .pImageInfo       = imageInfos[i].data()
      });
    }

    for (auto& view : m_canvasViews) {
      canvasStorageInfos[i].emplace_back(vk::DescriptorImageInfo{
        .sampler      = nullptr,
        .imageView    = view,
        .imageLayout  = vk::ImageLayout::eGeneral
      });

      canvasSamplerInfos[i].emplace_back(vk::DescriptorImageInfo{
        .sampler      = m_sampler,
        .imageView    = view,
        .imageLayout  = vk::ImageLayout::eShaderReadOnlyOptimal
      });
    }
    if (builder.m_canvasCount != 0) {
      writes.emplace_back(vk::WriteDescriptorSet{
        .dstSet           = m_sets[i],
        .dstBinding       = binding++,
        .descriptorCount  = builder.m_canvasCount,
        .descriptorType   = vk::DescriptorType::eStorageImage,
        .pImageInfo       = canvasStorageInfos[i].data()
      });

      writes.emplace_back(vk::WriteDescriptorSet{
        .dstSet           = m_sets[i],
        .dstBinding       = binding++,
        .descriptorCount  = builder.m_canvasCount,
        .descriptorType   = vk::DescriptorType::eCombinedImageSampler,
        .pImageInfo       = canvasSamplerInfos[i].data()
      });
    }

    unsigned int j = 0;
    for (auto& immutable : builder.m_immutables) {
      bufferInfos.emplace_back(vk::DescriptorBufferInfo{
        .buffer = m_immutableBuffers[j++],
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

    j = i * builder.m_mutables.size();
    for (auto * buffer : builder.m_mutables) {
      bufferInfos.emplace_back(vk::DescriptorBufferInfo{
        .buffer = *m_mutableBuffers[j++],
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