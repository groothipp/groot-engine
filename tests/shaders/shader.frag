#version 460

layout(set = 0, binding = 1) readonly buffer colors {
  vec3 color;
};

layout(push_constant) uniform push_constants {
  layout(row_major) mat4 ge_View;
  layout(row_major) mat4 ge_Projection;
};

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec4 frag_color;

void main() {
  frag_color = vec4(color, 1.0);
}