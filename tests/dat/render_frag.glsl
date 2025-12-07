#version 450

layout(binding = 2) uniform sampler2D _Texture;

layout(location = 0) in vec2 _UV;
layout(location = 1) in vec3 _Normal;

layout(location = 0) out vec4 _FragColor;

#define LIGHT_DIR normalize(vec3(0.0, -1.0, 1.0))

void main() {
  vec4 base_color = texture(_Texture, _UV);
  float diffuse = max(dot(_Normal, LIGHT_DIR), 0.0);
  vec3 color = diffuse * base_color.rgb;

  _FragColor = vec4(color, base_color.a);
}