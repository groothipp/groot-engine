#include "src/include/allocator.hpp"
#include "src/include/engine.hpp"
#include "src/include/input_mananger.hpp"
#include "src/include/object.hpp"
#include "src/include/renderer.hpp"
#include "src/include/shader_compiler.hpp"
#include "src/include/stb_image.h"
#include "src/include/structs.hpp"
#include "src/include/tiny_obj_loader.h"
#include "src/include/vulkan_context.hpp"
#include "vulkan/vulkan.hpp"

#include <GLFW/glfw3.h>

#include <chrono>

namespace groot {

Engine::Engine(const Settings& settings) : m_settings(settings) {
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
  m_context->createCommandPools();
  m_context->printInfo();

  m_allocator = new Allocator(m_context, m_context->gpu().getProperties().apiVersion);
  m_compiler = new ShaderCompiler();
  m_renderer = new Renderer(m_window, m_context, m_allocator, m_settings);

  m_inputManager = new InputManager;
  glfwSetWindowUserPointer(m_window, m_inputManager);
  glfwSetKeyCallback(m_window, InputManager::keyCallback);
  glfwSetCursorPosCallback(m_window, InputManager::cursorCallback);
  glfwSetMouseButtonCallback(m_window, InputManager::mouseCallback);
  glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
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
      case ResourceType::Mesh: {
        MeshHandle * mesh = reinterpret_cast<MeshHandle *>(handle);

        m_allocator->destroyBuffer(mesh->vertexBuffer);
        m_allocator->destroyBuffer(mesh->indexBuffer);
        delete mesh;

        break;
      }
      case ResourceType::UniformBuffer:
      case ResourceType::StorageBuffer:
        m_allocator->destroyBuffer(reinterpret_cast<VkBuffer>(handle));
        break;
      case ResourceType::Sampler: {
        m_context->device().destroySampler(reinterpret_cast<VkSampler>(handle));
        break;
      }
      case ResourceType::StorageImage:
      case ResourceType::StorageTexture:
      case ResourceType::Texture: {
        ImageHandle * image = reinterpret_cast<ImageHandle *>(handle);

        m_context->device().destroyImageView(image->view);
        m_allocator->destroyImage(image->image);
        delete image;

        break;
      }
      default: continue;
    }
  }

  m_renderer->destroy(m_context, m_allocator);
  delete m_renderer;

  delete m_allocator;
  delete m_context;
  delete m_inputManager;

  glfwDestroyWindow(m_window);
  glfwTerminate();
}

mat4 Engine::camera_view() const {
  return mat4::view(m_cameraEye, m_cameraTarget, vec3(0.0f, 1.0f, 0.0f));
}

mat4 Engine::camera_projection() const {
  auto [w, h] = m_renderer->extent();
  float ar = static_cast<float>(w) / static_cast<float>(h);
  return mat4::perspective_projection(radians(m_settings.fov), ar, 0.1f, 1000.0f);
}

std::pair<unsigned int, unsigned int> Engine::viewport_dims() const {
  return m_renderer->extent();
}

void Engine::close_window() const {
  glfwSetWindowShouldClose(m_window, true);
}

bool Engine::is_pressed(Key key) const {
  return m_inputManager->pressed(key);
}

bool Engine::is_pressed(MouseButton button) const {
  return m_inputManager->pressed(button);
}

bool Engine::just_pressed(Key key) const {
  return m_inputManager->just_pressed(key);
}

bool Engine::just_pressed(MouseButton button) const {
  return m_inputManager->just_pressed(button);
}

bool Engine::just_released(Key key) const {
  return m_inputManager->just_released(key);
}

bool Engine::just_released(MouseButton button) const {
  return m_inputManager->just_released(button);
}

vec2 Engine::mouse_pos() const {
  auto [x, y] = m_inputManager->cursor();
  return vec2(x, y);
}

std::tuple<vec3, vec3, vec3> Engine::camera_basis() const {
  vec3 forward = (m_cameraTarget - m_cameraEye).normalized();
  vec3 right = forward.cross(vec3(0.0f, 1.0f, 0.0f)).normalized();
  return { forward, right, vec3(0.0f, 1.0f, 0.0f) };
}

void Engine::capture_cursor() const {
 if (glfwGetInputMode(m_window, GLFW_CURSOR) != GLFW_CURSOR_CAPTURED)
   glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
}

