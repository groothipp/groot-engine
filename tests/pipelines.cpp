#include "include/groot.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace groot;

TEST_CASE( "pipeline creation" ) {
  Engine engine;

  std::string shaderPath = std::format("{}/shaders/shader.glsl", GROOT_TEST_DIR);

  RID buffer = engine.create_uniform_buffer(1024);
  RID set = engine.create_descriptor_set({ buffer });

  REQUIRE( buffer.is_valid() );
  REQUIRE( set.is_valid() );

  SECTION( "compute pipeline" ) {
    std::println("--- create compute pipeline ---");

    RID shader = engine.compile_shader(ShaderType::Compute, shaderPath);
    REQUIRE( shader.is_valid() );

    RID pipeline = engine.create_compute_pipeline(shader, set);
    CHECK( pipeline.is_valid());
  }

  SECTION( "graphics pipeline" ) {
    std::println("--- create graphics pipeline ---");

    RID vertexShader = engine.compile_shader(ShaderType::Vertex, shaderPath);
    RID fragmentShader = engine.compile_shader(ShaderType::Fragment, shaderPath);

    REQUIRE( vertexShader.is_valid() );
    REQUIRE( fragmentShader.is_valid() );

    RID pipeline = engine.create_graphics_pipeline(GraphicsPipelineShaders{
      .vertex                 = vertexShader,
      .fragment               = fragmentShader,
    }, set, GraphicsPipelineSettings{});

    CHECK( pipeline.is_valid() );
  }
}

TEST_CASE( "pipeline destruction" ) {
  std::println("--- pipeline destruction ---");

  Engine engine;

  RID buffer = engine.create_uniform_buffer(1024);
  RID set = engine.create_descriptor_set({ buffer });
  RID shader = engine.compile_shader(ShaderType::Compute, std::format("{}/shaders/shader.glsl", GROOT_TEST_DIR));
  RID pipeline = engine.create_compute_pipeline(shader, set);

  REQUIRE( buffer.is_valid() );
  REQUIRE( set.is_valid() );
  REQUIRE( shader.is_valid() );
  REQUIRE(pipeline.is_valid() );

  engine.destroy_pipeline(pipeline);

  CHECK_FALSE( pipeline.is_valid() );
}

TEST_CASE( "invalid pipeline operations" ) {
  Engine engine;

  std::string shaderPath = std::format("{}/shaders/shader.glsl", GROOT_TEST_DIR);
  RID vertex = engine.compile_shader(ShaderType::Vertex, shaderPath);
  RID fragment = engine.compile_shader(ShaderType::Fragment, shaderPath);
  RID compute = engine.compile_shader(ShaderType::Compute, shaderPath);
  RID buffer = engine.create_uniform_buffer(1024);
  RID set = engine.create_descriptor_set({ buffer });

  REQUIRE( vertex.is_valid() );
  REQUIRE( fragment.is_valid() );
  REQUIRE( compute.is_valid() );
  REQUIRE( buffer.is_valid() );
  REQUIRE( set.is_valid() );

  SECTION( "invalid shader RID" ) {
    std::println("--- create pipeline with invalid RID ---");

    RID rid;

    RID computePipeline = engine.create_compute_pipeline(rid, set);
    CHECK_FALSE( computePipeline.is_valid() );

    RID graphicsPipeline = engine.create_graphics_pipeline(GraphicsPipelineShaders{
      .vertex = rid,
      .fragment = fragment
    }, set, GraphicsPipelineSettings{});
    CHECK_FALSE( graphicsPipeline.is_valid() );

    graphicsPipeline = engine.create_graphics_pipeline(GraphicsPipelineShaders{
      .vertex = vertex,
      .fragment = rid
    }, set, GraphicsPipelineSettings{});
    CHECK_FALSE( graphicsPipeline.is_valid() );
  }

  SECTION( "non-shader RID") {
    std::println("--- create pipeline with non-shader RID ---");

    RID computePipeline = engine.create_compute_pipeline(buffer, set);
    CHECK_FALSE( computePipeline.is_valid() );

    RID graphicsPipeline = engine.create_graphics_pipeline(GraphicsPipelineShaders{
      .vertex = buffer,
      .fragment = fragment
    }, set, GraphicsPipelineSettings{});
    CHECK_FALSE( graphicsPipeline.is_valid() );

    graphicsPipeline = engine.create_graphics_pipeline(GraphicsPipelineShaders{
      .vertex = vertex,
      .fragment = buffer
    }, set, GraphicsPipelineSettings{});
    CHECK_FALSE( graphicsPipeline.is_valid() );
  }

  SECTION( "invalid descriptor set RID" ) {
    std::println("--- create pipeline with invalid descriptor set RID ---");

    RID rid;

    RID computePipeline = engine.create_compute_pipeline(compute, rid);
    CHECK_FALSE( computePipeline.is_valid() );

    RID graphicsPipeline = engine.create_graphics_pipeline(GraphicsPipelineShaders{
      .vertex = vertex,
      .fragment = fragment
    }, rid, GraphicsPipelineSettings{});
    CHECK_FALSE( graphicsPipeline.is_valid() );
  }

  SECTION( "non-descriptor-set RID") {
    std::println("--- create pipeline with non-descriptor-set RID ---\n");

    RID computePipeline = engine.create_compute_pipeline(compute, buffer);
    CHECK_FALSE( computePipeline.is_valid() );

    RID graphicsPipeline = engine.create_graphics_pipeline(GraphicsPipelineShaders{
      .vertex = vertex,
      .fragment = fragment
    }, buffer, GraphicsPipelineSettings{});
    CHECK_FALSE( graphicsPipeline.is_valid() );
  }

  SECTION( "destroy invalid RID" ) {
    std::println("--- destroy invalid pipeline RID ---");

    RID rid;
    engine.destroy_pipeline(rid);
    CHECK( true );
  }

  SECTION( "destroy non-pipeline RID" ) {
    std::println("--- destroy non-pipeline RID ---\n");

    engine.destroy_pipeline(buffer);
    CHECK( buffer.is_valid() );
  }
}