#include "src/include/log.hpp"
#include "src/include/object.hpp"

namespace groot {

Object::Object(const Object& obj)
: m_id(RID()), m_mesh(obj.m_mesh), m_pipeline(obj.m_pipeline), m_set(obj.m_set) {}

Object& Object::operator=(const Object& obj) {
  if (this == &obj) return *this;

  m_id = RID();
  m_mesh = obj.m_mesh;
  m_pipeline = obj.m_pipeline;
  m_set = obj.m_set;

  return *this;
}

bool Object::operator<(const Object& rhs) const {
  return m_id < rhs.m_id;
}

bool Object::is_in_scene() const {
  return m_id.is_valid();
}

void Object::set_mesh(const RID& rid) {
  if (!rid.is_valid()) {
    Log::warn("tried to set object mesh to invalid RID");
    return;
  }

  if (rid.m_type != ResourceType::Mesh) {
    Log::warn("tried to set object mesh to non-mesh RID");
    return;
  }

  m_mesh = rid;
}

void Object::set_pipeline(const RID& rid) {
  if (!rid.is_valid()) {
    Log::warn("tried to set object pipeline to invlaid RID");
    return;
  }

  if (rid.m_type != ResourceType::Pipeline) {
    Log::warn("tried to set object pipeline to non-pipeline RID");
    return;
  }

  m_pipeline = rid;
}

void Object::set_descriptor_set(const RID& rid) {
  if (!rid.is_valid()) {
    Log::warn("tried to set object descriptor set to invalid RID");
    return;
  }

  if (rid.m_type != ResourceType::DescriptorSet) {
    Log::warn("tried to set object descriptor set to non-descriptor-set RID");
    return;
  }

  m_set = rid;
}

} // namespace groot