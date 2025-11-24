#include "src/include/allocator.hpp"
#include "src/include/engine.hpp"
#include "src/include/log.hpp"
#include "src/include/shader_compiler.hpp"
#include "src/include/vulkan_context.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_core.h"

#include <GLFW/glfw3.h>

#include <chrono>

namespace groot {

Engine::Engine(Settings settings) : m_settings(settings) {
  if (!glfwInit())
    Log::runtime_error("failed to initialize GLFW");

  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  auto [width, height] = m_settings.window_size;
  m_window = glfwCreateWindow(width, height, m_settings.window_title.c_str(), nullptr, nullptr);
  if (m_window == nullptr)
    Log::runtime_error("failed to create GLFW window");

  m_context = new VulkanContext(m_settings.application_name, m_settings.application_version);

  m_context->createSurface(m_window);

  std::vector<const char *> requiredExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
  };

  m_context->chooseGPU(m_settings.gpu_index, requiredExtensions);
  m_context->createDevice(requiredExtensions);
  m_context->printInfo();

  m_allocator = new Allocator(m_context, m_context->gpu().getProperties().apiVersion);

  m_compiler = new ShaderCompiler();
}

Engine::~Engine() {
  for (auto& [rid, handle] : m_resources) {
    switch (rid.m_type) {
      case ResourceType::Invalid:
        break;
      case ResourceType::Shader:
        m_context->device().destroyShaderModule(reinterpret_cast<VkShaderModule>(handle));
        break;
      case ResourceType::Pipeline: {
        PipelineHandle * pipeline = reinterpret_cast<PipelineHandle *>(handle);

        m_context->device().destroyPipelineLayout(pipeline->layout);
        m_context->device().destroyPipeline(pipeline->pipeline);
        delete pipeline;

        break;
      }
      case ResourceType::DescriptorSet: {
        DescriptorSetHandle * set = reinterpret_cast<DescriptorSetHandle *>(handle);

        m_context->device().destroyDescriptorSetLayout(set->layout);
        m_context->device().destroyDescriptorPool(set->pool);
        delete set;

        break;
      }
      case ResourceType::UniformBuffer:
      case ResourceType::StorageBuffer:
        m_allocator->destroyBuffer(reinterpret_cast<VkBuffer>(handle));
        break;
      case ResourceType::Image: {
        ImageHandle * image = reinterpret_cast<ImageHandle *>(handle);

        m_context->device().destroyImageView(image->view);
        m_allocator->destroyImage(image->image);
        delete image;

        break;
      }
    }
  }

  delete m_allocator;
  delete m_context;

  glfwDestroyWindow(m_window);
  glfwTerminate();
}

void Engine::run(std::function<void(double)> code) {
  while (!glfwWindowShouldClose(m_window)) {
    updateTimes();
    glfwPollEvents();

    while (m_accumulator >= m_settings.time_step) {
      code(m_settings.time_step);
      m_accumulator -= m_settings.time_step;
    }
  }
}

RID Engine::create_uniform_buffer(unsigned int size) {
  if (size == 0) {
    Log::warn("cannot create buffer with size 0");
    return RID();
  }

  vk::Buffer buffer = m_allocator->allocateBuffer(vk::BufferCreateInfo{
    .size         = size,
    .usage        = vk::BufferUsageFlagBits::eUniformBuffer,
    .sharingMode  = vk::SharingMode::eExclusive
  });

  RID rid = RID(m_nextRID++, ResourceType::UniformBuffer);
  m_resources[rid] = reinterpret_cast<unsigned long>(static_cast<VkBuffer>(buffer));

  return rid;
}

RID Engine::create_storage_buffer(unsigned int size) {
  if (size == 0) {
    Log::warn("cannot create buffer with size 0");
    return RID();
  }

  vk::Buffer buffer = m_allocator->allocateBuffer(vk::BufferCreateInfo{
    .size         = size,
    .usage        = vk::BufferUsageFlagBits::eStorageBuffer,
    .sharingMode  = vk::SharingMode::eExclusive
  });

  RID rid = RID(m_nextRID++, ResourceType::StorageBuffer);
  m_resources[rid] = reinterpret_cast<unsigned long>(static_cast<VkBuffer>(buffer));

  return rid;
}

