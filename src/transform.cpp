#include "src/include/objects.hpp"
#include "src/include/transform.hpp"

namespace ge {

Transform::Transform(const vec3& p, const vec3& r, const vec3& s) : m_position(p), m_rotation(r), m_scale(s) {}

const vec3& Transform::position() const {
  return m_position;
}

const vec3& Transform::rotation() const {
  return m_rotation;
}

const vec3& Transform::scale() const {
  return m_scale;
}

const double& Transform::elapsed_time() const {
  return m_time;
}

const mat4 Transform::matrix() const {
  return mat4::translation(m_position) * mat4::rotation(m_rotation) * mat4::scale(m_scale);
}

void Transform::translate(const vec3& p) {
  m_position += p;
  m_object->batch(m_index);
}

void Transform::rotate(const vec3& r) {
  vec3 rot(radians(r.x), radians(r.y), radians(r.z));
  m_rotation += rot;
  m_object->batch(m_index);
}

void Transform::resize(float s) {
  m_scale *= s;
  m_object->batch(m_index);
}

void Transform::resize(const vec3& s) {
  m_scale += s;
  m_object->batch(m_index);
}

void Transform::set_position(const vec3& p) {
  m_position = p;
  m_object->batch(m_index);
}

void Transform::set_rotation(const vec3& r) {
  vec3 rot(radians(r.x), radians(r.y), radians(r.z));
  m_rotation = rot;
  m_object->batch(m_index);
}

void Transform::set_scale(float s) {
  m_scale = vec3(s, s, s);
  m_object->batch(m_index);
}

void Transform::set_scale(const vec3& s) {
  m_scale = s;
  m_object->batch(m_index);
}

} // namespace ge