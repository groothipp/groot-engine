#include "include/groot.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace groot;

TEST_CASE("engine") {
  bool success = true;

  std::vector<int> testBuffer;
  for (unsigned int i = 0; i < 1000; ++i)
    testBuffer.emplace_back(i);

  std::size_t size = sizeof(int) * testBuffer.size();

  try {
    Engine engine;

    std::string shaderPath = std::format("{}/shaders/shader.glsl", GROOT_TEST_DIR);

    RID vertex = engine.compile_shader(ShaderType::Vertex, shaderPath);
    RID fragment = engine.compile_shader(ShaderType::Fragment, shaderPath);
    RID compute = engine.compile_shader(ShaderType::Compute, shaderPath);

    RID buffer1 = engine.create_uniform_buffer(size);
    RID buffer2 = engine.create_storage_buffer(size);
    RID set = engine.create_descriptor_set({ buffer1, buffer2 });
    RID pipeline = engine.create_compute_pipeline(compute, set);

    engine.run([&engine, &testBuffer, &size, &vertex, &fragment](double t) {
      RID uniformBuffer = engine.create_uniform_buffer(size);
      RID storageBuffer = engine.create_storage_buffer(size);
      RID descriptorSet = engine.create_descriptor_set({ uniformBuffer, storageBuffer });
      RID graphicsPipeline = engine.create_graphics_pipeline(GraphicsPipelineShaders{
        .vertex = vertex,
        .fragment = fragment
      }, descriptorSet, GraphicsPipelineSettings{});

      engine.update_buffer(uniformBuffer, size, testBuffer.data());
      engine.update_buffer(storageBuffer, size, testBuffer.data());

      engine.destroy_pipeline(graphicsPipeline);
      engine.destroy_descriptor_set(descriptorSet);
      engine.destroy_buffer(uniformBuffer);
      engine.destroy_buffer(storageBuffer);
    });
  }
  catch (const std::exception& e) {
    Log::warn(e.what());
    success = false;
  }

  CHECK( success );
}