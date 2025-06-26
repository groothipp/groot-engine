#version 460

layout(set = 0, binding = 1) uniform sampler2D ge_Textures[];

layout(set = 0, binding = 2) readonly buffer immutable_tint {
  vec3 tint;
};

layout(set = 0, binding = 3) readonly buffer mutable_color {
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
  frag_color = vec4(color * tint, 1.0) * texture(ge_Textures[0], uv);
}