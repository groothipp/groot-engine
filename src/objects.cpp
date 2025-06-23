#include "src/include/allocator.hpp"
#include "src/include/engine.hpp"
#include "src/include/objects.hpp"
#include "src/include/parsers.hpp"

namespace ge {

const ObjectManager::Output ObjectManager::operator[](std::string material) const {
  const ObjectData& obj = m_objects.at(material);

  return {
    m_vertexBuffers[obj.bufferIndex],
    m_indexBuffers[obj.bufferIndex],
    m_indirectBuffers[obj.bufferIndex],
    obj.commands.size(),
  };
}

bool ObjectManager::hasObjects(std::string material) const {
  return m_objects.contains(material);
}

unsigned int ObjectManager::commandSize() const {
  return sizeof(IndirectCommand);
}

const std::map<std::string, std::vector<mat4>> ObjectManager::transforms() const {
  std::map<std::string, std::vector<mat4>> matrices;

  for (auto& [material, data] : m_objects)
    matrices.emplace(material, data.matrices);

  return matrices;
}

transform ObjectManager::add(const std::string& material, const std::string& path, const Transform& transform) {
  auto [vertices, indices] = ObjParser::parse(path);

  ObjectData& obj = m_objects[material];
  obj.commands.emplace_back(IndirectCommand{
    .indexCount     = static_cast<unsigned int>(indices.size()),
    .instanceCount  = 1,
    .firstIndex     = static_cast<unsigned int>(obj.indices.size()),
    .vertexOffset   = static_cast<unsigned int>(obj.vertices.size()),
    .firstInstance  = static_cast<unsigned int>(obj.commands.size())
  });
  obj.vertices.insert(obj.vertices.end(), vertices.begin(), vertices.end());
  obj.indices.insert(obj.indices.end(), indices.begin(), indices.end());
  obj.transforms.emplace_back(std::make_shared<Transform>(transform));
  obj.matrices.emplace_back(transform.matrix());

  obj.transforms.back()->m_manager = this;
  obj.transforms.back()->m_location = { material, obj.transforms.size() - 1 };
  return obj.transforms.back();
}

void ObjectManager::batch(const std::pair<std::string, unsigned int>& location, const std::tuple<vec3, vec3, vec3>& vals) {
  m_updates.insert_or_assign(location, vals);
}

void ObjectManager::updateTransforms() {
  for (auto& [location, vals] : m_updates) {
    auto [position, rotation, scale] = vals;
    auto [material, index] = location;

    m_objects[material].matrices[index] = mat4::translation(position) *
                                          mat4::rotation(rotation) *
                                          mat4::scale(scale);
  }
}

void ObjectManager::updateTimes(double time) {
  for (auto& [material, obj] : m_objects) {
    for (auto& transform : obj.transforms)
      transform->m_time += time;
  }
}

void ObjectManager::load(const Engine& engine) {
  if (m_objects.empty()) return;

  std::vector<vk::BufferCreateInfo> vertInfos;
  std::vector<vk::BufferCreateInfo> indInfos;
  std::vector<vk::BufferCreateInfo> indirInfos;

  for (auto& [material, obj] : m_objects) {
    obj.bufferIndex = vertInfos.size();

    vertInfos.emplace_back(vk::BufferCreateInfo{
      .size         = obj.vertices.size() * sizeof(Vertex),
      .usage        = vk::BufferUsageFlagBits::eTransferSrc,
      .sharingMode  = vk::SharingMode::eExclusive
    });

    indInfos.emplace_back(vk::BufferCreateInfo{
      .size         = obj.indices.size() * sizeof(unsigned int),
      .usage        = vk::BufferUsageFlagBits::eTransferSrc,
      .sharingMode  = vk::SharingMode::eExclusive
    });

    indirInfos.emplace_back(vk::BufferCreateInfo{
      .size         = obj.commands.size() * sizeof(IndirectCommand),
      .usage        = vk::BufferUsageFlagBits::eTransferSrc,
      .sharingMode  = vk::SharingMode::eExclusive
    });
  }

  auto [vertTransMem, vertTransBufs, vertTransOffs, vertTransSize] = Allocator::bufferPool(engine, vertInfos,
    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
  );

  auto [indTransMem, indTransBufs, indTransOffs, indTransSize] = Allocator::bufferPool(engine, indInfos,
    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
  );

  auto [indirTransMem, indirTransBufs, indirTransOffs, indirTransSize] = Allocator::bufferPool(engine, indirInfos,
    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
  );

  void * vertMap = vertTransMem.mapMemory(0, vertTransSize);
  void * indMap = indTransMem.mapMemory(0, indTransSize);
  void * indirMap = indirTransMem.mapMemory(0, indirTransSize);

  std::vector<std::tuple<vk::BufferCopy, vk::BufferCopy, vk::BufferCopy>> copies;
  for (const auto& [material, obj] : m_objects) {
    memcpy(reinterpret_cast<char *>(vertMap) + vertTransOffs[obj.bufferIndex], obj.vertices.data(), obj.vertices.size() * sizeof(Vertex));
    memcpy(reinterpret_cast<char *>(indMap) + indTransOffs[obj.bufferIndex], obj.indices.data(), obj.indices.size() * sizeof(unsigned int));
    memcpy(reinterpret_cast<char *>(indirMap) + indirTransOffs[obj.bufferIndex], obj.commands.data(), obj.commands.size() * sizeof(IndirectCommand));

    vertInfos[obj.bufferIndex].usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    indInfos[obj.bufferIndex].usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    indirInfos[obj.bufferIndex].usage = vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eTransferDst;

    copies.emplace_back(std::tuple<vk::BufferCopy, vk::BufferCopy, vk::BufferCopy>{
      vk::BufferCopy{ .size = obj.vertices.size() * sizeof(Vertex) },
      vk::BufferCopy{ .size = obj.indices.size() * sizeof(unsigned int) },
      vk::BufferCopy{ .size = obj.commands.size() * sizeof(IndirectCommand) }
    });
  }

  vertTransMem.unmapMemory();
  indTransMem.unmapMemory();
  indirTransMem.unmapMemory();
  vertMap = indMap = indirMap = nullptr;

  auto [tmp_vertMem, tmp_vertBufs, _v1, _v2] = Allocator::bufferPool(engine, vertInfos,
    vk::MemoryPropertyFlagBits::eDeviceLocal
  );
  m_vertexMemory = std::move(tmp_vertMem);
  m_vertexBuffers = std::move(tmp_vertBufs);

  auto [tmp_indMem, tmp_indBufs, _i1, _i2] = Allocator::bufferPool(engine, indInfos,
    vk::MemoryPropertyFlagBits::eDeviceLocal
  );
  m_indexMemory = std::move(tmp_indMem);
  m_indexBuffers = std::move(tmp_indBufs);

  auto [tmp_indirMem, tmp_indirBufs, _in1, _in2] = Allocator::bufferPool(engine, indirInfos,
    vk::MemoryPropertyFlagBits::eDeviceLocal
  );
  m_indirectMemory = std::move(tmp_indirMem);
  m_indirectBuffers = std::move(tmp_indirBufs);

  vk::raii::CommandBuffer transferCmd = std::move(engine.getCmds(QueueFamilyType::Transfer, 1)[0]);
  transferCmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

  unsigned int index = 0;
  for (const auto& [vertCopy, indCopy, indirCopy] : copies) {
    transferCmd.copyBuffer(vertTransBufs[index], m_vertexBuffers[index], vertCopy);
    transferCmd.copyBuffer(indTransBufs[index], m_indexBuffers[index], indCopy);
    transferCmd.copyBuffer(indirTransBufs[index], m_indirectBuffers[index], indirCopy);
    ++index;
  }

  transferCmd.end();

  vk::SubmitInfo transferSubmit{
    .commandBufferCount = 1,
    .pCommandBuffers    = &*transferCmd
  };

  vk::raii::Fence transferFence = std::move(Allocator::fences(engine, 1)[0]);

  engine.m_context.queueFamily(QueueFamilyType::Transfer).queue.submit(transferSubmit, transferFence);

  if (engine.m_context.device().waitForFences(*transferFence, true, ge_timeout) != vk::Result::eSuccess)
    throw std::runtime_error("groot-engine: hung waiting for objects buffer transfer");
}

} // namespace ge