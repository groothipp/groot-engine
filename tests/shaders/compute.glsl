#version 450

layout(binding = 0) buffer test_buffer {
  int _Nums[];
};

layout(push_constant) uniform push_constants {
  int _Num;
};

layout(local_size_x = 8, local_size_y = 1, local_size_z = 1) in;

void main() {
  uint index = gl_GlobalInvocationID.x;
  _Nums[index] = _Num;
}