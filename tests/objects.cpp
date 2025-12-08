#include "include/groot/groot.hpp"

#include <catch2/catch_test_macros.hpp>

#include <iostream>

using namespace groot;

TEST_CASE( "add to scene" ) {
  std::println(std::cout, "--- add object to scene ---");

  Engine engine;

  std::string shader = std::format("{}/dat/shader.glsl", GROOT_TEST_DIR);
  RID vertShader = engine.compile_shader(ShaderType::Vertex, shader);
  RID fragShader = engine.compile_shader(ShaderType::Fragment, shader);
  REQUIRE( vertShader.is_valid() );
  REQUIRE( fragShader.is_valid() );

  RID set = engine.create_descriptor_set({});
  REQUIRE( set.is_valid() );

  RID pipeline = engine.create_graphics_pipeline(GraphicsPipelineShaders{
    .vertex = vertShader,
    .fragment = fragShader,
  }, set, {});
  REQUIRE( pipeline.is_valid() );

  RID mesh = engine.load_mesh(std::format("{}/dat/cube.obj", GROOT_TEST_DIR));
  REQUIRE( mesh.is_valid() );

  Object obj;
  obj.set_mesh(mesh);
  obj.set_descriptor_set(set);
  obj.set_pipeline(pipeline);

  engine.add_to_scene(obj);

  CHECK( obj.is_in_scene() );
}

TEST_CASE( "remove from scene" ) {
  std::println(std::cout, "--- remove object to scene ---");

  Engine engine;

  std::string shader = std::format("{}/dat/shader.glsl", GROOT_TEST_DIR);
  RID vertShader = engine.compile_shader(ShaderType::Vertex, shader);
  RID fragShader = engine.compile_shader(ShaderType::Fragment, shader);
  REQUIRE( vertShader.is_valid() );
  REQUIRE( fragShader.is_valid() );

  RID set = engine.create_descriptor_set({});
  REQUIRE( set.is_valid() );

  RID pipeline = engine.create_graphics_pipeline(GraphicsPipelineShaders{
    .vertex = vertShader,
    .fragment = fragShader,
  }, set, {});
  REQUIRE( pipeline.is_valid() );

  RID mesh = engine.load_mesh(std::format("{}/dat/cube.obj", GROOT_TEST_DIR));
  REQUIRE( mesh.is_valid() );

  Object obj;
  obj.set_mesh(mesh);
  obj.set_descriptor_set(set);
  obj.set_pipeline(pipeline);

  engine.add_to_scene(obj);
  REQUIRE( obj.is_in_scene() );

  engine.remove_from_scene(obj);

  CHECK_FALSE( obj.is_in_scene() );
}