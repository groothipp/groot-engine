#version 450

layout(location = 0) in vec2 _UV;
layout(location = 1) in vec3 _Normal;
layout(location = 2) in vec3 _WorldPos;

layout(location = 0) out vec4 _FragColor;

#define LIGHT_DIR normalize(vec3(0.0, 0.0, 1.0))
#define MAX_ITER 64
#define C vec4(-0.2, 0.6, 0.2, -0.5)

// Quaternion square: q * q
vec4 qsq(vec4 q) {
  return vec4(
    2.0 * q.w * q.x,
    2.0 * q.w * q.y,
    2.0 * q.w * q.z,
    q.w * q.w - q.x * q.x - q.y * q.y - q.z * q.z
  );
}

vec3 palette(float t) {
  vec3 a = vec3(0.5, 0.5, 0.5);
  vec3 b = vec3(0.5, 0.5, 0.5);
  vec3 c = vec3(1.0, 1.0, 1.0);
  vec3 d = vec3(0.0, 0.33, 0.67);
  return a + b * cos(6.28318 * (c * t + d));
}

void main() {
  // Use world position directly for continuous wrapping across faces
  // Try smaller scale to zoom into the fractal details
  vec3 scaledPos = _WorldPos * 0.5;
  vec4 q = vec4(scaledPos, 0.0);

  int i;
  for (i = 0; i < MAX_ITER; i++) {
    q = qsq(q) + C;
    if (dot(q, q) > 4.0) break;
  }

  if (i == MAX_ITER) {
    // Point is IN the set - render it with color and lighting
    float t = length(_WorldPos) * 0.5;  // Color based on distance from origin
    vec3 fractal_color = palette(t);

    // Apply lighting
    float diffuse = max(dot(_Normal, LIGHT_DIR), 0.3);
    vec3 color = diffuse * fractal_color;

    _FragColor = vec4(color, 1.0);
  } else {
    // Point is OUTSIDE the set - make it transparent
    discard;
  }
}