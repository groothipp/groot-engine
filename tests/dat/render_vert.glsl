#version 450

layout(binding = 0) readonly buffer transform {
  mat4 _Model;
  mat4 _View;
  mat4 _Proj;
  mat4 _Norm;
};

layout(location = 0) in vec3 _VertexPosition;
layout(location = 1) in vec2 _VertexUV;
layout(location = 2) in vec3 _VertexNormal;

layout(location = 0) out vec2 _UV;
layout(location = 1) out vec3 _Normal;

void main() {
  gl_Position = _Proj * _View * _Model * vec4(_VertexPosition, 1.0f);
  _UV = _VertexUV;
  _Normal = normalize(mat3(_Norm) * _VertexNormal);
}