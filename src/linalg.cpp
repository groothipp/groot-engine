#include "src/include/linalg.hpp"

#define SINGLUAR_TOLERANCE 1e-8

namespace groot {

mat2::mat2(float s) : m_col1(vec2(s)), m_col2(vec2(s)) {}

mat2::mat2(const vec2& col1, const vec2& col2) : m_col1(col1), m_col2(col2) {}

vec2& mat2::operator[](unsigned int index) {
  switch (index) {
    case 0: return m_col1;
    case 1: return m_col2;
    default:
      Log::out_of_range("mat2 access out of range");
      return m_col1;
  }
}

const vec2& mat2::operator[](unsigned int index) const {
  switch (index) {
    case 0: return m_col1;
    case 1: return m_col2;
    default:
      Log::out_of_range("mat2 access out of range");
      return m_col1;
  }
}

mat2 mat2::operator+(const mat2& rhs) const {
  return mat2(m_col1 + rhs.m_col1, m_col2 + rhs.m_col2);
}

mat2 mat2::operator-(const mat2& rhs) const {
  return mat2(m_col1 - rhs.m_col1, m_col2 - rhs.m_col2);
}

mat2 mat2::operator-() const {
  return mat2(-m_col1, -m_col2);
}

mat2 mat2::operator*(const mat2& rhs) const {
  return mat2(*this * rhs.m_col1, *this * rhs.m_col2);
}

vec2 mat2::operator*(const vec2& rhs) const {
  return rhs.x * m_col1 + rhs.y * m_col2;
}

mat2 mat2::operator*(float rhs) const {
  return mat2(rhs * m_col1, rhs * m_col2);
}

mat2 mat2::operator/(float rhs) const {
  return mat2(m_col1 / rhs, m_col2 / rhs);
}

mat2 mat2::inverse() const {
  float det = determinant();

  if (std::abs(det) < SINGLUAR_TOLERANCE) {
    Log::warn("tried to take inverse of a singular matrix");
    return mat2();
  }

  vec2 col1(m_col2.y / det, -m_col1.y / det);
  vec2 col2(-m_col2.x / det, m_col1.x / det);

  return mat2(col1, col2);
}

mat2 mat2::transpose() const {
  return mat2(vec2(m_col1.x, m_col2.x), vec2(m_col1.y, m_col2.y));
}

float mat2::determinant() const {
  return m_col1.x * m_col2.y - m_col2.x * m_col1.y;
}

float mat2::trace() const {
  return m_col1.x + m_col2.y;
}

mat2 mat2::identity() {
  return mat2(vec2(1.0f, 0.0f), vec2(0.0f, 1.0f));
}

mat2 mat2::rotation(float theta) {
  float c = std::cos(theta);
  float s = std::sin(theta);

  return mat2(vec2(c, s), vec2(-s, c));
}

mat2 mat2::scale(float sx, float sy) {
  return mat2(vec2(sx, 0.0f), vec2(0.0f, sy));
}

mat3::mat3(float s) : m_col1(vec3(s)), m_col2(vec3(s)), m_col3(vec3(s)) {}

mat3::mat3(const vec3& col1, const vec3& col2, const vec3& col3) : m_col1(col1), m_col2(col2), m_col3(col3) {}

mat3::mat3(const mat2& m, float s) : m_col1(vec3(m[0], s)), m_col2(vec3(m[1], s)), m_col3(vec3(s)) {}

vec3& mat3::operator[](unsigned int index) {
  switch (index) {
    case 0: return m_col1;
    case 1: return m_col2;
    case 2: return m_col3;
    default:
      Log::out_of_range("mat3 access out of range");
      return m_col1;
  }
}

const vec3& mat3::operator[](unsigned int index) const {
  switch (index) {
    case 0: return m_col1;
    case 1: return m_col2;
    case 2: return m_col3;
    default:
      Log::out_of_range("mat3 access out of range");
      return m_col1;
  }
}

mat3 mat3::operator+(const mat3& rhs) const {
  return mat3(m_col1 + rhs.m_col1, m_col2 + rhs.m_col2, m_col3 + rhs.m_col3);
}

mat3 mat3::operator-(const mat3& rhs) const {
  return mat3(m_col1 - rhs.m_col1, m_col2 - rhs.m_col2, m_col3 - rhs.m_col3);
}

mat3 mat3::operator-() const {
  return mat3(-m_col1, -m_col2, -m_col3);
}

mat3 mat3::operator*(const mat3& rhs) const {
  return mat3(*this * rhs.m_col1, *this * rhs.m_col2, *this * rhs.m_col3);
}

vec3 mat3::operator*(const vec3& rhs) const {
  return rhs.x * m_col1 + rhs.y * m_col2 + rhs.z * m_col3;
}

mat3 mat3::operator*(float s) const {
  return mat3(m_col1 * s, m_col2 * s, m_col3 * s);
}

mat3 mat3::operator/(float s) const {
  return mat3(m_col1 / s, m_col2 / s, m_col3 / s);
}

mat3 mat3::inverse() const {
  float det = determinant();

  if (std::abs(det) < SINGLUAR_TOLERANCE) {
    Log::warn("tried to take inverse of a singular matrix");
    return mat3();
  }

  vec3 c1 = vec3(
    m_col2.y * m_col3.z - m_col2.z * m_col3.y,
    m_col2.z * m_col3.x - m_col2.x * m_col3.z,
    m_col2.x * m_col3.y - m_col2.y * m_col3.x
  );

  vec3 c2 = vec3(
    m_col1.z * m_col3.y - m_col1.y * m_col3.z,
    m_col1.x * m_col3.z - m_col1.z * m_col3.x,
    m_col1.y * m_col3.x - m_col1.x * m_col3.y
  );

  vec3 c3 = vec3(
    m_col1.y * m_col2.z - m_col1.z * m_col2.y,
    m_col1.z * m_col2.x - m_col1.x * m_col2.z,
    m_col1.x * m_col2.y - m_col1.y * m_col2.x
  );

  return mat3(c1, c2, c3).transpose() / det;
}

mat3 mat3::transpose() const {
  vec3 row1(m_col1.x, m_col2.x, m_col3.x);
  vec3 row2(m_col1.y, m_col2.y, m_col3.y);
  vec3 row3(m_col1.z, m_col2.z, m_col3.z);

  return mat3(row1, row2, row3);
}

float mat3::determinant() const {
  return m_col1.x * (m_col2.y * m_col3.z - m_col2.z * m_col3.y)
       + m_col1.y * (m_col3.x * m_col2.z - m_col3.z * m_col2.x)
       + m_col1.z * (m_col2.x * m_col3.y - m_col2.y * m_col3.x);
}

float mat3::trace() const {
  return m_col1.x + m_col2.y + m_col3.z;
}

mat3 mat3::identity() {
  return mat3(vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
}

mat3 mat3::rotation_x(float theta) {
  float c = std::cos(theta);
  float s = std::sin(theta);

  return mat3(
    vec3(1.0f, 0.0f, 0.0f),
    vec3(0.0f, c, s),
    vec3(0.0f, -s, c)
  );
}

mat3 mat3::rotation_y(float theta) {
  float c = std::cos(theta);
  float s = std::sin(theta);

  return mat3(
    vec3(c, 0.0f, -s),
    vec3(0.0f, 1.0f, 0.0f),
    vec3(s, 0.0f, c)
  );
}

mat3 mat3::rotation_z(float theta) {
  float c = std::cos(theta);
  float s = std::sin(theta);

  return mat3(
    vec3(c, s, 0.0f),
    vec3(-s, c, 0.0f),
    vec3(0.0f, 0.0f, 1.0f)
  );
}

mat3 mat3::rotation(const vec3& axis, float theta) {
  vec3 a = axis.normalized();
  float c = std::cos(theta);
  float s = std::sin(theta);
  float t = 1.0f - c;

  return mat3(
    vec3(t * a.x * a.x + c, t * a.x * a.y + s * a.z, t * a.x * a.z - s * a.y),
    vec3(t * a.x * a.y - s * a.z, t * a.y * a.y + c, t * a.y * a.z + s * a.x),
    vec3(t * a.x * a.z + s * a.y, t * a.y * a.z - s * a.x, t * a.z * a.z + c)
  );
}

mat3 mat3::euler_rotation(float phi, float theta, float psi) {
  return rotation_z(psi) * rotation_x(phi) * rotation_y(theta);
}

mat3 mat3::scale(float sx, float sy, float sz) {
  return mat3(
    vec3(sx, 0.0f, 0.0f),
    vec3(0.0f, sy, 0.0f),
    vec3(0.0f, 0.0f, sz)
  );
}

mat4::mat4(float s) : m_col1(vec4(s)), m_col2(vec4(s)), m_col3(vec4(s)), m_col4(vec4(s)) {}

mat4::mat4(const vec4& col1, const vec4& col2, const vec4& col3, const vec4& col4)
: m_col1(col1), m_col2(col2), m_col3(col3), m_col4(col4) {}

mat4::mat4(const mat2& m, float s)
: m_col1(vec4(m[0], s, s)), m_col2(vec4(m[1], s, s)), m_col3(vec4(s)), m_col4(vec4(s)) {}

mat4::mat4(const mat3& m, float s)
: m_col1(vec4(m[0], s)), m_col2(vec4(m[1], s)), m_col3(vec4(m[2], s)), m_col4(vec4(s)) {}

vec4& mat4::operator[](unsigned int index) {
  switch (index) {
    case 0: return m_col1;
    case 1: return m_col2;
    case 2: return m_col3;
    case 3: return m_col4;
    default:
      Log::out_of_range("mat4 access out of range");
      return m_col1;
  }
}

const vec4& mat4::operator[](unsigned int index) const {
  switch (index) {
    case 0: return m_col1;
    case 1: return m_col2;
    case 2: return m_col3;
    case 3: return m_col4;
    default:
      Log::out_of_range("mat4 access out of range");
      return m_col1;
  }
}

mat4 mat4::operator+(const mat4& rhs) const {
  return mat4(
    m_col1 + rhs.m_col1,
    m_col2 + rhs.m_col2,
    m_col3 + rhs.m_col3,
    m_col4 + rhs.m_col4
  );
}

mat4 mat4::operator-(const mat4& rhs) const {
  return mat4(
    m_col1 - rhs.m_col1,
    m_col2 - rhs.m_col2,
    m_col3 - rhs.m_col3,
    m_col4 - rhs.m_col4
  );
}

mat4 mat4::operator-() const {
  return mat4(-m_col1, -m_col2, -m_col3, -m_col4);
}

mat4 mat4::operator*(const mat4& rhs) const {
  return mat4(
    *this * rhs.m_col1,
    *this * rhs.m_col2,
    *this * rhs.m_col3,
    *this * rhs.m_col4
  );
}

vec4 mat4::operator*(const vec4& rhs) const {
  return rhs.x * m_col1 + rhs.y * m_col2 + rhs.z * m_col3 + rhs.w * m_col4;
}

mat4 mat4::operator*(float s) const {
  return mat4(s * m_col1, s * m_col2, s * m_col3, s * m_col4);
}

mat4 mat4::operator/(float s) const {
  return mat4(m_col1 / s, m_col2 / s, m_col3 / s, m_col4 / s);
}

mat4 mat4::inverse() const {
  float det = determinant();
  if (std::abs(det) < SINGLUAR_TOLERANCE) {
    Log::warn("tried to take the inverse of a singular matrix");
    return mat4();
  }

  mat4 cofactor;
  for (unsigned int col = 0; col < 4; ++col) {
    for (unsigned int row = 0; row < 4; ++row) {
      mat3 minor = getMinorMatrix(row, col);
      float sign = ((row + col) & 1) ? -1.0f : 1.0f;
      cofactor[col][row] = sign * minor.determinant();
    }
  }

  return cofactor.transpose() / det;
}

mat4 mat4::transpose() const {
  return mat4(
    vec4(m_col1.x, m_col2.x, m_col3.x, m_col4.x),
    vec4(m_col1.y, m_col2.y, m_col3.y, m_col4.y),
    vec4(m_col1.z, m_col2.z, m_col3.z, m_col4.z),
    vec4(m_col1.w, m_col2.w, m_col3.w, m_col4.w)
  );
}

float mat4::determinant() const {
  float det = 0.0f;

  for (unsigned int i = 0; i < 4; ++i) {
    float sign = (i & 1) ? -1.0f : 1.0f;
    det += sign * (*this)[0][i] * getMinorMatrix(i, 0).determinant();
  }

  return det;
}

float mat4::trace() const {
  return m_col1.x + m_col2.y + m_col3.z + m_col4.w;
}

mat4 mat4::identity() {
  return mat4(
    vec4(1.0f, 0.0f, 0.0f, 0.0f),
    vec4(0.0f, 1.0f, 0.0f, 0.0f),
    vec4(0.0f, 0.0f, 1.0f, 0.0f),
    vec4(0.0f, 0.0f, 0.0f, 1.0f)
  );
}

mat4 mat4::translation(const vec3 & pos) {
  return mat4(
    vec4(1.0f, 0.0f, 0.0f, 0.0f),
    vec4(0.0f, 1.0f, 0.0f, 0.0f),
    vec4(0.0f, 0.0f, 1.0f, 0.0f),
    vec4(pos, 1.0f)
  );
}

mat4 mat4::rotation(const vec3 & axis, float angle) {
  mat4 rot(mat3::rotation(axis, angle));
  rot.m_col4.w = 1.0f;
  return rot;
}

mat4 mat4::scale(float sx, float sy, float sz) {
  mat4 s(mat3::scale(sx, sy, sz));
  s.m_col4.w = 1.0f;
  return s;
}

mat4 mat4::view(const vec3& eye, const vec3& target, const vec3& up) {
  vec3 f = (target - eye).normalized();
  vec3 r = f.cross(up).normalized();
  vec3 u = r.cross(f).normalized();

  return mat4(
    vec4(r.x, u.x, f.x, 0.0f),
    vec4(r.y, u.y, f.y, 0.0f),
    vec4(r.z, u.z, f.z, 0.0f),
    vec4(-r.dot(eye), -u.dot(eye), -f.dot(eye), 1.0f)
  );
}

mat4 mat4::perspective_projection(float fov, float ar, float near, float far) {
  float tanFov = std::tan(0.5 * fov);
  float range = far - near;

  return mat4(
    vec4(1.0f / (ar * tanFov), 0.0f, 0.0f, 0.0f),
    vec4(0.0f, -1.0f / tanFov, 0.0f, 0.0f),
    vec4(0.0f, 0.0f, far / range, 1.0f),
    vec4(0.0f, 0.0f, -(far * near) / range, 0.0f)
  );
}

mat3 mat4::getMinorMatrix(unsigned int skipRow, unsigned int skipCol) const {
  vec3 cols[3];
  int colIndex = 0;

  for (int col = 0; col < 4; ++col) {
    if (col == skipCol) continue;

    int rowIndex = 0;
    float vals[3];

    for (int row = 0; row < 4; ++row) {
      if (row == skipRow) continue;
      vals[rowIndex++] = (*this)[col][row];
    }

    cols[colIndex++] = vec3(vals[0], vals[1], vals[2]);
  }

  return mat3(cols[0], cols[1], cols[2]);
}

} // namespace groot