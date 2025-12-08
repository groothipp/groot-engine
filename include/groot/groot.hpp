#pragma once

#include "engine.hpp"
#include "enums.hpp"
#include "linalg.hpp"
#include "log.hpp"
#include "object.hpp"
#include "rid.hpp"
#include "structs.hpp"

#include <numbers>

namespace groot {
  inline float radians(float deg) {
    return std::numbers::pi_v<float> / 180.0 * deg;
  }
}

inline groot::vec2 operator*(float lhs, const groot::vec2& rhs) {
  return rhs * lhs;
}

inline groot::ivec2 operator*(int lhs, const groot::ivec2& rhs) {
  return rhs * lhs;
}

inline groot::uvec2 operator*(unsigned int lhs, const groot::uvec2& rhs) {
  return rhs * lhs;
}

inline groot::vec3 operator*(float lhs, const groot::vec3& rhs) {
  return rhs * lhs;
}

inline groot::ivec3 operator*(int lhs, const groot::ivec3& rhs) {
  return rhs * lhs;
}

inline groot::ivec3 operator*(unsigned int lhs, const groot::uvec3& rhs) {
  return rhs * lhs;
}

inline groot::vec4 operator*(float lhs, const groot::vec4& rhs) {
  return rhs * lhs;
}

inline groot::ivec4 operator*(int lhs, const groot::ivec4& rhs) {
  return rhs * lhs;
}

inline groot::uvec4 operator*(unsigned int lhs, const groot::uvec4& rhs) {
  return rhs * lhs;
}

inline groot::mat2 operator*(float lhs, const groot::mat2& rhs) {
  return rhs * lhs;
}

inline groot::mat3 operator*(float lhs, const groot::mat3& rhs) {
  return rhs * lhs;
}

inline groot::mat4 operator*(float lhs, const groot::mat4& rhs) {
  return rhs * lhs;
}