void Engine::destroy_buffer(RID& rid) {
  if (!rid.is_valid()) {
    Log::warn("tried to destroy a buffer with an invalid RID");
    return;
  }

  if (rid.m_type != ResourceType::UniformBuffer && rid.m_type != ResourceType::StorageBuffer) {
    Log::warn("tried to destroy buffer of a non-buffer resource");
    return;
  }

  vk::Buffer buffer = reinterpret_cast<VkBuffer>(m_resources.at(rid));
  m_allocator->destroyBuffer(buffer);
  m_resources.erase(rid);

  rid.invalidate();
}

RID Engine::create_storage_image(unsigned int width, unsigned int height, Format format) {
  if (format == Format::undefined) {
    Log::warn("tried to create storage image with undefined format");
    return RID();
  }

  if (width == 0) {
    Log::warn("tried to create storage image with width 0");
    return RID();
  }

  if (height == 0) {
    Log::warn("tried to create storage image with height 0");
    return RID();
  }

  vk::ImageCreateInfo imageCreateInfo{
    .imageType    = vk::ImageType::e2D,
    .format       = static_cast<vk::Format>(format),
    .extent       = vk::Extent3D{ width, height, 1 },
    .mipLevels    = 1,
    .arrayLayers  = 1,
    .samples      = vk::SampleCountFlagBits::e1,
    .tiling       = vk::ImageTiling::eOptimal,
    .usage        = vk::ImageUsageFlagBits::eStorage,
    .sharingMode  = vk::SharingMode::eExclusive
  };

  vk::Image image = m_allocator->allocateImage(imageCreateInfo);

  vk::ImageViewCreateInfo viewCreateInfo{
    .image = image,
    .viewType = vk::ImageViewType::e2D,
    .format = static_cast<vk::Format>(format),
    .components = {
      .r = vk::ComponentSwizzle::eIdentity,
      .g = vk::ComponentSwizzle::eIdentity,
      .b = vk::ComponentSwizzle::eIdentity,
      .a = vk::ComponentSwizzle::eIdentity
    },
    .subresourceRange = {
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .levelCount = 1,
      .layerCount = 1
    }
  };

  vk::ImageView view = m_context->device().createImageView(viewCreateInfo);

  ImageHandle * handle = new ImageHandle;
  handle->image = image;
  handle->view = view;

  RID rid(m_nextRID++, ResourceType::Image);
  m_resources[rid] = reinterpret_cast<unsigned long>(handle);

  return rid;
}

void Engine::destroy_image(RID& rid) {
  if (!rid.is_valid()) {
    Log::warn("tried to destroy invalid image RID");
    return;
  }

  if (rid.m_type != ResourceType::Image) {
    Log::warn("tried to destroy image of non-image RID");
    return;
  }

  ImageHandle * image = reinterpret_cast<ImageHandle *>(m_resources.at(rid));

  m_context->device().destroyImageView(image->view);
  m_allocator->destroyImage(image->image);
  delete image;

  m_resources.erase(rid);

  rid.invalidate();
}

RID Engine::compile_shader(ShaderType type, const std::string& path) {
  std::vector<unsigned int> code = m_compiler->compileShader(type, path);
  if (code.empty()) return RID();

  vk::ShaderModuleCreateInfo createInfo{
    .codeSize = code.size() * sizeof(unsigned int),
    .pCode = code.data()
  };

  vk::ShaderModule module = m_context->device().createShaderModule(createInfo);

  Log::generic(std::format("compiled {}", path));

  RID rid = RID(m_nextRID++, ResourceType::Shader);
  m_resources[rid] = reinterpret_cast<unsigned long>(static_cast<VkShaderModule>(module));

  return rid;
}

void Engine::destroy_shader(RID& rid) {
  if (!rid.is_valid()) {
    Log::warn("tried to destroy shader with an invalid RID");
    return;
  }

  if (rid.m_type != ResourceType::Shader) {
    Log::warn("tried to destroy shader of a non-shader resource");
    return;
  }

  vk::ShaderModule module = reinterpret_cast<VkShaderModule>(m_resources.at(rid));
  m_context->device().destroyShaderModule(module);
  m_resources.erase(rid);

  rid.invalidate();
}

