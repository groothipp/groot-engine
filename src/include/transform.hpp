#pragma once

#include "src/include/linalg.hpp"

namespace ge {

class Object;

class Transform {
  friend class ObjectManager;
  friend class Object;

  public:
    Transform() = default;
    Transform(const Transform&) = default;
    Transform(Transform&&) = default;
    Transform(const vec3&, const vec3&, const vec3&);

    ~Transform() = default;

    Transform& operator=(const Transform&) = default;
    Transform& operator=(Transform&&) = default;

    const vec3& position() const;
    const vec3& rotation() const;
    const vec3& scale() const;
    const double& elapsed_time() const;
    const mat4 matrix() const;

    void translate(const vec3&);
    void rotate(const vec3&);
    void resize(float);
    void resize(const vec3&);
    void set_position(const vec3&);
    void set_rotation(const vec3&);
    void set_scale(float);
    void set_scale(const vec3&);

  private:
    Object * m_object = nullptr;
    unsigned int m_index = 0;

    vec3 m_position = vec3(0.0f);
    vec3 m_rotation = vec3(0.0f);
    vec3 m_scale = vec3(1.0f);
    double m_time = 0.0;
};

} // namespace ge