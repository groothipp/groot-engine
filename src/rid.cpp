#include "src/include/rid.hpp"

#include <functional>

namespace groot {

std::size_t RID::Hash::operator()(const RID& rid) const {
  return std::hash<unsigned long>{}(*rid);
}

RID::RID(unsigned long id, ResourceType type) : m_id(id), m_type(type) {}

bool RID::operator==(const RID& rhs) const {
  return m_id == rhs.m_id;
}

bool RID::operator<(const RID& rhs) const {
  return m_id < rhs.m_id;
}

bool RID::operator>(const RID& rhs) const {
  return m_id > rhs.m_id;
}

const unsigned long& RID::operator*() const {
  return m_id;
}

bool RID::is_valid() const {
  return m_id != ~(0x0);
}

void RID::invalidate() {
  m_id = ~(0x0);
  m_type = ResourceType::Invalid;
}

} // namespace groot