RID Engine::create_descriptor_set(const std::vector<RID>& descriptors) {
  std::vector<vk::DescriptorPoolSize> poolSizes = {};

  std::vector<vk::DescriptorBufferInfo> bufferInfos;
  bufferInfos.reserve(descriptors.size());

  std::vector<vk::DescriptorImageInfo> imageInfos;
  imageInfos.reserve(descriptors.size());

  std::vector<vk::WriteDescriptorSet> writes;

  unsigned int binding = 0;
  int uniformPoolIndex = -1, storagePoolIndex = -1, imagePoolIndex = -1;
  std::vector<vk::DescriptorSetLayoutBinding> bindings = {};
  for (const auto& descriptor : descriptors) {
    switch (descriptor.m_type) {
      case UniformBuffer:
        if (uniformPoolIndex == -1) {
          uniformPoolIndex = poolSizes.size();
          poolSizes.emplace_back(vk::DescriptorPoolSize{
            .type = vk::DescriptorType::eUniformBuffer
          });
        }
        ++poolSizes[uniformPoolIndex].descriptorCount;

        bindings.emplace_back(vk::DescriptorSetLayoutBinding{
          .binding          = binding,
          .descriptorType   = vk::DescriptorType::eUniformBuffer,
          .descriptorCount  = 1
        });

        bufferInfos.emplace_back(vk::DescriptorBufferInfo{
          .buffer = reinterpret_cast<VkBuffer>(m_resources.at(descriptor)),
          .range  = vk::WholeSize
        });

        writes.emplace_back(vk::WriteDescriptorSet{
          .dstSet           = nullptr,
          .dstBinding       = binding++,
          .descriptorCount  = 1,
          .descriptorType   = vk::DescriptorType::eUniformBuffer,
          .pBufferInfo      = &bufferInfos.back()
        });

        break;
      case StorageBuffer:
        if (storagePoolIndex == -1) {
          storagePoolIndex = poolSizes.size();
          poolSizes.emplace_back(vk::DescriptorPoolSize{
            .type = vk::DescriptorType::eStorageBuffer
          });
        }
        ++poolSizes[storagePoolIndex].descriptorCount;

        bindings.emplace_back(vk::DescriptorSetLayoutBinding{
          .binding          = binding,
          .descriptorType   = vk::DescriptorType::eStorageBuffer,
          .descriptorCount  = 1
        });

        bufferInfos.emplace_back(vk::DescriptorBufferInfo{
          .buffer = reinterpret_cast<VkBuffer>(m_resources.at(descriptor)),
          .range  = vk::WholeSize
        });

        writes.emplace_back(vk::WriteDescriptorSet{
          .dstSet           = nullptr,
          .dstBinding       = binding++,
          .descriptorCount  = 1,
          .descriptorType   = vk::DescriptorType::eStorageBuffer,
          .pBufferInfo      = &bufferInfos.back()
        });

        break;
      case Image: {
        if (imagePoolIndex == -1) {
          imagePoolIndex = poolSizes.size();
          poolSizes.emplace_back(vk::DescriptorPoolSize{
            .type = vk::DescriptorType::eStorageImage
          });
        }
        ++poolSizes[imagePoolIndex].descriptorCount;

        bindings.emplace_back(vk::DescriptorSetLayoutBinding{
          .binding          = binding,
          .descriptorType   = vk::DescriptorType::eStorageImage,
          .descriptorCount  = 1
        });

        ImageHandle * image = reinterpret_cast<ImageHandle *>(m_resources.at(descriptor));

        imageInfos.emplace_back(vk::DescriptorImageInfo{
          .imageView    = image->view,
          .imageLayout = vk::ImageLayout::eGeneral
        });

        writes.emplace_back(vk::WriteDescriptorSet{
          .dstSet           = nullptr,
          .dstBinding       = binding++,
          .descriptorCount  = 1,
          .descriptorType   = vk::DescriptorType::eStorageImage,
          .pImageInfo       = &imageInfos.back()
        });

        break;
      }
      case Invalid:
        Log::warn("invalid RID given as a descriptor");
        return RID();
      default:
        Log::warn("non-buffer/image/sampler RID given as a descriptor");
        return RID();
    }
  }

  vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{
    .bindingCount = static_cast<unsigned int>(bindings.size()),
    .pBindings    = bindings.data(),
  };

  DescriptorSetHandle * set = new DescriptorSetHandle;
  set->layout = m_context->device().createDescriptorSetLayout(layoutCreateInfo);

  vk::DescriptorPoolCreateInfo poolCreateInfo{
    .maxSets        = 1,
    .poolSizeCount  = static_cast<unsigned int>(poolSizes.size()),
    .pPoolSizes     = poolSizes.data(),
  };

  set->pool = m_context->device().createDescriptorPool(poolCreateInfo);

  vk::DescriptorSetAllocateInfo allocateInfo{
    .descriptorPool     = set->pool,
    .descriptorSetCount = 1,
    .pSetLayouts        = &set->layout
  };

  set->set = m_context->device().allocateDescriptorSets(allocateInfo)[0];

  for (auto& write : writes)
    write.dstSet = set->set;

  m_context->device().updateDescriptorSets(writes, nullptr);

  RID rid(m_nextRID++, ResourceType::DescriptorSet);
  m_resources[rid] = reinterpret_cast<unsigned long>(set);

  return rid;
}

