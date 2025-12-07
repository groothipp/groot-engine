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

  RID cube_vert = engine.compile_shader(ShaderType::Vertex, std::format("{}/dat/cube_vert.glsl", GROOT_TEST_DIR));
  RID cube_frag = engine.compile_shader(ShaderType::Fragment, std::format("{}/dat/cube_frag.glsl", GROOT_TEST_DIR));
  RID plane_vert = engine.compile_shader(ShaderType::Vertex, std::format("{}/dat/plane_vert.glsl", GROOT_TEST_DIR));
  RID plane_frag = engine.compile_shader(ShaderType::Fragment, std::format("{}/dat/plane_frag.glsl", GROOT_TEST_DIR));
  REQUIRE( cube_vert.is_valid() );
  REQUIRE( cube_frag.is_valid() );
  REQUIRE( plane_vert.is_valid() );
  REQUIRE( plane_frag.is_valid() );

  RID sampler = engine.create_sampler(SamplerSettings{
    .anisotropic_filtering = false
  });
  REQUIRE( sampler.is_valid() );

  RID cloud_texture = engine.create_texture(std::format("{}/dat/test.png", GROOT_TEST_DIR), sampler);
  REQUIRE( cloud_texture.is_valid() );

  RID cube_buffer = engine.create_storage_buffer(sizeof(TransformBuffer));
  REQUIRE( cube_buffer.is_valid() );

  RID plane_buffer = engine.create_storage_buffer(sizeof(TransformBuffer));
  REQUIRE( plane_buffer.is_valid() );

  RID cube_set = engine.create_descriptor_set({ cube_buffer });
  REQUIRE( cube_set.is_valid() );

  RID plane_set = engine.create_descriptor_set({ plane_buffer, cloud_texture });
  REQUIRE( plane_set.is_valid() );

  RID cube_pipeline_back = engine.create_graphics_pipeline(GraphicsPipelineShaders{
    .vertex   = cube_vert,
    .fragment = cube_frag
  }, cube_set, GraphicsPipelineSettings{
    .cull_mode = CullMode::Front,
    .enable_depth_write = false
  });

  RID cube_pipeline_front = engine.create_graphics_pipeline(GraphicsPipelineShaders{
    .vertex   = cube_vert,
    .fragment = cube_frag
  }, cube_set, GraphicsPipelineSettings{
    .cull_mode          = CullMode::Back,
    .enable_depth_write = false
  });

  RID plane_pipeline = engine.create_graphics_pipeline(GraphicsPipelineShaders{
    .vertex = plane_vert,
    .fragment = plane_frag
  }, plane_set, GraphicsPipelineSettings{
    .cull_mode = CullMode::None
  });

  REQUIRE( cube_pipeline_back.is_valid() );
  REQUIRE( cube_pipeline_front.is_valid() );
  REQUIRE( plane_pipeline.is_valid() );

  RID cube_mesh = engine.load_mesh(std::format("{}/dat/cube.obj", GROOT_TEST_DIR));
  REQUIRE( cube_mesh.is_valid() );

  RID plane_mesh = engine.load_mesh(std::format("{}/dat/plane.obj", GROOT_TEST_DIR));
  REQUIRE( plane_mesh.is_valid() );

  Object cube_back;
  cube_back.set_mesh(cube_mesh);
  cube_back.set_pipeline(cube_pipeline_back);
  cube_back.set_descriptor_set(cube_set);

  Object cube_front;
  cube_front.set_mesh(cube_mesh);
  cube_front.set_pipeline(cube_pipeline_front);
  cube_front.set_descriptor_set(cube_set);

  Object plane;
  plane.set_descriptor_set(plane_set);
  plane.set_mesh(plane_mesh);
  plane.set_pipeline(plane_pipeline);

  engine.add_to_scene(cube_back);
  engine.add_to_scene(cube_front);
  engine.add_to_scene(plane);

  Transform transform;
  TransformBuffer transform_buffer{
    .model  = transform.matrix(),
    .view   = engine.camera_view(),
    .proj   = engine.camera_projection(),
    .norm   = transform.matrix().inverse().transpose()
  };

  auto [width, height] = engine.viewport_dims();

  Transform plane_transform{
    .position = vec3(0.0, 0.0, -2.0),
    .scale    = vec3(5.0, 2.8, 1.0)
  };
  TransformBuffer plane_transform_buffer{
    .model  = plane_transform.matrix(),
    .view   = engine.camera_view(),
    .proj   = engine.camera_projection(),
    .norm   = mat4::identity()
  };

  engine.write_buffer(cube_buffer, transform_buffer);
  engine.write_buffer(plane_buffer, plane_transform_buffer);

  engine.run([&engine, &cube_buffer, &transform_buffer, &transform](double dt) {
    transform.rotation.y += radians(5.0f) * dt;
    transform_buffer.model = transform.matrix();
    transform_buffer.norm = transform.matrix().inverse().transpose();

    engine.write_buffer(cube_buffer, transform_buffer);
  });
}