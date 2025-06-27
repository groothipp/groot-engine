#version 460

layout(set = 0, binding = 1) uniform sampler2D ge_Textures[];

layout(set = 0, binding = 3) uniform sampler2D ge_Canvases[];

layout(push_constant) uniform push_constants {
  layout(row_major) mat4 ge_View;
  layout(row_major) mat4 ge_Projection;
};

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec4 frag_color;

void main() {
  vec4 tex_color = texture(ge_Textures[0], uv);
  vec4 tint_color = texture(ge_Canvases[0], uv);

  frag_color = tex_color * tint_color;
}