void Engine::destroy_descriptor_set(RID& rid) {
  if (!rid.is_valid()) {
    Log::warn("tried to destroy descriptor set of invalid RID");
    return;
  }

  if (rid.m_type != ResourceType::DescriptorSet) {
    Log::warn("tried to detroy descriptor set of non-descripor-set RID");
    return;
  }

  DescriptorSetHandle * set = reinterpret_cast<DescriptorSetHandle *>(m_resources.at(rid));
  m_context->device().destroyDescriptorSetLayout(set->layout);
  m_context->device().destroyDescriptorPool(set->pool);
  delete set;

  m_resources.erase(rid);

  rid.invalidate();
}

RID Engine::create_compute_pipeline(const RID& shader, const RID& descriptorSet) {
  if (!shader.is_valid()) {
    Log::warn("tried to make compute pipeline with invalid RID");
    return RID();
  }

  if (shader.m_type != ResourceType::Shader) {
    Log::warn("tried to make compute pipeline with non-shader RID");
    return RID();
  }

  if (!descriptorSet.is_valid()) {
    Log::warn("tried to make compute pipeline with invalid descriptor set");
    return RID();
  }

  if (descriptorSet.m_type != ResourceType::DescriptorSet) {
    Log::warn("tried to make compute pipeline with non-descriptor-set RID");
    return RID();
  }

  vk::PipelineShaderStageCreateInfo shaderStage{
    .stage  = vk::ShaderStageFlagBits::eCompute,
    .module = reinterpret_cast<VkShaderModule>(m_resources.at(shader)),
    .pName  = "main"
  };

  DescriptorSetHandle * set = reinterpret_cast<DescriptorSetHandle *>(m_resources.at(descriptorSet));

  vk::PipelineLayoutCreateInfo layoutCreateInfo{
    .setLayoutCount = 0,
    .pSetLayouts    = &set->layout
  };

  PipelineHandle * pipeline = new PipelineHandle;
  pipeline->layout = m_context->device().createPipelineLayout(layoutCreateInfo);

  vk::ComputePipelineCreateInfo pipelineCreateInfo{
    .stage  = shaderStage,
    .layout = pipeline->layout
  };

  vk::ResultValue<vk::Pipeline> result = m_context->device().createComputePipeline(nullptr, pipelineCreateInfo);
  if (!result.has_value()) {
    Log::warn("failed to create compute pipeline");

    m_context->device().destroyPipelineLayout(pipeline->layout);
    delete pipeline;

    return RID();
  }

  pipeline->pipeline = result.value;

  RID rid(m_nextRID++, ResourceType::Pipeline);
  m_resources[rid] = reinterpret_cast<unsigned long>(pipeline);

  return rid;
}

