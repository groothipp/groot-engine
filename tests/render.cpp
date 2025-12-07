#include "include/groot.hpp"

#include <catch2/catch_test_macros.hpp>

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
  RID comp = engine.compile_shader(ShaderType::Compute, std::format("{}/dat/render_comp.glsl", GROOT_TEST_DIR));
  REQUIRE( vert.is_valid() );
  REQUIRE( frag.is_valid() );
  REQUIRE( comp.is_valid() );

  RID sampler = engine.create_sampler(SamplerSettings{
    .anisotropic_filtering = false
  });
  REQUIRE( sampler.is_valid() );

  RID texture = engine.create_storage_texture(256, 256, sampler, Format::rgba8_unorm);
  REQUIRE( texture.is_valid() );

  RID buffer = engine.create_storage_buffer(sizeof(TransformBuffer));
  REQUIRE( buffer.is_valid() );

  RID set = engine.create_descriptor_set({ buffer, texture });
  REQUIRE( set.is_valid() );

  RID compute_pipeline = engine.create_compute_pipeline(comp, set);
  RID graphics_pipeline = engine.create_graphics_pipeline(GraphicsPipelineShaders{
    .vertex   = vert,
    .fragment = frag
  }, set, GraphicsPipelineSettings{
    .draw_direction = DrawDirection::Clockwise,
    .enable_depth = false,
    .enable_blend = true
  });
  REQUIRE( compute_pipeline.is_valid() );
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

  engine.run([&engine, &compute_pipeline, &set, &buffer, &transform_buffer, &transform](double dt) {
    engine.compute_command(ComputeCommand{
      .pipeline       = compute_pipeline,
      .descriptor_set = set,
      .work_groups    = { 32, 32, 1 }
    });
    engine.dispatch();

    transform.rotation.y += radians(5.0f) * dt;
    transform_buffer.model = transform.matrix();
    transform_buffer.norm = transform.matrix().inverse().transpose();

    engine.write_buffer(buffer, transform_buffer);
  });
}