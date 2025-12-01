#pragma once

#include <cmath>
#include <compare>

namespace groot::detail {

template <typename T>
struct Vec2 {
  T x = T{}, y = T{};

  inline Vec2() = default;
  inline explicit Vec2(T s) : x(s), y(s) {}
  inline Vec2(T a, T b) : x(a), y(b) {}
  inline Vec2(const Vec2&) = default;
  inline Vec2(Vec2&&) = default;

  template <typename K>
  inline Vec2(const Vec2<K>& vec) : x(static_cast<T>(vec.x)), y(static_cast<T>(vec.y)) {}

  inline ~Vec2() = default;

  inline Vec2& operator=(const Vec2&) = default;
  inline Vec2& operator=(Vec2&&) = default;

  inline std::partial_ordering operator<=>(const Vec2&) const = default;

  inline Vec2 operator+(const Vec2& rhs) const { return Vec2(x + rhs.x, y + rhs.y); }
  inline Vec2 operator-(const Vec2& rhs) const { return Vec2(x - rhs.x, y - rhs.y); }
  inline Vec2 operator-() const { return Vec2(-x, -y); }
  inline Vec2 operator*(const Vec2& rhs) const { return Vec2(x * rhs.x, y * rhs.y); }
  inline Vec2 operator*(T s) const { return Vec2(x * s, y * s); }
  inline Vec2 operator/(const Vec2& rhs) const { return Vec2( x / rhs.x, y / rhs.y ); }
  inline Vec2 operator/(T s) const { return Vec2(x / s, y / s); }

  inline double dot(const Vec2& vec) const { return x * vec.x + y * vec.y; }
  inline double mag() const { return std::sqrt(dot(*this)); }
  inline double mag_squared() const { return dot(*this); }
  inline Vec2<float> normalized() const { return *this / mag(); }
};

template <typename T>
struct Vec3 {
  T x{}, y{}, z{};

  inline Vec3() = default;
  inline explicit Vec3(T s) : x(s), y(s), z(s) {}
  inline Vec3(T a, T b, T c) : x(a), y(b), z(c) {}
  inline Vec3(const Vec2<T>& u, T s) : x(u.x), y(u.y), z(s) {}
  inline Vec3(const Vec3&) = default;
  inline Vec3(Vec3&&) = default;

  template <typename K>
  inline Vec3(const Vec3<K>& vec) : x(static_cast<T>(vec.x)), y(static_cast<T>(vec.y)), z(static_cast<T>(vec.z)) {}

  inline ~Vec3() = default;

  inline Vec3& operator=(const Vec3&) = default;
  inline Vec3& operator=(Vec3&&) = default;

  inline std::partial_ordering operator<=>(const Vec3&) const = default;

  inline Vec3 operator+(const Vec3& rhs) const { return Vec3(x + rhs.x, y + rhs.y, z + rhs.z); }
  inline Vec3 operator-(const Vec3& rhs) const { return Vec3(x - rhs.x, y - rhs.y, z - rhs.z); }
  inline Vec3 operator-() const { return Vec3(-x, -y, -z); }
  inline Vec3 operator*(const Vec3& rhs) const { return Vec3(rhs.x * x, rhs.y * y, rhs.z * z); }
  inline Vec3 operator*(T rhs) const { return Vec3(rhs * x, rhs * y, rhs * z); }
  inline Vec3 operator/(const Vec3& rhs) const { return Vec3(x / rhs.x, y / rhs.y, z / rhs.z); }
  inline Vec3 operator/(T rhs) const { return Vec3(x / rhs, y / rhs, z / rhs); }

  inline double dot(const Vec3& vec) const { return x * vec.x + y * vec.y + z * vec.z; }
  inline Vec3 cross(const Vec3& vec) const { return Vec3(y * vec.z - vec.z * y, z * vec.x - x * vec.z, x * vec.y - y * vec.x); }
  inline double mag() const { return std::sqrt(dot(*this)); }
  inline double mag_squared() const { return dot(*this); }
  inline Vec3 normalized() const { return *this / mag(); }
};

template<typename T>
struct Vec4 {
  T x{}, y{}, z{}, w{};

  inline Vec4() = default;
  inline Vec4(T s) : x(s), y(s), z(s), w(s) {}
  inline Vec4(T a, T b, T c, T d) : x(a), y(b), z(c), w(d) {}
  inline Vec4(const Vec2<T>& vec, T a, T b) : x(vec.x), y(vec.y), z(a), w(b) {}
  inline Vec4(const Vec3<T>& vec, T s) : x(vec.x), y(vec.y), z(vec.z), w(s) {}
  inline Vec4(const Vec4&) = default;
  inline Vec4(Vec4&&) = default;

  template <typename K>
  inline Vec4(const Vec4<K>& vec) : x(static_cast<T>(vec.x)), y(static_cast<T>(vec.y)), z(static_cast<T>(vec.z)), w(static_cast<T>(vec.w)) {}

  inline ~Vec4() = default;

  inline Vec4& operator=(const Vec4&) = default;
  inline Vec4& operator=(Vec4&&) = default;

  inline std::partial_ordering operator<=>(const Vec4&) const = default;

  inline Vec4 operator+(const Vec4& rhs) const { return Vec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
  inline Vec4 operator-(const Vec4& rhs) const { return Vec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
  inline Vec4 operator-() const { return Vec4(-x, -y, -z, -w); }
  inline Vec4 operator*(const Vec4& rhs) const { return Vec4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
  inline Vec4 operator*(T rhs) const { return Vec4(x * rhs, y * rhs, z * rhs, w * rhs); }
  inline Vec4 operator/(const Vec4& rhs) const { return Vec4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }
  inline Vec4 operator/(T rhs) const { return Vec4(x / rhs, y / rhs, z / rhs, w / rhs); }

  inline double dot(const Vec4& vec) const { return x * vec.x + y * vec.y + z * vec.z + w * vec.w; }
  inline double mag() const { return std::sqrt(dot(*this)); }
  inline double mag_squared() const { return dot(*this); }
  inline Vec4 normalized() const { return *this / mag(); }
};

struct Mat4 {

};

} // namespace groot::detail

namespace groot {

using vec2  = detail::Vec2<float>;
using ivec2 = detail::Vec2<int>;
using uvec2 = detail::Vec2<unsigned int>;
using vec3  = detail::Vec3<float>;
using ivec3 = detail::Vec3<int>;
using uvec3 = detail::Vec3<unsigned int>;
using vec4  = detail::Vec4<float>;
using ivec4 = detail::Vec4<int>;
using uvec4 = detail::Vec4<unsigned int>;

} // namespace groot

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

inline groot::uvec3 operator*(unsigned int lhs, const groot::uvec3& rhs) {
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