void Engine::release_cursor() const {
  if (glfwGetInputMode(m_window, GLFW_CURSOR) != GLFW_CURSOR_NORMAL)
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

RID Engine::render_target() {
  if (m_renderTarget == nullptr) {
    Log::warn("Tried to get render target RID when no render target has been created yet");
    return RID();
  }

  return RID(0, ResourceType::RenderTarget);
}

void Engine::translate_camera(const vec3& delta) {
  m_cameraEye = m_cameraEye + delta;
  m_cameraTarget = m_cameraTarget + delta;
}

void Engine::rotate_camera(float pitch, float yaw) {
  vec3 dir = m_cameraTarget - m_cameraEye;
  float dist = dir.mag();
  dir = dir.normalized();
  vec3 right = dir.cross(vec3(0.0f, 1.0f, 0.0f)).normalized();

  dir = mat3::rotation_y(yaw) * dir;
  dir = mat3::rotation(right, pitch) * dir;

  if (std::abs(dir.dot(vec3(0.0f, 1.0f, 0.0f))) > 0.99f)
    dir = (m_cameraTarget - m_cameraEye).normalized();

  m_cameraTarget = m_cameraEye + dist * dir;
}

void Engine::run(std::function<void(double)> code) {
  while (!glfwWindowShouldClose(m_window)) {
    updateTimes();
    m_inputManager->reset();
    glfwPollEvents();

    unsigned int imgIndex = m_renderer->prepFrame(m_context->device());
    auto [drawImage, drawView] = m_renderer->drawTarget(imgIndex);
    auto [renderImage, renderView] = m_renderer->renderTarget(imgIndex);

    m_drawOutput = new ImageHandle;
    m_drawOutput->image = drawImage;
    m_drawOutput->view = drawView;

    m_renderTarget = new ImageHandle;
    m_renderTarget->image = renderImage;
    m_renderTarget->view = renderView;

    transitionImagesCompute();
    code(m_frameTime);

    transitionImagesGraphics(m_renderer->renderBuffer());
    m_renderer->render(m_context, m_scene, m_resources, imgIndex);
    blit();

    postProcess();

    m_renderer->submit(m_context, imgIndex);

    delete m_drawOutput;
    delete m_renderTarget;
  }

  m_context->device().waitIdle();
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

RID Engine::create_sampler(const SamplerSettings& settings) {
  bool anisotropy = settings.anisotropic_filtering;
  if (anisotropy && m_context->supportsAnisotropy()) {
    Log::warn("GPU does not support anisotropic filtering. sampler will not use this feature");
    anisotropy = false;
  }

  vk::Sampler sampler = m_context->device().createSampler(vk::SamplerCreateInfo{
    .magFilter        = static_cast<vk::Filter>(settings.mag_filter),
    .minFilter        = static_cast<vk::Filter>(settings.min_filter),
    .addressModeU     = static_cast<vk::SamplerAddressMode>(settings.mode_u),
    .addressModeV     = static_cast<vk::SamplerAddressMode>(settings.mode_v),
    .addressModeW     = static_cast<vk::SamplerAddressMode>(settings.mode_w),
    .anisotropyEnable = true,
    .maxAnisotropy    = m_context->gpu().getProperties().limits.maxSamplerAnisotropy
  });

  RID rid(m_nextRID++, ResourceType::Sampler);
  m_resources[rid] = reinterpret_cast<unsigned long>(static_cast<VkSampler>(sampler));

  return rid;
}

void Engine::destroy_sampler(RID& rid) {
  if (!rid.is_valid()) {
    Log::warn("tried to destroy sampler invalid rid");
    return;
  }

  if (rid.m_type != ResourceType::Sampler) {
    Log::warn("tried to destroy sampler of non-sampler RID");
    return;
  }

  if (m_busySamplers.contains(rid)) {
    Log::warn("cannot destroy sampler -- sampler is in use");
    return;
  }

  m_context->device().destroySampler(reinterpret_cast<VkSampler>(m_resources.at(rid)));
  m_resources.erase(rid);

  rid.invalidate();
}

RID Engine::create_storage_image(unsigned int width, unsigned int height, ImageType type, Format format) {
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
    .imageType    = static_cast<vk::ImageType>(type),
    .format       = static_cast<vk::Format>(format),
    .extent       = vk::Extent3D{ width, height, 1 },
    .mipLevels    = 1,
    .arrayLayers  = 1,
    .samples      = vk::SampleCountFlagBits::e1,
    .tiling       = vk::ImageTiling::eOptimal,
    .usage        = vk::ImageUsageFlagBits::eStorage,
  };

  vk::Image image = m_allocator->allocateImage(imageCreateInfo);

  vk::CommandBuffer cmdBuf = m_context->beginTransfer();

  vk::ImageMemoryBarrier shaderBarrier{
    .oldLayout        = vk::ImageLayout::eUndefined,
    .newLayout        = vk::ImageLayout::eGeneral,
    .image            = image,
    .subresourceRange = {
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .levelCount = 1,
      .layerCount = 1
    }
  };

  cmdBuf.pipelineBarrier(
    vk::PipelineStageFlagBits::eTopOfPipe,
    vk::PipelineStageFlagBits::eTopOfPipe,
    vk::DependencyFlags(),
    nullptr,
    nullptr,
    shaderBarrier
  );

  m_context->endTransfer(cmdBuf);

  vk::ImageViewCreateInfo viewCreateInfo{
    .image    = image,
    .viewType = static_cast<vk::ImageViewType>(type),
    .format   = static_cast<vk::Format>(format),
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

  RID rid(m_nextRID++, ResourceType::StorageImage);
  m_resources[rid] = reinterpret_cast<unsigned long>(handle);

  return rid;
}

RID Engine::create_texture(const std::string& path, const RID& sampler) {
  if (!sampler.is_valid()) {
    Log::warn("tried to create image with invalid sampler RID");
    return RID();
  }

  if (sampler.m_type != ResourceType::Sampler) {
    Log::warn("tried to create image with non-sampler RID");
    return RID();
  }

  int width, height, channels;
  unsigned char * pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
  if (!pixels) {
    Log::warn(std::format("failed to load image: {}", std::string(stbi_failure_reason())));
    return RID();
  }
  unsigned int size = width * height * 4;

  vk::Buffer buffer = m_allocator->allocateBuffer(vk::BufferCreateInfo{
    .size   = static_cast<unsigned int>(width * height * 4),
    .usage  = vk::BufferUsageFlagBits::eTransferSrc
  });

  void * map = m_allocator->mapBuffer(buffer);
  std::memcpy(map, pixels, size);
  m_allocator->unmapBuffer(buffer);

  stbi_image_free(pixels);
  map = pixels = nullptr;

  vk::Image image = m_allocator->allocateImage(vk::ImageCreateInfo{
    .imageType    = vk::ImageType::e2D,
    .format       = vk::Format::eR8G8B8A8Srgb,
    .extent       = { static_cast<unsigned int>(width), static_cast<unsigned int>(height), 1 },
    .mipLevels    = 1,
    .arrayLayers  = 1,
    .samples      = vk::SampleCountFlagBits::e1,
    .tiling       = vk::ImageTiling::eOptimal,
    .usage        = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst
  });

  vk::CommandBuffer cmdBuf = m_context->beginTransfer();

  vk::ImageMemoryBarrier copyBarrier{
    .dstAccessMask    = vk::AccessFlagBits::eTransferWrite,
    .newLayout        = vk::ImageLayout::eTransferDstOptimal,
    .image            = image,
    .subresourceRange = {
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .levelCount = 1,
      .layerCount = 1
    }
  };

  cmdBuf.pipelineBarrier(
    vk::PipelineStageFlagBits::eTopOfPipe,
    vk::PipelineStageFlagBits::eTransfer,
    vk::DependencyFlags(),
    nullptr,
    nullptr,
    copyBarrier
  );

  cmdBuf.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, vk::BufferImageCopy{
    .imageSubresource = {
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .layerCount = 1
    },
    .imageExtent = { static_cast<unsigned int>(width), static_cast<unsigned int>(height), 1 }
  });

  vk::ImageMemoryBarrier shaderBarrier{
    .srcAccessMask    = vk::AccessFlagBits::eTransferWrite,
    .dstAccessMask    = vk::AccessFlagBits::eShaderRead,
    .oldLayout        = vk::ImageLayout::eTransferDstOptimal,
    .newLayout        = vk::ImageLayout::eShaderReadOnlyOptimal,
    .image            = image,
    .subresourceRange = {
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .levelCount = 1,
      .layerCount = 1
    }
  };

  cmdBuf.pipelineBarrier(
    vk::PipelineStageFlagBits::eTransfer,
    vk::PipelineStageFlagBits::eFragmentShader,
    vk::DependencyFlags(),
    nullptr,
    nullptr,
    shaderBarrier
  );

  m_context->endTransfer(cmdBuf);

  m_allocator->destroyBuffer(buffer);

  vk::ImageView view = m_context->device().createImageView(vk::ImageViewCreateInfo{
    .image = image,
    .viewType = vk::ImageViewType::e2D,
    .format = vk::Format::eR8G8B8A8Srgb,
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
  });

  ImageHandle * handle = new ImageHandle;
  handle->image = image;
  handle->view = view;
  handle->sampler = sampler;

  RID rid(m_nextRID++, ResourceType::Texture);
  m_resources[rid] = reinterpret_cast<unsigned long>(handle);
  m_busySamplers.emplace(rid);

  return rid;
}

RID Engine::create_storage_texture(unsigned int width, unsigned int height, const RID& sampler, ImageType type, Format format) {
  if (!sampler.is_valid()) {
    Log::warn("tried to create storage texture with invalid sampler RID");
    return RID();
  }

  if (sampler.m_type != ResourceType::Sampler) {
    Log::warn("tried to create storage texture with non-sampler RID");
    return RID();
  }

  if (format == Format::undefined) {
    Log::warn("tried to create storage texture with undefined format");
    return RID();
  }

  vk::ImageCreateInfo imageCreateInfo{
    .imageType    = static_cast<vk::ImageType>(type),
    .format       = static_cast<vk::Format>(format),
    .extent       = vk::Extent3D{ width, height, 1 },
    .mipLevels    = 1,
    .arrayLayers  = 1,
    .samples      = vk::SampleCountFlagBits::e1,
    .tiling       = vk::ImageTiling::eOptimal,
    .usage        = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
  };

  vk::Image image = m_allocator->allocateImage(imageCreateInfo);

  vk::CommandBuffer cmdBuf = m_context->beginTransfer();

  vk::ImageMemoryBarrier shaderBarrier{
    .oldLayout        = vk::ImageLayout::eUndefined,
    .newLayout        = vk::ImageLayout::eShaderReadOnlyOptimal,
    .image            = image,
    .subresourceRange = {
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .levelCount = 1,
      .layerCount = 1
    }
  };

  cmdBuf.pipelineBarrier(
    vk::PipelineStageFlagBits::eTopOfPipe,
    vk::PipelineStageFlagBits::eTopOfPipe,
    vk::DependencyFlags(),
    nullptr,
    nullptr,
    shaderBarrier
  );

  m_context->endTransfer(cmdBuf);

  vk::ImageViewCreateInfo viewCreateInfo{
    .image = image,
    .viewType = static_cast<vk::ImageViewType>(type),
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
  handle->sampler = sampler;

  RID rid(m_nextRID++, ResourceType::StorageTexture);
  m_resources[rid] = reinterpret_cast<unsigned long>(handle);
  m_busySamplers.emplace(sampler);
  m_storageTextures.emplace(reinterpret_cast<unsigned long>(static_cast<VkImage>(image)));

  return rid;
}

void Engine::destroy_image(RID& rid) {
  if (!rid.is_valid()) {
    Log::warn("tried to destroy invalid image RID");
    return;
  }

  if (rid.m_type != ResourceType::StorageImage && rid.m_type != ResourceType::StorageTexture && rid.m_type != ResourceType::Texture) {
    Log::warn("tried to destroy image of non-image RID");
    return;
  }

  ImageHandle * image = reinterpret_cast<ImageHandle *>(m_resources.at(rid));

  m_context->device().destroyImageView(image->view);
  m_allocator->destroyImage(image->image);
  if (image->sampler.is_valid())
    m_busySamplers.erase(image->sampler);

  if (rid.m_type == ResourceType::StorageTexture)
    m_storageTextures.erase(reinterpret_cast<unsigned long>(static_cast<VkImage>(image->image)));
  m_resources.erase(rid);
  delete image;

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
  imageInfos.reserve(2 * descriptors.size());

  std::vector<vk::WriteDescriptorSet> writes;

  unsigned int binding = 0;
  int uniformPoolIndex = -1, storagePoolIndex = -1, imagePoolIndex = -1, texturePoolIndex = -1;
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
          .descriptorCount  = 1,
          .stageFlags       = vk::ShaderStageFlagBits::eAll
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
          .descriptorCount  = 1,
          .stageFlags       = vk::ShaderStageFlagBits::eAll
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
      case StorageImage: {
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
          .descriptorCount  = 1,
          .stageFlags       = vk::ShaderStageFlagBits::eAll
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
      case Texture: {
        if (texturePoolIndex == -1) {
          texturePoolIndex = poolSizes.size();
          poolSizes.emplace_back(vk::DescriptorPoolSize{
            .type = vk::DescriptorType::eCombinedImageSampler
          });
        }
        ++poolSizes[texturePoolIndex].descriptorCount;

        bindings.emplace_back(vk::DescriptorSetLayoutBinding{
          .binding          = binding,
          .descriptorType   = vk::DescriptorType::eCombinedImageSampler,
          .descriptorCount  = 1,
          .stageFlags       = vk::ShaderStageFlagBits::eAll
        });

        ImageHandle * image = reinterpret_cast<ImageHandle *>(m_resources.at(descriptor));

        imageInfos.emplace_back(vk::DescriptorImageInfo{
          .sampler      = reinterpret_cast<VkSampler>(m_resources.at(image->sampler)),
          .imageView    = image->view,
          .imageLayout  = vk::ImageLayout::eShaderReadOnlyOptimal
        });

        writes.emplace_back(vk::WriteDescriptorSet{
          .dstSet           = nullptr,
          .dstBinding       = binding++,
          .descriptorCount  = 1,
          .descriptorType   = vk::DescriptorType::eCombinedImageSampler,
          .pImageInfo       = &imageInfos.back()
        });

        break;
      }
      case StorageTexture: {
        if (imagePoolIndex == -1) {
          imagePoolIndex = poolSizes.size();
          poolSizes.emplace_back(vk::DescriptorPoolSize{
            .type = vk::DescriptorType::eStorageImage
          });
        }
        ++poolSizes[imagePoolIndex].descriptorCount;

        if (texturePoolIndex == -1) {
          texturePoolIndex = poolSizes.size();
          poolSizes.emplace_back(vk::DescriptorPoolSize{
            .type = vk::DescriptorType::eCombinedImageSampler
          });
        }
        ++poolSizes[texturePoolIndex].descriptorCount;

        ImageHandle * image = reinterpret_cast<ImageHandle *>(m_resources.at(descriptor));

        bindings.emplace_back(vk::DescriptorSetLayoutBinding{
          .binding          = binding,
          .descriptorType   = vk::DescriptorType::eStorageImage,
          .descriptorCount  = 1,
          .stageFlags       = vk::ShaderStageFlagBits::eAll
        });

        imageInfos.emplace_back(vk::DescriptorImageInfo{
          .imageView    = image->view,
          .imageLayout  = vk::ImageLayout::eGeneral
        });

        writes.emplace_back(vk::WriteDescriptorSet{
          .dstSet           = nullptr,
          .dstBinding       = binding++,
          .descriptorCount  = 1,
          .descriptorType   = vk::DescriptorType::eStorageImage,
          .pImageInfo       = &imageInfos.back()
        });

        bindings.emplace_back(vk::DescriptorSetLayoutBinding{
          .binding          = binding,
          .descriptorType   = vk::DescriptorType::eCombinedImageSampler,
          .descriptorCount  = 1,
          .stageFlags       = vk::ShaderStageFlagBits::eAll
        });

        imageInfos.emplace_back(vk::DescriptorImageInfo{
          .sampler      = reinterpret_cast<VkSampler>(m_resources.at(image->sampler)),
          .imageView    = image->view,
          .imageLayout  = vk::ImageLayout::eShaderReadOnlyOptimal
        });

        writes.emplace_back(vk::WriteDescriptorSet{
          .dstSet           = nullptr,
          .dstBinding       = binding++,
          .descriptorCount  = 1,
          .descriptorType   = vk::DescriptorType::eCombinedImageSampler,
          .pImageInfo       = &imageInfos.back()
        });

        break;
      }
      case RenderTarget: {
        if (imagePoolIndex == -1) {
          imagePoolIndex = poolSizes.size();
          poolSizes.emplace_back(vk::DescriptorPoolSize{
            .type = vk::DescriptorType::eStorageImage
          });
        }
        poolSizes[imagePoolIndex].descriptorCount += 2;

        bindings.emplace_back(vk::DescriptorSetLayoutBinding{
          .binding          = binding,
          .descriptorType   = vk::DescriptorType::eStorageImage,
          .descriptorCount  = 1,
          .stageFlags       = vk::ShaderStageFlagBits::eAll
        });

        imageInfos.emplace_back(vk::DescriptorImageInfo{
          .imageView    = m_drawOutput->view,
          .imageLayout  = vk::ImageLayout::eGeneral
        });

        writes.emplace_back(vk::WriteDescriptorSet{
          .dstSet           = nullptr,
          .dstBinding       = binding++,
          .descriptorCount  = 1,
          .descriptorType   = vk::DescriptorType::eStorageImage,
          .pImageInfo       = &imageInfos.back()
        });

        bindings.emplace_back(vk::DescriptorSetLayoutBinding{
          .binding          = binding,
          .descriptorType   = vk::DescriptorType::eStorageImage,
          .descriptorCount  = 1,
          .stageFlags       = vk::ShaderStageFlagBits::eAll
        });

        imageInfos.emplace_back(vk::DescriptorImageInfo{
          .imageView    = m_renderTarget->view,
          .imageLayout  = vk::ImageLayout::eGeneral
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

  vk::PushConstantRange pushConstants{
    .stageFlags = vk::ShaderStageFlagBits::eCompute,
    .size = m_context->gpu().getProperties().limits.maxPushConstantsSize
  };

  vk::PipelineLayoutCreateInfo layoutCreateInfo{
    .setLayoutCount         = 1,
    .pSetLayouts            = &set->layout,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges    = &pushConstants
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

  vk::PushConstantRange pushConstants{
    .stageFlags = vk::ShaderStageFlagBits::eAll,
    .size = m_context->gpu().getProperties().limits.maxPushConstantsSize
  };

  vk::PipelineLayoutCreateInfo layoutCreateInfo{
    .setLayoutCount         = 1,
    .pSetLayouts            = &set->layout,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges    = &pushConstants
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

  auto binding = Vertex::binding();
  auto attributes = Vertex::attributes();
  vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo{
    .vertexBindingDescriptionCount    = 1,
    .pVertexBindingDescriptions       = &binding,
    .vertexAttributeDescriptionCount  = static_cast<unsigned int>(attributes.size()),
    .pVertexAttributeDescriptions     = attributes.data()
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
    .depthTestEnable        = s.enable_depth_test,
    .depthWriteEnable       = s.enable_depth_write,
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
    .depthAttachmentFormat    = m_renderer->depthFormat()
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

RID Engine::load_mesh(const std::string& path) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::string err;

  if (!tinyobj::LoadObj(&attrib, &shapes, nullptr, &err, path.c_str())) {
    Log::runtime_error(std::format("failed to load object: {}", err));
    return RID();
  }

  using IndexTrio = std::tuple<unsigned int, unsigned int, unsigned int>;
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::unordered_map<Vertex, unsigned int, Vertex::Hash> indexMap;

  bool hasNormals = !attrib.normals.empty();
  bool hasTexcoords = !attrib.texcoords.empty();

  for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
      Vertex v{};

      v.position = vec3(
        attrib.vertices[3 * index.vertex_index],
        attrib.vertices[3 * index.vertex_index + 1],
        attrib.vertices[3 * index.vertex_index + 2]
      );

      if (hasTexcoords) {
        v.uv = vec2(
          attrib.texcoords[2 * index.texcoord_index],
          attrib.texcoords[2 * index.texcoord_index + 1]
        );
      }

      if (hasNormals) {
        v.normal = vec3(
          attrib.normals[3 * index.normal_index],
          attrib.normals[3 * index.normal_index + 1],
          attrib.normals[3 * index.normal_index + 2]
        );
      }

      if (!indexMap.contains(v)) {
        indexMap[v] = static_cast<unsigned int>(vertices.size());
        vertices.emplace_back(v);
      }

      indices.emplace_back(indexMap.at(v));
    }
  }

  if (vertices.empty() || indices.empty()) {
    Log::warn(std::format("mesh '{}' has no geometry", path));
    return RID();
  }

  if (!hasTexcoords) {
    Log::warn(std::format("mesh '{}' is missing UVs, generating planar projection", path));

    vec3 minBounds = vertices[0].position;
    vec3 maxBounds = vertices[0].position;

    for (const auto& vertex : vertices) {
      minBounds.x = std::min(minBounds.x, vertex.position.x);
      minBounds.y = std::min(minBounds.y, vertex.position.y);
      minBounds.z = std::min(minBounds.z, vertex.position.z);

      maxBounds.x = std::max(maxBounds.x, vertex.position.x);
      maxBounds.y = std::max(maxBounds.y, vertex.position.y);
      maxBounds.z = std::max(maxBounds.z, vertex.position.z);
    }

    vec3 size = maxBounds - minBounds;

    for (auto& vertex : vertices) {
      vec3 normalized = (vertex.position - minBounds) / size;

      if (size.x >= size.y && size.x >= size.z) {
        vertex.uv = vec2(normalized.y, normalized.z);
        continue;
      }

      if (size.y >= size.x && size.y >= size.z) {
        vertex.uv = vec2(normalized.x, normalized.z);
        continue;
      }

      vertex.uv = vec2(normalized.x, normalized.y);
    }
  }

  if (!hasNormals) {
    Log::warn(std::format("mesh '{}' is missing normals, calculating face normals", path));

    for (std::size_t i = 0; i < indices.size(); i += 3) {
      unsigned int i0 = indices[i];
      unsigned int i1 = indices[i + 1];
      unsigned int i2 = indices[i + 2];

      vec3 v0 = vertices[i0].position;
      vec3 v1 = vertices[i1].position;
      vec3 v2 = vertices[i2].position;

      vec3 edge1 = v1 - v0;
      vec3 edge2 = v2 - v0;
      vec3 faceNormal = edge1.cross(edge2);

      vertices[i0].normal = vertices[i0].normal + faceNormal;
      vertices[i1].normal = vertices[i1].normal + faceNormal;
      vertices[i2].normal = vertices[i2].normal + faceNormal;
    }

    for (auto& vertex : vertices)
      vertex.normal = vertex.normal.normalized();
  }

  vk::Buffer vertexStaging = m_allocator->allocateBuffer(vk::BufferCreateInfo{
    .size   = sizeof(Vertex) * vertices.size(),
    .usage  = vk::BufferUsageFlagBits::eTransferSrc
  });

  void * map = m_allocator->mapBuffer(vertexStaging);
  std::memcpy(map, vertices.data(), sizeof(Vertex) * vertices.size());
  m_allocator->unmapBuffer(vertexStaging);

  vk::Buffer indexStaging = m_allocator->allocateBuffer(vk::BufferCreateInfo{
    .size   = sizeof(unsigned int) * indices.size(),
    .usage  = vk::BufferUsageFlagBits::eTransferSrc
  });

  map = m_allocator->mapBuffer(indexStaging);
  std::memcpy(map, indices.data(), sizeof(unsigned int) * indices.size());
  m_allocator->unmapBuffer(indexStaging);

  vk::Buffer vertexBuffer = m_allocator->allocateBuffer(vk::BufferCreateInfo{
    .size   = sizeof(Vertex) * vertices.size(),
    .usage  = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst
  }, VMA_MEMORY_USAGE_GPU_ONLY, 0);

  vk::Buffer indexBuffer = m_allocator->allocateBuffer(vk::BufferCreateInfo{
    .size   = sizeof(unsigned int) * indices.size(),
    .usage  = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst
  }, VMA_MEMORY_USAGE_GPU_ONLY, 0);

  vk::CommandBuffer cmdBuf = m_context->beginTransfer();

  cmdBuf.copyBuffer(vertexStaging, vertexBuffer, vk::BufferCopy{
    .size = sizeof(Vertex) * vertices.size()
  });

  cmdBuf.copyBuffer(indexStaging, indexBuffer, vk::BufferCopy{
    .size = sizeof(unsigned int) * indices.size()
  });

  m_context->endTransfer(cmdBuf);

  m_allocator->destroyBuffer(vertexStaging);
  m_allocator->destroyBuffer(indexStaging);

  MeshHandle * mesh = new MeshHandle;

  mesh->vertexBuffer = vertexBuffer;
  mesh->indexBuffer = indexBuffer;
  mesh->indexCount = indices.size();

  RID rid(m_nextRID++, ResourceType::Mesh);
  m_resources[rid] = reinterpret_cast<unsigned long>(mesh);

  return rid;
}

void Engine::destroy_mesh(RID& rid) {
  if (!rid.is_valid()) {
    Log::warn("tried to destroy invalid mesh RID");
    return;
  }

  if (rid.m_type != ResourceType::Mesh) {
    Log::warn("tried to destroy mesh of non-mesh RID");
    return;
  }

  MeshHandle * mesh = reinterpret_cast<MeshHandle *>(m_resources.at(rid));

  m_allocator->destroyBuffer(mesh->vertexBuffer);
  m_allocator->destroyBuffer(mesh->indexBuffer);
  delete mesh;

  m_resources.erase(rid);

  rid.invalidate();
}

void Engine::compute_command(const ComputeCommand& cmd) {
  if (!cmd.pipeline.is_valid()) {
    Log::warn("tried to submit compute command with invalid pipeline");
    return;
  }

  if (cmd.pipeline.m_type != ResourceType::Pipeline) {
    Log::warn("tried to submit compute command with non-pipeline RID");
    return;
  }

  if (!cmd.descriptor_set.is_valid()) {
    Log::warn("tried to submit compute command with invalid descriptor set");
    return;
  }

  if (cmd.descriptor_set.m_type != ResourceType::DescriptorSet) {
    Log::warn("tried to submit compute command with non-descriptor-set RID");
    return;
  }

  cmd.post_process ? m_postProcessCmds.emplace(cmd) : m_computeCmds.emplace(cmd);
}

void Engine::dispatch() {
  if (m_computeCmds.empty()) {
    Log::warn("tried to dispatch 0 compute commands");
    return;
  }

  vk::CommandBuffer cmdBuf = nullptr;

  while (!m_computeCmds.empty()) {
    ComputeCommand cmd = m_computeCmds.front();
    m_computeCmds.pop();

    if (cmdBuf == nullptr)
      cmdBuf = m_context->beginDispatch();
    else if (cmd.barrier) {
      m_context->endDispatch(cmdBuf);
      cmdBuf = m_context->beginDispatch();
    }

    PipelineHandle * pipeline = reinterpret_cast<PipelineHandle *>(m_resources.at(cmd.pipeline));
    DescriptorSetHandle * set = reinterpret_cast<DescriptorSetHandle *>(m_resources.at(cmd.descriptor_set));

    cmdBuf.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline->pipeline);

    cmdBuf.bindDescriptorSets(
      vk::PipelineBindPoint::eCompute,
      pipeline->layout,
      0,
      1,
      &set->set,
      0,
      nullptr
    );

    if (!cmd.push_constants.empty()) {
      cmdBuf.pushConstants<uint8_t>(
        pipeline->layout,
        vk::ShaderStageFlagBits::eCompute,
        0,
        cmd.push_constants
      );
    }

    auto [x, y, z] = cmd.work_groups;
    cmdBuf.dispatch(x, y, z);
  }

  m_context->endDispatch(cmdBuf);
}

void Engine::add_to_scene(Object& object) {
  if (object.is_in_scene()) {
    Log::warn("tried to add object to scene that was already added to the scene");
    return;
  }

  object.m_id.m_id = m_nextRID++;
  m_scene.emplace(object);
}

void Engine::remove_from_scene(Object& object) {
  if (!object.is_in_scene()) {
    Log::warn("tried to remove object from scene that was not in the scene");
    return;
  }

  m_scene.erase(object);
  object.m_id.invalidate();
}

void Engine::updateTimes() {
  static std::chrono::time_point start = std::chrono::high_resolution_clock::now();
  std::chrono::time_point now = std::chrono::high_resolution_clock::now();

  double time = std::chrono::duration(now - start).count();

  m_frameTime = std::min(time - m_time, 0.25);
  m_time = time;
}

std::pair<unsigned int, void *> Engine::readBufferRaw(const RID& rid) const {
  if (!rid.is_valid()) {
    Log::warn("tried to read from invalid buffer RID");
    return std::make_pair(0, nullptr);
  }

  if (rid.m_type != ResourceType::UniformBuffer && rid.m_type != ResourceType::StorageBuffer) {
    Log::warn("tried to read buffer from non-buffer RID");
    return std::make_pair(0, nullptr);
  }

  vk::Buffer buffer = reinterpret_cast<VkBuffer>(m_resources.at(rid));

  unsigned int size = m_allocator->bufferSize(buffer);
  void * data = new char[size];

  void * map = m_allocator->mapBuffer(buffer);
  std::memcpy(data, map, size);
  m_allocator->unmapBuffer(buffer);

  return std::make_pair(size, data);
}

void Engine::writeBufferRaw(const RID& rid, std::size_t size, const void * data) const {
  if (!rid.is_valid()) {
    Log::warn("tried to write to invalid buffer RID");
    return;
  }

  if (rid.m_type != ResourceType::UniformBuffer && rid.m_type != ResourceType::StorageBuffer) {
    Log::warn("tried to write to buffer of non-buffer RID");
    return;
  }

  if (size == 0) {
    Log::warn("tried to write 0 bytes to buffer RID");
    return;
  }

  vk::Buffer buffer = reinterpret_cast<VkBuffer>(m_resources.at(rid));

  void * map = m_allocator->mapBuffer(buffer);
  std::memcpy(map, data, size);
  m_allocator->unmapBuffer(buffer);
}

void Engine::transitionImagesCompute() const {
  if (m_storageTextures.empty()) return;

  vk::CommandBuffer cmdBuf = m_context->beginDispatch();

  for (const auto& image : m_storageTextures) {
    vk::ImageMemoryBarrier barrier {
      .srcAccessMask    = vk::AccessFlagBits::eShaderRead,
      .dstAccessMask    = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite,
      .oldLayout        = vk::ImageLayout::eShaderReadOnlyOptimal,
      .newLayout        = vk::ImageLayout::eGeneral,
      .image            = reinterpret_cast<VkImage>(image),
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    };

    cmdBuf.pipelineBarrier(
      vk::PipelineStageFlagBits::eFragmentShader,
      vk::PipelineStageFlagBits::eComputeShader,
      vk::DependencyFlags(),
      nullptr,
      nullptr,
      barrier
    );
  }

  m_context->endDispatch(cmdBuf);
}

void Engine::transitionImagesGraphics(const vk::CommandBuffer& cmdBuf) const {
  if (m_storageTextures.empty()) return;

  for (const auto& image : m_storageTextures) {
    vk::ImageMemoryBarrier barrier {
      .srcAccessMask    = vk::AccessFlagBits::eShaderWrite,
      .dstAccessMask    = vk::AccessFlagBits::eShaderRead,
      .oldLayout        = vk::ImageLayout::eGeneral,
      .newLayout        = vk::ImageLayout::eShaderReadOnlyOptimal,
      .image            = reinterpret_cast<VkImage>(image),
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    };

    cmdBuf.pipelineBarrier(
      vk::PipelineStageFlagBits::eComputeShader,
      vk::PipelineStageFlagBits::eFragmentShader,
      vk::DependencyFlags(),
      nullptr,
      nullptr,
      barrier
    );
  }
}

void Engine::postProcess() {
  if (m_postProcessCmds.empty()) {
    vk::CommandBuffer cmdBuf = m_context->beginDispatch();

    vk::ImageMemoryBarrier barrier{
      .srcAccessMask  = vk::AccessFlagBits::eShaderWrite,
      .oldLayout      = vk::ImageLayout::eGeneral,
      .newLayout      = vk::ImageLayout::ePresentSrcKHR,
      .image          = m_renderTarget->image,
      .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
      }
    };

    cmdBuf.pipelineBarrier(
      vk::PipelineStageFlagBits::eComputeShader,
      vk::PipelineStageFlagBits::eBottomOfPipe,
      vk::DependencyFlags(),
      nullptr,
      nullptr,
      barrier
    );

    m_context->endDispatch(cmdBuf);
    return;
  }

  vk::CommandBuffer cmdBuf = nullptr;
  std::vector<RID> rids;
  while (!m_postProcessCmds.empty()) {
    ComputeCommand cmd = m_postProcessCmds.front();
    m_postProcessCmds.pop();

    if (cmdBuf == nullptr)
      cmdBuf = m_context->beginDispatch();
    else if (cmd.barrier) {
      m_context->endDispatch(cmdBuf);
      cmdBuf = m_context->beginDispatch();
    }

    PipelineHandle * pipeline = reinterpret_cast<PipelineHandle *>(m_resources.at(cmd.pipeline));
    DescriptorSetHandle * set = reinterpret_cast<DescriptorSetHandle *>(m_resources.at(cmd.descriptor_set));

    cmdBuf.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline->pipeline);

    cmdBuf.bindDescriptorSets(
      vk::PipelineBindPoint::eCompute,
      pipeline->layout,
      0,
      1,
      &set->set,
      0,
      nullptr
    );

    if (!cmd.push_constants.empty()) {
      cmdBuf.pushConstants<uint8_t>(
        pipeline->layout,
        vk::ShaderStageFlagBits::eCompute,
        0,
        cmd.push_constants
      );
    }

    auto [x, y, z] = cmd.work_groups;
    cmdBuf.dispatch(x, y, z);

    rids.emplace_back(cmd.pipeline);
    rids.emplace_back(cmd.descriptor_set);
  }

  vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier{
    .srcAccessMask  = vk::AccessFlagBits::eShaderWrite,
    .oldLayout      = vk::ImageLayout::eGeneral,
    .newLayout      = vk::ImageLayout::ePresentSrcKHR,
    .image          = m_renderTarget->image,
    .subresourceRange = {
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .levelCount = 1,
      .layerCount = 1
    }
  };

  cmdBuf.pipelineBarrier(
    vk::PipelineStageFlagBits::eComputeShader,
    vk::PipelineStageFlagBits::eBottomOfPipe,
    vk::DependencyFlags(),
    nullptr,
    nullptr,
    barrier
  );

  m_context->endDispatch(cmdBuf);

  for (auto& rid : rids) {
    switch (rid.m_type) {
      case DescriptorSet:
        destroy_descriptor_set(rid);
        break;
      case Pipeline:
        destroy_pipeline(rid);
      default: break;
    }
  }
}

void Engine::blit() {
  vk::CommandBuffer cmd = m_context->beginTransfer();

  vk::ImageMemoryBarrier drawBarrier{
    .dstAccessMask  = vk::AccessFlagBits::eTransferWrite,
    .oldLayout      = vk::ImageLayout::eColorAttachmentOptimal,
    .newLayout      = vk::ImageLayout::eTransferSrcOptimal,
    .image          = m_drawOutput->image,
    .subresourceRange = {
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .levelCount = 1,
      .layerCount = 1
    }
  };

  vk::ImageMemoryBarrier renderBarrier{
    .dstAccessMask  = vk::AccessFlagBits::eTransferRead,
    .oldLayout      = vk::ImageLayout::ePresentSrcKHR,
    .newLayout      = vk::ImageLayout::eTransferDstOptimal,
    .image          = m_renderTarget->image,
    .subresourceRange = {
      .aspectMask = vk::ImageAspectFlagBits::eColor,
      .levelCount = 1,
      .layerCount = 1
    }
  };

  cmd.pipelineBarrier(
    vk::PipelineStageFlagBits::eColorAttachmentOutput,
    vk::PipelineStageFlagBits::eTransfer,
    vk::DependencyFlags(),
    nullptr,
    nullptr,
    { drawBarrier, renderBarrier }
  );

  auto [width, height] = m_renderer->extent();

  cmd.copyImage(
    m_drawOutput->image, vk::ImageLayout::eTransferSrcOptimal,
    m_renderTarget->image, vk::ImageLayout::eTransferDstOptimal,
    vk::ImageCopy{
      .srcSubresource = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .layerCount = 1,
      },
      .dstSubresource = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .layerCount = 1
      },
      .extent = { width, height, 1 }
    }
  );

  drawBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
  drawBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
  drawBarrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
  drawBarrier.newLayout = vk::ImageLayout::eGeneral;

  renderBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
  renderBarrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
  renderBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
  renderBarrier.newLayout = vk::ImageLayout::eGeneral;

  cmd.pipelineBarrier(
    vk::PipelineStageFlagBits::eTransfer,
    vk::PipelineStageFlagBits::eComputeShader,
    vk::DependencyFlags(),
    nullptr,
    nullptr,
    { drawBarrier, renderBarrier }
  );

  m_context->endTransfer(cmd);
}

} // namespace groot