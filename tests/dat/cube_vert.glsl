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

layout(location = 0) out vec3 _Normal;
layout(location = 1) out vec3 _WorldPos;
layout(location = 2) out vec3 _ViewDir;

void main() {
  vec4 world_pos = _Model * vec4(_VertexPosition, 1.0);
  gl_Position = _Proj * _View * world_pos;

  _Normal = normalize(mat3(_Norm) * _VertexNormal);
  _WorldPos = world_pos.xyz;
  _ViewDir = normalize(-(_View * world_pos).xyz);
}