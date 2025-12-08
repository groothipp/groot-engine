#pragma once

#include "src/include/enums.hpp"

#include <shaderc/shaderc.hpp>

namespace groot {

class ShaderCompiler {
  shaderc::Compiler m_compiler;
  shaderc::CompileOptions m_opts;

  public:
    ShaderCompiler();
    ShaderCompiler(const ShaderCompiler&) = delete;
    ShaderCompiler(ShaderCompiler&&) = delete;

    ~ShaderCompiler() = default;

    ShaderCompiler& operator=(const ShaderCompiler&) = delete;
    ShaderCompiler& operator=(ShaderCompiler&&) = delete;

    std::vector<unsigned int> compileShader(ShaderType, const std::string&) const;

  private:
    std::string stringify(const std::string& path) const;
};

} // namespace groot