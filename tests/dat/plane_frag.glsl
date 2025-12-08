#version 450

layout(binding = 1) uniform sampler2D _Texture;

layout(location = 0) in vec2 _UV;

layout(location = 0) out vec4 _FragColor;

void main() {
  _FragColor = texture(_Texture, _UV);
}