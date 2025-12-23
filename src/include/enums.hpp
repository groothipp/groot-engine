#pragma once


namespace groot {

enum class ShaderType {
  Vertex,
  Fragment,
  TesselationControl,
  TesselationEvaluation,
  Compute
};

enum ResourceType {
  Invalid,
  Shader,
  Pipeline,
  DescriptorSet,
  Mesh,
  UniformBuffer,
  StorageBuffer,
  Sampler,
  StorageImage,
  StorageTexture,
  Texture
};

enum class Format {
  undefined           = 0,
  r8_unorm            = 9,
  r8_snorm            = 10,
  r8_uint             = 13,
  r8_sint             = 14,
  rg8_unorm           = 16,
  rg8_snorm           = 17,
  rg8_uint            = 20,
  rg8_sint            = 21,
  rgba8_unorm         = 37,
  rgba8_snorm         = 38,
  rgba8_uint          = 41,
  rgba8_sint          = 42,
  rgba8_srgb          = 43,
  bgra8_unorm         = 44,
  bgra8_srgb          = 50,
  r16_unorm           = 70,
  r16_snorm           = 71,
  r16_uint            = 74,
  r16_sint            = 75,
  r16_sfloat          = 76,
  rg16_unorm          = 77,
  rg16_snorm          = 78,
  rg16_uint           = 81,
  rg16_sint           = 82,
  rg16_sfloat         = 83,
  rgba16_unorm        = 91,
  rgba16_snorm        = 92,
  rgba16_uint         = 95,
  rgba16_sint         = 96,
  rgba16_sfloat       = 97,
  r32_uint            = 98,
  r32_sint            = 99,
  r32_sfloat          = 100,
  rg32_uint           = 101,
  rg32_sint           = 102,
  rg32_sfloat         = 103,
  rgb32_uint          = 104,
  rgb32_sint          = 105,
  rgb32_sfloat        = 106,
  rgba32_uint         = 107,
  rgba32_sint         = 108,
  rgba32_sfloat       = 109,
  bc1_rgb_unorm       = 131,
  bc1_rgb_srgb        = 132,
  bc1_rgba_unorm      = 133,
  bc1_rgba_srgb       = 134,
  bc2_unorm           = 135,
  bc2_srgb            = 136,
  bc3_unorm           = 137,
  bc3_srgb            = 138,
  bc4_unorm           = 139,
  bc4_snorm           = 140,
  bc5_unorm           = 141,
  bc5_snorm           = 142,
  bc6h_ufloat         = 143,
  bc6h_sfloat         = 144,
  bc7_unorm           = 145,
  bc7_srgb            = 146
};

enum InputRate {
  VertexRate,
  InstanceRate
};

enum MeshType {
  Solid,
  Wireframe,
  Vertices
};

enum CullMode {
  None,
  Front,
  Back
};

enum DrawDirection {
  CounterClockwise,
  Clockwise
};

enum SampleMode {
  Repeat,
  MirroredRepeat,
  ClampToEdge,
  ClampToBorder
};

enum Filter {
  Nearest,
  Linear
};

enum ColorSpace {
  srgb_nonlinear        = 0,
  display_p3_nonlinear  = 1000104001,
  extended_srgb_linear  = 1000104002
};

enum RenderMode {
  NoSync,
  TripleBuffer,
  VSync
};

enum class MouseButton {
  Left      = 0,
  Right     = 1,
  Middle    = 2,
  Button4   = 3,
  Button5   = 4,
  Button6   = 5,
  Button7   = 6,
  Button8   = 7
};

enum class Key {
  Space         = 32,
  Apostrophe    = 39,
  Comma         = 44,
  Minus         = 45,
  Period        = 46,
  Slash         = 47,
  Num0          = 48,
  Num1          = 49,
  Num2          = 50,
  Num3          = 51,
  Num4          = 52,
  Num5          = 53,
  Num6          = 54,
  Num7          = 55,
  Num8          = 56,
  Num9          = 57,
  Semicolon     = 59,
  Equal         = 61,
  A             = 65,
  B             = 66,
  C             = 67,
  D             = 68,
  E             = 69,
  F             = 70,
  G             = 71,
  H             = 72,
  I             = 73,
  J             = 74,
  K             = 75,
  L             = 76,
  M             = 77,
  N             = 78,
  O             = 79,
  P             = 80,
  Q             = 81,
  R             = 82,
  S             = 83,
  T             = 84,
  U             = 85,
  V             = 86,
  W             = 87,
  X             = 88,
  Y             = 89,
  Z             = 90,
  LeftBracket   = 91,
  Backslash     = 92,
  RightBracket  = 93,
  GraveAccent   = 96,
  Escape        = 256,
  Enter         = 257,
  Tab           = 258,
  Backspace     = 259,
  Insert        = 260,
  Delete        = 261,
  Right         = 262,
  Left          = 263,
  Down          = 264,
  Up            = 265,
  PageUp        = 266,
  PageDown      = 267,
  Home          = 268,
  End           = 269,
  CapsLock      = 280,
  ScrollLock    = 281,
  NumLock       = 282,
  PrintScreen   = 283,
  Pause         = 284,
  F1            = 290,
  F2            = 291,
  F3            = 292,
  F4            = 293,
  F5            = 294,
  F6            = 295,
  F7            = 296,
  F8            = 297,
  F9            = 298,
  F10           = 299,
  F11           = 300,
  F12           = 301,
  F13           = 302,
  F14           = 303,
  F15           = 304,
  F16           = 305,
  F17           = 306,
  F18           = 307,
  F19           = 308,
  F20           = 309,
  F21           = 310,
  F22           = 311,
  F23           = 312,
  F24           = 313,
  F25           = 314,
  Kp0           = 320,
  Kp1           = 321,
  Kp2           = 322,
  Kp3           = 323,
  Kp4           = 324,
  Kp5           = 325,
  Kp6           = 326,
  Kp7           = 327,
  Kp8           = 328,
  Kp9           = 329,
  KpDecimal     = 330,
  KpDivide      = 331,
  KpMultiply    = 332,
  KpSubtract    = 333,
  KpAdd         = 334,
  KpEnter       = 335,
  KpEqual       = 336,
  LeftShift     = 340,
  LeftControl   = 341,
  LeftAlt       = 342,
  LeftSuper     = 343,
  RightShift    = 344,
  RightControl  = 345,
  RightAlt      = 346,
  RightSuper    = 347,
  Menu          = 348
};

enum class ImageType {
  one_dim,
  two_dim,
  three_dim
};

} // namespace groot