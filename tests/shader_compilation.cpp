#include "include/groot.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace groot;

TEST_CASE( "compile shader" ) {
  std::println("--- compile shader ---");

  Engine engine;

  RID shader = engine.compile_shader(ShaderType::Vertex, std::format("{}/shaders/shader.glsl", GROOT_TEST_DIR));
  CHECK( shader.is_valid() );
}

TEST_CASE( "destroy shader" ) {
  std::println("--- destroy shader ---");

  Engine engine;

  RID shader = engine.compile_shader(ShaderType::Vertex, std::format("{}/shaders/shader.glsl", GROOT_TEST_DIR));
  REQUIRE( shader.is_valid() );

  engine.destroy_shader(shader);
  CHECK_FALSE( shader.is_valid() );
}

TEST_CASE( "invalid shader path" ) {
  std::println("--- invalid shader path ---");

  Engine engine;

  RID shader = engine.compile_shader(ShaderType::Vertex, "");
  CHECK_FALSE( shader.is_valid() );
}