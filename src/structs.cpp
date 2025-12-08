#include "src/include/structs.hpp"

namespace groot {

std::size_t VkBufferHash::operator()(VkBuffer buffer) const {
  return std::hash<unsigned long>{}(reinterpret_cast<unsigned long>(buffer));
}

std::size_t VkImageHash::operator()(VkImage image) const {
  return std::hash<unsigned long>{}(reinterpret_cast<unsigned long>(image));
}

std::size_t Vertex::Hash::operator()(const Vertex& v) const {
  std::hash<float> floatHash{};

  std::size_t px = floatHash(v.position.x);
  std::size_t py = floatHash(v.position.y);
  std::size_t pz = floatHash(v.position.z);
  std::size_t tu = floatHash(v.uv.x);
  std::size_t tv = floatHash(v.uv.y);
  std::size_t nx = floatHash(v.normal.x);
  std::size_t ny = floatHash(v.normal.y);
  std::size_t nz = floatHash(v.normal.z);

  std::size_t posHash = (px ^ (py << 1)) ^ (pz << 1);
  std::size_t texHash = tu ^ (tv << 1);
  std::size_t normHash = (nx ^ (ny << 1)) ^ (nz << 1);

  return posHash ^ texHash ^ normHash;
}

bool Vertex::operator==(const Vertex& rhs) const {
  return position == rhs.position && uv == rhs.uv && normal == rhs.normal;
}

vk::VertexInputBindingDescription Vertex::binding() {
  return vk::VertexInputBindingDescription{
    .binding    = 0,
    .stride     = sizeof(Vertex),
    .inputRate  = vk::VertexInputRate::eVertex
  };
}

std::array<vk::VertexInputAttributeDescription, 3> Vertex::attributes() {
  return {
    vk::VertexInputAttributeDescription{
      .location = 0,
      .binding  = 0,
      .format   = vk::Format::eR32G32B32Sfloat,
      .offset   = offsetof(Vertex, position)
    },
    vk::VertexInputAttributeDescription{
      .location = 1,
      .binding  = 0,
      .format   = vk::Format::eR32G32Sfloat,
      .offset   = offsetof(Vertex, uv)
    },
    vk::VertexInputAttributeDescription{
      .location = 2,
      .binding  = 0,
      .format   = vk::Format::eR32G32B32Sfloat,
      .offset   = offsetof(Vertex, normal)
    }
  };
}

mat4 Transform::matrix() const {
  return mat4::translation(position) * mat4::rotation(rotation) * mat4::scale(scale.x, scale.y, scale.z);
}

} // namespace groot