RID Engine::create_graphics_pipeline(const GraphicsPipelineShaders& shaders, const RID& descriptorSet, const GraphicsPipelineSettings& s) {
  if (!shaders.vertex.is_valid()) {
    Log::warn("invalid vertex shader RID");
    return {};
  }

  if (shaders.vertex.m_type != ResourceType::Shader) {
    Log::warn("vertex RID is not a shader RID");
    return {};
  }

  if (!shaders.fragment.is_valid()) {
    Log::warn("invalid fragment shader RID");
    return {};
  }

  if (shaders.fragment.m_type != ResourceType::Shader) {
    Log::warn("fragment RID is not a shader RID");
    return {};
  }

  if (shaders.tesselation_control.is_valid() && !shaders.tesselation_evaluation.is_valid()) {
    Log::warn("found valid tesselation control shader RID but invalid tesselation evaluation shader RID");
    return {};
  }

  if (shaders.tesselation_control.is_valid() && shaders.tesselation_control.m_type != ResourceType::Shader) {
    Log::warn("tesselation control RID is not a shader RID");
    return {};
  }

  if (shaders.tesselation_evaluation.is_valid() && shaders.tesselation_evaluation.m_type != ResourceType::Shader) {
    Log::warn("tesselation evaluation RID is not a shader RID");
    return {};
  }

  if ((shaders.tesselation_control.is_valid() || shaders.tesselation_evaluation.is_valid()) && !m_context->supportsTesselation()) {
    Log::warn("tesselation shaders are not supported on this GPU and will be skipped during pipeline creation");
  }

  if (!descriptorSet.is_valid()) {
    Log::warn("tried to create graphics pipeline with invalid descriptor set RID");
    return RID();
  }

  if (descriptorSet.m_type != ResourceType::DescriptorSet) {
    Log::warn("tried to create graphics pipeline with non-descriptor-set RID");
    return RID();
  }

  std::unordered_map<vk::ShaderStageFlagBits, vk::ShaderModule> modules;
  modules.emplace(vk::ShaderStageFlagBits::eVertex, reinterpret_cast<VkShaderModule>(m_resources.at(shaders.vertex)));
  modules.emplace(vk::ShaderStageFlagBits::eFragment, reinterpret_cast<VkShaderModule>(m_resources.at(shaders.fragment)));

  if (shaders.tesselation_control.is_valid() && m_context->supportsTesselation())
    modules.emplace(vk::ShaderStageFlagBits::eTessellationControl, reinterpret_cast<VkShaderModule>(m_resources.at(shaders.tesselation_evaluation)));

  if (shaders.tesselation_control.is_valid() && m_context->supportsTesselation())
    modules.emplace(vk::ShaderStageFlagBits::eTessellationEvaluation, reinterpret_cast<VkShaderModule>(m_resources.at(shaders.tesselation_evaluation)));

  std::vector<vk::PipelineShaderStageCreateInfo> stages = {};
  for (const auto& [stage, module] : modules) {
    stages.emplace_back(vk::PipelineShaderStageCreateInfo{
      .stage  = stage,
      .module = module,
      .pName  = "main"
    });
  }

  DescriptorSetHandle * set = reinterpret_cast<DescriptorSetHandle *>(m_resources.at(descriptorSet));

  vk::PipelineLayoutCreateInfo layoutCreateInfo{
    .setLayoutCount = 1,
    .pSetLayouts    = &set->layout
  };

  PipelineHandle * pipeline = new PipelineHandle;
  pipeline->layout = m_context->device().createPipelineLayout(layoutCreateInfo);

  std::array<vk::DynamicState, 2> dynamicStates = {
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor
  };

  vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{
    .dynamicStateCount  = 2,
    .pDynamicStates     = dynamicStates.data()
  };

  vk::PipelineViewportStateCreateInfo viewportStateCreateInfo{
    .viewportCount  = 1,
    .scissorCount   = 1
  };

  vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo{
    .vertexBindingDescriptionCount    = static_cast<unsigned int>(s.vertex_bindings.size()),
    .pVertexBindingDescriptions       = reinterpret_cast<const vk::VertexInputBindingDescription *>(s.vertex_bindings.data()),
    .vertexAttributeDescriptionCount  = static_cast<unsigned int>(s.vertex_attributes.size()),
    .pVertexAttributeDescriptions     = reinterpret_cast<const vk::VertexInputAttributeDescription *>(s.vertex_attributes.data())
  };

  vk::PipelineInputAssemblyStateCreateInfo assemblyCreateInfo{
    .topology               = vk::PrimitiveTopology::eTriangleList,
    .primitiveRestartEnable = false
  };


  vk::PolygonMode polygonMode = static_cast<vk::PolygonMode>(s.mesh_type);
  if (s.mesh_type != MeshType::Solid && !m_context->supportsNonSolidMesh()) {
    Log::warn("setting pipeline mesh type to solid. GPU does not support non solid meshes");
    polygonMode = vk::PolygonMode::eFill;
  }

  vk::PipelineRasterizationStateCreateInfo rasterizerCreateInfo{
    .depthClampEnable         = false,
    .rasterizerDiscardEnable  = false,
    .polygonMode              = polygonMode,
    .cullMode                 = static_cast<vk::CullModeFlagBits>(s.cull_mode),
    .frontFace                = static_cast<vk::FrontFace>(s.draw_direction),
    .depthBiasEnable          = false,
    .lineWidth                = 1.0f
  };

  vk::PipelineMultisampleStateCreateInfo multisampleCreateInfo{
    .rasterizationSamples = vk::SampleCountFlagBits::e1,
    .sampleShadingEnable  = false
  };

  vk::PipelineDepthStencilStateCreateInfo depthCreateInfo{
    .depthTestEnable        = s.enable_depth,
    .depthWriteEnable       = s.enable_depth,
    .depthCompareOp         = vk::CompareOp::eLess,
    .depthBoundsTestEnable  = false,
    .stencilTestEnable      = false
  };

  vk::PipelineColorBlendAttachmentState colorAttachmentState{
    .blendEnable          = s.enable_blend,
    .srcColorBlendFactor  = vk::BlendFactor::eSrcAlpha,
    .dstColorBlendFactor  = vk::BlendFactor::eOneMinusSrcAlpha,
    .colorBlendOp         = vk::BlendOp::eAdd,
    .srcAlphaBlendFactor  = vk::BlendFactor::eOneMinusSrcAlpha,
    .alphaBlendOp         = vk::BlendOp::eAdd,
    .colorWriteMask       = vk::ColorComponentFlagBits::eR |
                            vk::ColorComponentFlagBits::eG |
                            vk::ColorComponentFlagBits::eB |
                            vk::ColorComponentFlagBits::eA
  };

  vk::PipelineColorBlendStateCreateInfo blendStateCreateInfo{
    .logicOpEnable    = false,
    .attachmentCount  = 1,
    .pAttachments     = &colorAttachmentState
  };

  vk::PipelineRenderingCreateInfo renderingCreateInfo{
    .colorAttachmentCount     = 1,
    .pColorAttachmentFormats  = reinterpret_cast<vk::Format *>(&m_settings.color_format),
    .depthAttachmentFormat    = static_cast<vk::Format>(m_settings.depth_format)
  };

  vk::GraphicsPipelineCreateInfo pipelineCreateInfo{
    .pNext                = &renderingCreateInfo,
    .stageCount           = static_cast<unsigned int>(stages.size()),
    .pStages              = stages.data(),
    .pVertexInputState    = &vertexInputCreateInfo,
    .pInputAssemblyState  = &assemblyCreateInfo,
    .pViewportState       = &viewportStateCreateInfo,
    .pRasterizationState  = &rasterizerCreateInfo,
    .pMultisampleState    = &multisampleCreateInfo,
    .pDepthStencilState   = &depthCreateInfo,
    .pColorBlendState     = &blendStateCreateInfo,
    .pDynamicState        = &dynamicStateCreateInfo,
    .layout               = pipeline->layout
  };

  vk::ResultValue<vk::Pipeline> result = m_context->device().createGraphicsPipeline(nullptr, pipelineCreateInfo);
  if (!result.has_value()) {
    Log::warn("failed to create graphics pipeline");

    m_context->device().destroyPipelineLayout(pipeline->layout);
    delete pipeline;

    return RID();
  }

  pipeline->pipeline = result.value;

  RID rid(m_nextRID++, ResourceType::Pipeline);
  m_resources[rid] = reinterpret_cast<unsigned long>(pipeline);

  return rid;
}

void Engine::destroy_pipeline(RID& rid) {
  if (!rid.is_valid()) {
    Log::warn("tried to destroy invalid pipeline RID");
    return;
  }

  if (rid.m_type != ResourceType::Pipeline) {
    Log::warn("tried to destroy pipeline of non-pipeline resource");
    return;
  }

  PipelineHandle * pipeline = reinterpret_cast<PipelineHandle *>(m_resources.at(rid));

  m_context->device().destroyPipelineLayout(pipeline->layout);
  m_context->device().destroyPipeline(pipeline->pipeline);
  delete pipeline;

  m_resources.erase(rid);

  rid.invalidate();
}

void Engine::updateTimes() {
  static std::chrono::time_point start = std::chrono::high_resolution_clock::now();
  std::chrono::time_point now = std::chrono::high_resolution_clock::now();

  double time = std::chrono::duration(now - start).count();

  m_frameTime = std::min(time - m_time, 0.25);
  m_time = time;
  m_accumulator += m_frameTime;
}

} // namespace groot