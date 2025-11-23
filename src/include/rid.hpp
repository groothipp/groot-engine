#pragma once

#include <cstddef>

namespace groot {

enum ResourceType {
  Invalid,
  Shader,
  Pipeline,
  DescriptorSet,
  UniformBuffer,
  StorageBuffer
};

class RID {
  friend class Engine;

  unsigned long m_id = ~(0x0);
  ResourceType m_type = ResourceType::Invalid;

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
    const unsigned long& operator*() const;

    bool is_valid() const;

  private:
    explicit RID(unsigned long, ResourceType);

    void invalidate();
};

} // namespace groot