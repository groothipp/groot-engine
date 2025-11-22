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

    RID buffer = engine.create_uniform_buffer(sizeof(int) * testBuffer.size());
    RID shader = engine.compile_shader(ShaderType::Compute, std::format("{}/shaders/shader.comp", GROOT_TEST_DIR));

    engine.run([&engine, &testBuffer, &size](double t) {
      groot::RID uniform_buffer = engine.create_uniform_buffer(size);
      groot::RID storage_buffer = engine.create_storage_buffer(size);

      engine.update_buffer(uniform_buffer, size, testBuffer.data());
      engine.update_buffer(storage_buffer, size, testBuffer.data());

      engine.destroy_buffer(uniform_buffer);
      engine.destroy_buffer(storage_buffer);
    });
  }
  catch (const std::exception& e) {
    Log::warn(std::format("[Groot Engine] Engine Test Failure: {}\n", e.what()));
    success = false;
  }

  CHECK( success );
}