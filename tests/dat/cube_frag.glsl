#version 450

layout(location = 0) in vec3 _Normal;
layout(location = 1) in vec3 _WorldPos;
layout(location = 2) in vec3 _ViewDir;

layout(location = 0) out vec4 _FragColor;

#define IOR 1.6
#define TINT vec3(1.0)
#define OPACITY 0.01
#define SPECULAR_STRENGTH 25.0
#define LIGHT_DIR normalize(vec3(0.0, 1.0, 1.0))

float fresnel(vec3 view_dir, vec3 normal, float ior) {
  float f0 = pow((1.0 - ior) / (1.0 + ior), 2.0);
  float c = abs(dot(view_dir, normal));
  return f0 + (1.0 - f0) * pow(1.0 - c, 5.0);
}

void main() {
  vec3 h = normalize(_ViewDir + LIGHT_DIR);
  float s = pow(abs(dot(_Normal, h)), SPECULAR_STRENGTH);
  float f = fresnel(_ViewDir, _Normal, IOR);

  vec3 color = TINT + vec3(s);
  float a = mix(OPACITY, 0.4, f);

  _FragColor = vec4(color, a);
}