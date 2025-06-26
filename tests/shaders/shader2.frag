#version 460

layout(set = 0, binding = 1) uniform sampler2D ge_Textures[];

layout(push_constant) uniform push_constants {
  layout(row_major) mat4 ge_View;
  layout(row_major) mat4 ge_Projection;
};

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec4 frag_color;

void main() {
  frag_color = texture(ge_Textures[0], uv);
}