#include "include/groot/groot.hpp"

#include <catch2/catch_test_macros.hpp>

#include <iostream>

using namespace groot;

TEST_CASE( "compute dispatch" ) {
  std::println(std::cout, "--- compute dispatch ---");

  Engine engine;

  RID buffer = engine.create_storage_buffer(256 * sizeof(int));
  REQUIRE( buffer.is_valid() );

  RID set = engine.create_descriptor_set({ buffer });
  REQUIRE( set.is_valid() );

  RID shader = engine.compile_shader(ShaderType::Compute, std::format("{}/dat/compute.glsl", GROOT_TEST_DIR));
  REQUIRE( shader.is_valid() );

  RID pipeline = engine.create_compute_pipeline(shader, set);
  REQUIRE( pipeline.is_valid() );

  ComputeCommand cmd1{
    .pipeline       = pipeline,
    .descriptor_set = set,
    .push_constants = { 15, 0, 0, 0 },
    .work_groups    = { 32, 1, 1 },
    .barrier        = true
  };

  ComputeCommand cmd2{
    .pipeline       = pipeline,
    .descriptor_set = set,
    .push_constants = { 12, 0, 0, 0 },
    .work_groups    = { 32, 1, 1 },
    .barrier        = true
  };

  ComputeCommand cmd3{
    .pipeline       = pipeline,
    .descriptor_set = set,
    .push_constants = { 8, 0, 0, 0 },
    .work_groups    = { 32, 1, 1 },
  };

  engine.run([&engine, &cmd1, &cmd2, &cmd3](double){
    engine.dispatch(cmd2);
    engine.dispatch(cmd1);
    engine.dispatch(cmd3);
    engine.close_window();
  });

  std::vector<int> nums = engine.read_buffer<int>(buffer);

  std::vector<int> result;
  for (int i = 0; i < 256; ++i)
    result.emplace_back(8);

  CHECK( nums == result );
}

TEST_CASE( "invalid dispatch operations" ) {
  Engine engine;

  SECTION( "invalid pipeline RID" ) {
    std::println(std::cout, "--- invalid pipeline RID dispatch ---");

    RID buffer = engine.create_storage_buffer(1024);
    REQUIRE( buffer.is_valid() );

    RID set = engine.create_descriptor_set({ buffer });
    REQUIRE( set.is_valid() );

    RID pipeline;

    engine.run([&engine, &pipeline, &set](double){
      engine.dispatch(ComputeCommand{
        .pipeline = pipeline,
        .descriptor_set = set
      });
      engine.close_window();
    });

    CHECK( true );
  }

  SECTION( "non-pipeline RID" ) {
    std::println(std::cout, "--- non-pipeline RID dispatch ---");

    RID buffer = engine.create_storage_buffer(1024);
    REQUIRE( buffer.is_valid() );

    RID set = engine.create_descriptor_set({ buffer });
    REQUIRE( set.is_valid() );

    engine.run([&engine, &buffer, &set](double){
      engine.dispatch(ComputeCommand{
        .pipeline = buffer,
        .descriptor_set = set
      });
      engine.close_window();
    });

    CHECK( true );
  }

  SECTION( "invalid set RID" ) {
    std::println(std::cout, "--- invalid descriptor set RID dispatch ---");

    RID shader = engine.compile_shader(ShaderType::Compute, std::format("{}/dat/shader.glsl", GROOT_TEST_DIR));
    REQUIRE( shader.is_valid() );

    RID set = engine.create_descriptor_set({});
    REQUIRE( set.is_valid() );

    RID pipeline = engine.create_compute_pipeline(shader, set);
    REQUIRE( pipeline.is_valid() );

    RID rid;

    engine.run([&engine, &pipeline, &rid](double){
      engine.dispatch(ComputeCommand{
        .pipeline = pipeline,
        .descriptor_set = rid
      });
      engine.close_window();
    });

    CHECK( true );
  }

  SECTION( "non-descriptor-set RID" ) {
    std::println(std::cout, "--- non-descriptor-set RID dispatch ---");

    RID shader = engine.compile_shader(ShaderType::Compute, std::format("{}/dat/shader.glsl", GROOT_TEST_DIR));
    REQUIRE( shader.is_valid() );

    RID set = engine.create_descriptor_set({});
    REQUIRE( set.is_valid() );

    RID pipeline = engine.create_compute_pipeline(shader, set);
    REQUIRE( pipeline.is_valid() );

    engine.run([&engine, &pipeline, &shader](double){
      engine.dispatch(ComputeCommand{
        .pipeline = pipeline,
        .descriptor_set = shader
      });
      engine.close_window();
    });

    CHECK( true );
  }
}