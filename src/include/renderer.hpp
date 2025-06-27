#pragma once

#include "src/include/materials.hpp"

#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_beta.h>

#include <vector>

namespace ge {

class Engine;

class Renderer {
  public:
    Renderer() = default;
    Renderer(Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    ~Renderer() = default;

    Renderer& operator = (Renderer&) = delete;
    Renderer& operator = (Renderer&&) = delete;

    const unsigned int& frameIndex() const;

    void initialize(Engine&);
    void render(const Engine&);

  private:
    void checkFormat(Engine&) const;
    void checkPresentMode(Engine&) const;
    std::pair<unsigned int, vk::SurfaceTransformFlagBitsKHR> checkExtent(Engine&) const;

    void createSwapchain(const Engine&, const unsigned int&, const vk::SurfaceTransformFlagBitsKHR&);
    void createViews(const Engine&);
    void createSyncObjects(const Engine&);
    void transitionImages(const Engine&, const unsigned int&);
    void preDraw(const Engine&, const unsigned int&);
    void draw(const Engine&);
    void endRendering();

  private:
    unsigned int m_frameIndex = 0;
    mat4 m_view = mat4::view(vec3(0.0f, 0.0f, -2.0f), vec3(0.0f), vec3(0.0f, -1.0f, 0.0f));
    mat4 m_projection = mat4::identity();

    vk::raii::SwapchainKHR m_swapchain = nullptr;
    std::vector<vk::Image> m_images;
    std::vector<vk::raii::ImageView> m_views;

    vk::raii::DeviceMemory m_depthMem = nullptr;
    vk::raii::Image m_depthImage = nullptr;
    vk::raii::ImageView m_depthView = nullptr;

    vk::raii::CommandBuffers m_renderCmds = nullptr;
    vk::raii::CommandBuffers m_computeCmds = nullptr;

    std::vector<vk::raii::Fence> m_flightFences;
    std::vector<vk::raii::Fence> m_computeFences;
    std::vector<vk::raii::Semaphore> m_renderSemaphores;
    std::vector<vk::raii::Semaphore> m_imageSemaphores;
    std::vector<vk::raii::Semaphore> m_computeSemaphores;
};

} // namespace ge