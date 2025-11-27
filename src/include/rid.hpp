#pragma once

#include "src/include/enums.hpp"

#include <cstddef>

namespace groot {

class RID {
  friend class Engine;

  unsigned long m_id = ~(0x0);
  ResourceType m_type = ResourceType::Invalid;

  public:
    struct Hash {
      std::size_t operator()(const RID&) const;
    };

  public:
    RID() = default;
    RID(const RID&) = default;
    RID(RID&&) = default;

    ~RID() = default;

    RID& operator=(const RID&) = default;
    RID& operator=(RID&&) = default;

    bool operator==(const RID&) const;
    bool operator<(const RID&) const;
    bool operator>(const RID&) const;
    const unsigned long& operator*() const;

    bool is_valid() const;

  private:
    explicit RID(unsigned long, ResourceType);

    void invalidate();
};

} // namespace groot