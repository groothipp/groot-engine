#pragma once

#include "src/include/rid.hpp"

namespace groot {

class alignas(64) Object {
  friend class Engine;

  RID m_id;
  RID m_mesh;
  RID m_pipeline;
  RID m_set;

  public:
    Object() = default;
    Object(const Object&);
    Object(Object&&) = default;

    ~Object() = default;

    Object& operator=(const Object&);
    Object& operator=(Object&&) = default;

    bool is_in_scene() const;

    void set_mesh(const RID&);
    void set_pipeline(const RID&);
    void set_descriptor_set(const RID&);
};

} // namespace groot