#include "include/groot/groot.hpp"

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
  RID post_comp = engine.compile_shader(ShaderType::Compute, std::format("{}/dat/post.comp", GROOT_TEST_DIR));
  REQUIRE( cube_vert.is_valid() );
  REQUIRE( cube_frag.is_valid() );
  REQUIRE( plane_vert.is_valid() );
  REQUIRE( plane_frag.is_valid() );
  REQUIRE( post_comp.is_valid() );

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

  double velocity = 0.25;
  double sensitivity = 0.005;
  vec2 lastCursorPos;
  vec2 cursorPos;
  double inputTimer = 0.0;

  engine.add_gui("Test",
    GUI::Builder()
      .text("Hello world")
      .build()
  );

  engine.capture_cursor();

  engine.run([
    &engine, &cube_buffer, &transform_buffer, &transform, &inputTimer, &width, &height,
    &velocity, &sensitivity, &lastCursorPos, &cursorPos, &plane_transform_buffer, &plane_buffer
  ](double dt) {
    inputTimer += dt;
    if (inputTimer > 0.005) {
      inputTimer = 0.0;

      if (engine.just_pressed(Key::Escape))
        engine.close_window();

      if (engine.just_pressed(Key::LeftAlt))
        engine.release_cursor();
      if (engine.just_released(Key::LeftAlt))
        engine.capture_cursor();

      vec3 dir = vec3(0.0f);
      if (engine.is_pressed(Key::W)) dir.z += 1.0f;
      if (engine.is_pressed(Key::A)) dir.x -= 1.0f;
      if (engine.is_pressed(Key::S)) dir.z -= 1.0f;
      if (engine.is_pressed(Key::D)) dir.x += 1.0f;
      if (engine.is_pressed(Key::Space)) dir.y += 1.0f;
      if (engine.is_pressed(Key::LeftShift)) dir.y -= 1.0f;

      if (dir.mag() > 1e-6) {
        auto [forward, right, up] = engine.camera_basis();
        dir = (dir.z * forward + dir.x * right + dir.y * up).normalized();
        engine.translate_camera(velocity * dt * dir);
      }

      if (engine.is_pressed(MouseButton::Left)) {
        cursorPos = engine.mouse_pos();
        vec2 delta = cursorPos - lastCursorPos;

        float pitch = -delta.y * sensitivity;
        float yaw = -delta.x * sensitivity;

        engine.rotate_camera(pitch, yaw);
      }
    }
    lastCursorPos = engine.mouse_pos();

    transform.rotation.y += radians(5.0f) * dt;
    transform_buffer.model = transform.matrix();
    transform_buffer.norm = transform.matrix().inverse().transpose();
    transform_buffer.view = engine.camera_view();

    engine.write_buffer(cube_buffer, transform_buffer);

    plane_transform_buffer.view = engine.camera_view();
    engine.write_buffer(plane_buffer, plane_transform_buffer);
  },
  [&engine, &width, &height, &post_comp](double){
    RID post_set = engine.create_descriptor_set({ engine.render_target() });
    RID post_pipeline = engine.create_compute_pipeline(post_comp, post_set);

    engine.dispatch(ComputeCommand{
      .pipeline       = post_pipeline,
      .descriptor_set = post_set,
      .work_groups    = { (width + 7) / 8, (height + 7) / 8, 1 },
    });
  });
}