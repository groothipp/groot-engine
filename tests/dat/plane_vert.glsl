#version 450

layout(binding = 0) readonly buffer transform {
  mat4 _Model;
  mat4 _View;
  mat4 _Proj;
};

layout(location = 0) in vec3 _VertexPosition;
layout(location = 1) in vec2 _VertexUV;
layout(location = 2) in vec3 _VertexNormal;

layout(location = 0) out vec2 _UV;

void main() {
  gl_Position = _Proj * _View * _Model * vec4(_VertexPosition, 1.0);
  _UV = _VertexUV;
}