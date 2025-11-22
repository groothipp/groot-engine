#include "src/include/log.hpp"
#include "src/include/shader_compiler.hpp"

#include <fstream>
#include <format>

namespace groot {

ShaderCompiler::ShaderCompiler() {
  m_opts.SetOptimizationLevel(shaderc_optimization_level_performance);
  m_opts.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
}

std::vector<unsigned int> ShaderCompiler::compileShader(ShaderType type, const std::string& path) const {
  std::string source = stringify(path);
  if (source == "") return {};

  shaderc_shader_kind shaderKind;
  switch (type) {
    case ShaderType::Vertex:
      shaderKind = shaderc_vertex_shader;
      break;
    case ShaderType::Fragment:
      shaderKind = shaderc_fragment_shader;
      break;
    case ShaderType::TesselationControl:
      shaderKind = shaderc_tess_control_shader;
      break;
    case ShaderType::TesselationEvaluation:
      shaderKind = shaderc_tess_evaluation_shader;
      break;
    case ShaderType::Compute:
      shaderKind = shaderc_compute_shader;
  }

  auto res = m_compiler.CompileGlslToSpv(source, shaderKind, path.c_str());
  if (res.GetNumErrors() > 0) {
    Log::warn(std::format("\033[31mfailed to compile {}:\033[0m\n{}", path, res.GetErrorMessage()));
    return {};
  }
  if (res.GetNumWarnings() > 0)
    Log::warn(std::format("warning generated while compiling {}:\033[0m\n{}", path, res.GetErrorMessage()));

  return std::vector<unsigned int>(res.begin(), res.end());
}

std::string ShaderCompiler::stringify(const std::string& path) const {
  std::ifstream file(path);
  if (!file) {
    Log::warn(std::format("{} not found", path));
    return "";
  }

  std::string source = "";
  std::string line = "";
  while (std::getline(file, line))
    source += std::format("{}\n", line);

  return source;
}

} // namespace groot