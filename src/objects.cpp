#include "src/include/allocator.hpp"
#include "src/include/engine.hpp"
#include "src/include/objects.hpp"
#include "src/include/parsers.hpp"

namespace ge {

ObjectManager::Output ObjectManager::operator[](const std::string& material) const {
  if (!m_objects.contains(material))
    throw std::out_of_range("groot-engine: no objects with material '" + material + "'");

  const auto& [buffers, commandCount] = *m_objects.at(material);

  return { buffers[0], buffers[1], buffers[2], commandCount };
}

bool ObjectManager::hasObjects(const std::string& material) const {
  return m_objects.contains(material);
}

std::map<std::string, std::vector<mat4>> ObjectManager::transforms() const {
  std::map<std::string, std::vector<mat4>> mats;

  for (const auto& [material, object] : m_objects)
    mats.emplace(material, object.transforms());

  return mats;
}

transform ObjectManager::add(const Engine& engine,
  const std::string& material, const std::string& objFile, const Transform& t
) {
  if (!engine.m_materials.exists(material))
    throw std::runtime_error("groot-engine: tried to add object with material that does not exist");

  if (!m_objData.contains(objFile)) {
    auto [vertices, indices] = ObjParser::parse(objFile);
    m_objData.emplace(objFile, std::make_pair(vertices, indices));
  }
  const auto& [vertices, indices] = m_objData.at(objFile);

  transform obj = m_objects[material].add(vertices, indices, t);
  obj->m_object = &m_objects.at(material);

  return obj;
}

void ObjectManager::load(const Engine& engine) {
  vk::raii::CommandBuffer transferCmd = std::move(engine.getCmds(QueueFamilyType::Transfer, 1)[0]);
  transferCmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

  std::vector<vk::raii::DeviceMemory> tMems;
  std::vector<vk::raii::Buffer> tBufs;

  for (auto& [material, object] : m_objects) {
    auto [tMem, tBuf] = object.load(engine, transferCmd);

    tMems.emplace_back(std::move(tMem));
    for (auto& buf : tBuf)
      tBufs.emplace_back(std::move(buf));
  }

  transferCmd.end();

  vk::raii::Fence fence = std::move(Allocator::fences(engine, 1)[0]);

  engine.m_context.queueFamily(QueueFamilyType::Transfer).queue.submit(vk::SubmitInfo{
    .commandBufferCount = 1,
    .pCommandBuffers    = &*transferCmd
  }, fence);

  if (engine.m_context.device().waitForFences(*fence, true, ge_timeout) != vk::Result::eSuccess)
    throw std::runtime_error("groot-engine: hung waiting for objects to load");
}

void ObjectManager::update(double dt) {
  for (auto& [material, object] : m_objects)
    object.update(dt);
}

Object::Output Object::operator*() const {
  return { m_buffers, m_commands.size() };
}

const std::vector<mat4>& Object::transforms() const {
  return m_matrices;
}

transform Object::add(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const Transform& t) {
  m_commands.emplace_back(Command{
    .indexCount     = static_cast<unsigned int>(indices.size()),
    .instanceCount  = 1,
    .firstIndex     = static_cast<unsigned int>(m_indices.size()),
    .vertexOffset   = static_cast<unsigned int>(m_vertices.size()),
    .firstInstance  = static_cast<unsigned int>(m_commands.size())
  });

  m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
  m_indices.insert(m_indices.end(), indices.begin(), indices.end());
  m_transforms.emplace_back(std::make_shared<Transform>(t));

  m_transforms.back()->m_index = m_matrices.size();
  m_matrices.emplace_back(t.matrix());

  return m_transforms.back();
}

std::pair<vk::raii::DeviceMemory, std::vector<vk::raii::Buffer>> Object::load(const Engine& engine, vk::raii::CommandBuffer& transferCmd) {
  vk::BufferCreateInfo ci_vertexBuffer{
    .size         = static_cast<unsigned int>(sizeof(Vertex) * m_vertices.size()),
    .usage        = vk::BufferUsageFlagBits::eTransferSrc,
  };

  vk::BufferCreateInfo ci_indexBuffer{
    .size         = static_cast<unsigned int>(sizeof(unsigned int) * m_indices.size()),
    .usage        = vk::BufferUsageFlagBits::eTransferSrc,
  };

  vk::BufferCreateInfo ci_indirectBuffer{
    .size         = static_cast<unsigned int>(sizeof(Command) * m_commands.size()),
    .usage        = vk::BufferUsageFlagBits::eTransferSrc,
  };

  auto [tMem, tBufs, tOffs, tSize] = Allocator::bufferPool(engine, { ci_vertexBuffer, ci_indexBuffer, ci_indirectBuffer },
    vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible
  );

  char * tMap = reinterpret_cast<char *>(tMem.mapMemory(0, tSize));

  memcpy(tMap + tOffs[0], m_vertices.data(), sizeof(Vertex) * m_vertices.size());
  memcpy(tMap + tOffs[1], m_indices.data(), sizeof(unsigned int) * m_indices.size());
  memcpy(tMap + tOffs[2], m_commands.data(), sizeof(Command) * m_commands.size());

  tMem.unmapMemory();

  ci_vertexBuffer.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
  ci_indexBuffer.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
  ci_indirectBuffer.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndirectBuffer;

  auto [tmp_mem, tmp_bufs, _, __] = Allocator::bufferPool(engine, { ci_vertexBuffer, ci_indexBuffer, ci_indirectBuffer },
    vk::MemoryPropertyFlagBits::eDeviceLocal
  );
  m_memory = std::move(tmp_mem);
  m_buffers = std::move(tmp_bufs);

  transferCmd.copyBuffer(tBufs[0], m_buffers[0], vk::BufferCopy{ .size = sizeof(Vertex) * m_vertices.size() });
  transferCmd.copyBuffer(tBufs[1], m_buffers[1], vk::BufferCopy{ .size = sizeof(unsigned int) * m_indices.size() });
  transferCmd.copyBuffer(tBufs[2], m_buffers[2], vk::BufferCopy{ .size = sizeof(Command) * m_commands.size() });

  return { std::move(tMem), std::move(tBufs) };
}

void Object::batch(unsigned int index) {
  if (!m_updates.contains(index))
    m_updates.emplace(index);
}

void Object::update(double dt) {
  for (const auto& index : m_updates)
    m_matrices[index] = m_transforms[index]->matrix();
  m_updates.clear();

  for (auto& transform : m_transforms)
    transform->m_time += dt;
}

} // namespace ge