#pragma once

#include "src/include/transform.hpp"
#include "src/include/vertex.hpp"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_beta.h>

#include <map>
#include <memory>
#include <set>

namespace ge {

class Engine;
class Object;

using transform = std::shared_ptr<Transform>;

class ObjectManager {
  using Output = std::tuple<
    const vk::raii::Buffer&,
    const vk::raii::Buffer&,
    const vk::raii::Buffer&,
    const unsigned int
  >;

  public:
    ObjectManager() = default;
    ObjectManager(const ObjectManager&) = delete;
    ObjectManager(ObjectManager&&) = delete;

    ~ObjectManager() = default;

    ObjectManager& operator=(const ObjectManager&) = delete;
    ObjectManager& operator=(ObjectManager&&) = delete;

    Output operator[](const std::string&) const;

    bool hasObjects(const std::string&) const;
    std::map<std::string, std::vector<mat4>> transforms() const;

    transform add(const Engine&, const std::string&, const std::string&, const Transform& t = Transform());
    void load(const Engine&);
    void update(double);

  private:
    std::map<std::string, std::pair<std::vector<Vertex>, std::vector<unsigned int>>> m_objData;
    std::map<std::string, Object> m_objects;
};

class Object {
  using Output = std::pair<
    const std::vector<vk::raii::Buffer>&,
    unsigned int
  >;

  public:
    struct Command {
      unsigned int indexCount = 0;
      unsigned int instanceCount = 0;
      unsigned int firstIndex = 0;
      unsigned int vertexOffset = 0;
      unsigned int firstInstance = 0;
    };

  public:
    Object() = default;
    Object(const Object&) = delete;
    Object(Object&&) = default;

    ~Object() = default;

    Object& operator=(const Object&) = delete;
    Object& operator=(Object&&) = default;

    Output operator*() const;

    const std::vector<mat4>& transforms() const;

    transform add(const std::vector<Vertex>&, const std::vector<unsigned int>&, const Transform&);
    std::pair<vk::raii::DeviceMemory, std::vector<vk::raii::Buffer>> load(const Engine&, vk::raii::CommandBuffer&);
    void batch(unsigned int);
    void update(double);

  private:
    std::set<unsigned int> m_updates;

    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    std::vector<Command> m_commands;
    std::vector<transform> m_transforms;
    std::vector<mat4> m_matrices;

    vk::raii::DeviceMemory m_memory = nullptr;
    std::vector<vk::raii::Buffer> m_buffers;
};

} // namespace ge