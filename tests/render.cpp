#include "include/groot.hpp"

#include <catch2/catch_test_macros.hpp>

// #include <numbers>

using namespace groot;

struct TransformBuffer {
  mat4 model = mat4::identity();
  mat4 view = mat4::identity();
  mat4 proj = mat4::identity();
  mat4 norm = mat4::identity();
};

TEST_CASE( "render", "[render]" ) {
  Engine engine(Settings{
    .application_name = "Render Test",
    .window_title     = "Groot Engine Render Test",
  });

  RID vert = engine.compile_shader(ShaderType::Vertex, std::format("{}/dat/render_vert.glsl", GROOT_TEST_DIR));
  RID frag = engine.compile_shader(ShaderType::Fragment, std::format("{}/dat/render_frag.glsl", GROOT_TEST_DIR));
  REQUIRE( vert.is_valid() );
  REQUIRE( frag.is_valid() );

  RID buffer = engine.create_storage_buffer(sizeof(TransformBuffer));
  REQUIRE( buffer.is_valid() );

  RID set = engine.create_descriptor_set({ buffer });
  REQUIRE( set.is_valid() );

  RID graphics_pipeline = engine.create_graphics_pipeline(GraphicsPipelineShaders{
    .vertex   = vert,
    .fragment = frag
  }, set, GraphicsPipelineSettings{
    .draw_direction = DrawDirection::Clockwise,
    .enable_depth = false,
    .enable_blend = true
  });
  REQUIRE( graphics_pipeline.is_valid() );

  RID mesh = engine.load_mesh(std::format("{}/dat/cube.obj", GROOT_TEST_DIR));
  REQUIRE( mesh.is_valid() );

  Object cube;
  cube.set_mesh(mesh);
  cube.set_pipeline(graphics_pipeline);
  cube.set_descriptor_set(set);

  engine.add_to_scene(cube);

  Transform transform;
  TransformBuffer transform_buffer{
    .model = transform.matrix(),
    .view = engine.camera_view(),
    .proj = engine.camera_projection(),
    .norm = transform.matrix().inverse().transpose()
  };

  engine.write_buffer(buffer, transform_buffer);

  engine.run([&engine, &set, &buffer, &transform_buffer, &transform](double dt) {
    transform.rotation.y += radians(5.0f) * dt;
    transform_buffer.model = transform.matrix();
    transform_buffer.norm = transform.matrix().inverse().transpose();

    engine.write_buffer(buffer, transform_buffer);
  });
}