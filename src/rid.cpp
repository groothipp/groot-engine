#include "src/include/rid.hpp"

namespace groot {

RID::RID(unsigned long id) : m_id(id) {}

const unsigned long& RID::operator*() const {
  return m_id;
}

bool RID::is_valid() const {
  return m_id != ~(0x0);
}

} // namespace groot