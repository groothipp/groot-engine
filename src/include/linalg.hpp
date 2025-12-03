#pragma once

#include "src/include/log.hpp"

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

  inline T& operator[](unsigned int index) {
    switch (index) {
      case 0: return x;
      case 1: return y;
      default:
        Log::out_of_range("vec2 access out of range");
        return x;
    }
  }

  inline const T& operator[](unsigned int index) const {
    switch(index) {
      case 0:   return x;
      case 1:   return y;
      default:
        Log::out_of_range("vec2 access out of range");
        return x;
    }
  }

  inline std::partial_ordering operator<=>(const Vec2&) const = default;

  inline Vec2 operator+(const Vec2& rhs) const { return Vec2(x + rhs.x, y + rhs.y); }
  inline Vec2 operator-(const Vec2& rhs) const { return Vec2(x - rhs.x, y - rhs.y); }
  inline Vec2 operator-() const { return Vec2(-x, -y); }
  inline Vec2 operator*(const Vec2& rhs) const { return Vec2(x * rhs.x, y * rhs.y); }
  inline Vec2 operator*(T s) const { return Vec2(x * s, y * s); }
  inline Vec2 operator/(const Vec2& rhs) const { return Vec2( x / rhs.x, y / rhs.y ); }
  inline Vec2 operator/(T s) const { return Vec2(x / s, y / s); }

  inline T dot(const Vec2& vec) const { return x * vec.x + y * vec.y; }
  inline T mag() const { return std::sqrt(dot(*this)); }
  inline T mag_squared() const { return dot(*this); }
  inline Vec2 normalized() const { return *this / mag(); }
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

  inline T& operator[](unsigned int index) {
    switch (index) {
      case 0: return x;
      case 1: return y;
      case 2: return z;
      default:
        Log::out_of_range("vec3 access out of range");
        return x;
    }
  }

  inline const T& operator[](unsigned int index) const {
    switch (index) {
      case 0: return x;
      case 1: return y;
      case 2: return z;
      default:
        Log::out_of_range("vec3 access out of range");
        return x;
    }
  }

  inline std::partial_ordering operator<=>(const Vec3&) const = default;

  inline Vec3 operator+(const Vec3& rhs) const { return Vec3(x + rhs.x, y + rhs.y, z + rhs.z); }
  inline Vec3 operator-(const Vec3& rhs) const { return Vec3(x - rhs.x, y - rhs.y, z - rhs.z); }
  inline Vec3 operator-() const { return Vec3(-x, -y, -z); }
  inline Vec3 operator*(const Vec3& rhs) const { return Vec3(rhs.x * x, rhs.y * y, rhs.z * z); }
  inline Vec3 operator*(T rhs) const { return Vec3(rhs * x, rhs * y, rhs * z); }
  inline Vec3 operator/(const Vec3& rhs) const { return Vec3(x / rhs.x, y / rhs.y, z / rhs.z); }
  inline Vec3 operator/(T rhs) const { return Vec3(x / rhs, y / rhs, z / rhs); }

  inline T dot(const Vec3& vec) const { return x * vec.x + y * vec.y + z * vec.z; }
  inline Vec3 cross(const Vec3& vec) const { return Vec3(y * vec.z - vec.z * y, z * vec.x - x * vec.z, x * vec.y - y * vec.x); }
  inline T mag() const { return std::sqrt(dot(*this)); }
  inline T mag_squared() const { return dot(*this); }
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

  inline T& operator[](unsigned int index) {
    switch (index) {
      case 0: return x;
      case 1: return y;
      case 2: return z;
      case 3: return w;
      default:
        Log::out_of_range("vec3 access out of range");
        return x;
    }
  }

  inline const T& operator[](unsigned int index) const {
    switch (index) {
      case 0: return x;
      case 1: return y;
      case 2: return z;
      case 3: return w;
      default:
        Log::out_of_range("vec3 access out of range");
        return x;
    }
  }

  inline std::partial_ordering operator<=>(const Vec4&) const = default;

  inline Vec4 operator+(const Vec4& rhs) const { return Vec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
  inline Vec4 operator-(const Vec4& rhs) const { return Vec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
  inline Vec4 operator-() const { return Vec4(-x, -y, -z, -w); }
  inline Vec4 operator*(const Vec4& rhs) const { return Vec4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
  inline Vec4 operator*(T rhs) const { return Vec4(x * rhs, y * rhs, z * rhs, w * rhs); }
  inline Vec4 operator/(const Vec4& rhs) const { return Vec4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }
  inline Vec4 operator/(T rhs) const { return Vec4(x / rhs, y / rhs, z / rhs, w / rhs); }

  inline T dot(const Vec4& vec) const { return x * vec.x + y * vec.y + z * vec.z + w * vec.w; }
  inline T mag() const { return std::sqrt(dot(*this)); }
  inline T mag_squared() const { return dot(*this); }
  inline Vec4 normalized() const { return *this / mag(); }
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

class mat2 {
  vec2 m_col1, m_col2;

  public:
    mat2() = default;
    explicit mat2(float);
    mat2(const vec2&, const vec2&);
    mat2(const mat2&) = default;
    mat2(mat2&&) = default;

    ~mat2() = default;

    mat2& operator=(const mat2&) = default;
    mat2& operator=(mat2&&) = default;

    vec2& operator[](unsigned int);
    const vec2& operator[](unsigned int) const;

    std::partial_ordering operator<=>(const mat2&) const = default;

    mat2 operator+(const mat2&) const;
    mat2 operator-(const mat2&) const;
    mat2 operator-() const;
    mat2 operator*(const mat2&) const;
    vec2 operator*(const vec2&) const;
    mat2 operator*(float) const;
    mat2 operator/(float) const;

    mat2 inverse() const;
    mat2 transpose() const;
    float determinant() const;
    float trace() const;

    static mat2 identity();
    static mat2 rotation(float);
    static mat2 scale(float, float);
};

class mat3 {
  vec3 m_col1, m_col2, m_col3;

  public:
    mat3() = default;
    explicit mat3(float);
    mat3(const vec3&, const vec3&, const vec3&);
    explicit mat3(const mat2&, float s = 0);
    mat3(const mat3&) = default;
    mat3(mat3&&) = default;

    ~mat3() = default;

    mat3& operator=(const mat3&) = default;
    mat3& operator=(mat3&&) = default;

    vec3& operator[](unsigned int);
    const vec3& operator[](unsigned int) const;

    std::partial_ordering operator<=>(const mat3&) const = default;

    mat3 operator+(const mat3&) const;
    mat3 operator-(const mat3&) const;
    mat3 operator-() const;
    mat3 operator*(const mat3&) const;
    vec3 operator*(const vec3&) const;
    mat3 operator*(float) const;
    mat3 operator/(float) const;

    mat3 inverse() const;
    mat3 transpose() const;
    float determinant() const;
    float trace() const;

    static mat3 identity();
    static mat3 rotation_x(float);
    static mat3 rotation_y(float);
    static mat3 rotation_z(float);
    static mat3 rotation(const vec3&, float);
    static mat3 euler_rotation(float, float, float);
    static mat3 scale(float, float, float);
};

class mat4 {
  vec4 m_col1, m_col2, m_col3, m_col4;

  public:
    mat4() = default;
    explicit mat4(float);
    mat4(const vec4&, const vec4&, const vec4&, const vec4&);
    explicit mat4(const mat2&, float s = 0.0f);
    explicit mat4(const mat3&, float s = 0.0f);
    mat4(const mat4&) = default;
    mat4(mat4&&) = default;

    ~mat4() = default;

    mat4& operator=(const mat4&) = default;
    mat4& operator=(mat4&&) = default;

    vec4& operator[](unsigned int);
    const vec4& operator[](unsigned int) const;

    std::partial_ordering operator<=>(const mat4&) const = default;

    mat4 operator+(const mat4&) const;
    mat4 operator-(const mat4&) const;
    mat4 operator-() const;
    mat4 operator*(const mat4&) const;
    vec4 operator*(const vec4&) const;
    mat4 operator*(float) const;
    mat4 operator/(float) const;

    mat4 inverse() const;
    mat4 transpose() const;
    float determinant() const;
    float trace() const;

    static mat4 identity();
    static mat4 translation(const vec3&);
    static mat4 rotation(const vec3&, float);
    static mat4 scale(float, float, float);
    static mat4 view(const vec3&, const vec3&, const vec3&);
    static mat4 perspective_projection(float, float, float, float);

  private:
    mat3 getMinorMatrix(unsigned int, unsigned int) const;
};

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

inline groot::mat2 operator*(float lhs, const groot::mat2& rhs) {
  return rhs * lhs;
}

inline groot::mat3 operator*(float lhs, const groot::mat3& rhs) {
  return rhs * lhs;
}

inline groot::mat4 operator*(float lhs, const groot::mat4& rhs) {
  return rhs * lhs;
}