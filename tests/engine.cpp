#include "include/groot.hpp"

#include <catch2/catch_test_macros.hpp>

#include <print>

TEST_CASE("engine") {
  bool success = true;

  std::vector<int> testBuffer;
  for (unsigned int i = 0; i < 1000; ++i)
    testBuffer.emplace_back(i);

  std::size_t size = sizeof(int) * testBuffer.size();

  try {
    groot::Engine engine;
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
    std::print("[Groot Engine] Engine Test Failure: {}\n", e.what());
    success = false;
  }

  CHECK( success );
}