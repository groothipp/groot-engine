#include "include/groot/groot.hpp"

#include <catch2/catch_test_macros.hpp>

#include <iostream>

using namespace groot;

TEST_CASE( "compile shader" ) {
  std::println(std::cout, "--- compile shader ---");

  Engine engine;

  RID shader = engine.compile_shader(ShaderType::Vertex, std::format("{}/dat/shader.glsl", GROOT_TEST_DIR));
  CHECK( shader.is_valid() );
}

TEST_CASE( "destroy shader" ) {
  std::println(std::cout, "--- destroy shader ---");

  Engine engine;

  RID shader = engine.compile_shader(ShaderType::Vertex, std::format("{}/dat/shader.glsl", GROOT_TEST_DIR));
  REQUIRE( shader.is_valid() );

  engine.destroy_shader(shader);
  CHECK_FALSE( shader.is_valid() );
}

TEST_CASE( "invalid shader path" ) {
  std::println(std::cout, "--- invalid shader path ---");

  Engine engine;

  RID shader = engine.compile_shader(ShaderType::Vertex, "");
  CHECK_FALSE( shader.is_valid() );
}