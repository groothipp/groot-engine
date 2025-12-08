#version 450

layout(binding = 1, rgba8) uniform writeonly image2D _Texture;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

void main() {
  uvec2 index = gl_GlobalInvocationID.xy;
  uvec2 size = imageSize(_Texture);
  if (!all(lessThan(index, size))) return;

  imageStore(_Texture, ivec2(index), vec4(0.0, 1.0, 0.0, 1.0));
}