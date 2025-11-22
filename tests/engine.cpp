#include "include/groot.hpp"

#include <catch2/catch_test_macros.hpp>

#include <print>

TEST_CASE("engine") {
  bool success = true;

  try {
    groot::Engine engine;

    groot::RID storage_buffer = engine.create_storage_buffer(1000);
    groot::RID uniform_buffer = engine.create_uniform_buffer(1000);

    engine.run();
  }
  catch (const std::exception& e) {
    std::print("[Groot Engine] Engine Test Failure: {}\n", e.what());
    success = false;
  }

  CHECK( success );
}