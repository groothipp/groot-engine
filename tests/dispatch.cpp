#include "include/groot.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace groot;

TEST_CASE( "compute dispatch" ) {
  std::print("--- compute dispatch ---");

  Engine engine;

  RID buffer = engine.create_storage_buffer(256 * sizeof(int));
  REQUIRE( buffer.is_valid() );

  RID set = engine.create_descriptor_set({ buffer });
  REQUIRE( set.is_valid() );

  RID shader = engine.compile_shader(ShaderType::Compute, std::format("{}/shaders/compute.glsl", GROOT_TEST_DIR));
  REQUIRE( shader.is_valid() );

  RID pipeline = engine.create_compute_pipeline(shader, set);
  REQUIRE( pipeline.is_valid() );

  ComputeCommand cmd1{
    .pipeline       = pipeline,
    .descriptor_set = set,
    .push_constants = { 15, 0, 0, 0 },
    .work_groups    = { 32, 1, 1 }
  };

  ComputeCommand cmd2{
    .pipeline       = pipeline,
    .descriptor_set = set,
    .push_constants = { 12, 0, 0, 0 },
    .barrier        = true,
    .work_groups    = { 32, 1, 1 }
  };

  ComputeCommand cmd3{
    .pipeline       = pipeline,
    .descriptor_set = set,
    .push_constants = { 8, 0, 0, 0 },
    .barrier        = true,
    .work_groups    = { 32, 1, 1 }
  };

  engine.compute_command(cmd1);
  engine.compute_command(cmd2);
  engine.compute_command(cmd3);
  engine.dispatch();

  std::vector<int> nums = engine.read_buffer<int>(buffer);

  std::vector<int> result;
  for (int i = 0; i < 256; ++i)
    result.emplace_back(8);

  CHECK( nums == result );
}

TEST_CASE( "invalid dispatch operations" ) {
  std::print("--- invalid dispatch operations ---");

  Engine engine;

  SECTION( "invalid pipeline RID" ) {
    RID buffer = engine.create_storage_buffer(1024);
    REQUIRE( buffer.is_valid() );

    RID set = engine.create_descriptor_set({ buffer });
    REQUIRE( set.is_valid() );

    RID pipeline;
    engine.compute_command(ComputeCommand{
      .pipeline = pipeline,
      .descriptor_set = set
    });

    engine.dispatch();

    CHECK( true );
  }

  SECTION( "non-pipeline RID" ) {
    RID buffer = engine.create_storage_buffer(1024);
    REQUIRE( buffer.is_valid() );

    RID set = engine.create_descriptor_set({ buffer });
    REQUIRE( set.is_valid() );

    engine.compute_command(ComputeCommand{
      .pipeline = buffer,
      .descriptor_set = set
    });

    engine.dispatch();

    CHECK( true );
  }

  SECTION( "invalid set RID" ) {
    RID shader = engine.compile_shader(ShaderType::Compute, std::format("{}/shaders/shader.glsl", GROOT_TEST_DIR));
    REQUIRE( shader.is_valid() );

    RID set = engine.create_descriptor_set({});
    REQUIRE( set.is_valid() );

    RID pipeline = engine.create_compute_pipeline(shader, set);
    REQUIRE( pipeline.is_valid() );

    RID rid;
    engine.compute_command(ComputeCommand{
      .pipeline = pipeline,
      .descriptor_set = rid
    });

    engine.dispatch();

    CHECK( true );
  }

  SECTION( "non-descriptor-set RID" ) {
    RID shader = engine.compile_shader(ShaderType::Compute, std::format("{}/shaders/shader.glsl", GROOT_TEST_DIR));
    REQUIRE( shader.is_valid() );

    RID set = engine.create_descriptor_set({});
    REQUIRE( set.is_valid() );

    RID pipeline = engine.create_compute_pipeline(shader, set);
    REQUIRE( pipeline.is_valid() );

    engine.compute_command(ComputeCommand{
      .pipeline = pipeline,
      .descriptor_set = shader
    });

    engine.dispatch();

    CHECK( true );
  }
}