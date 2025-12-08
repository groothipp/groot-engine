#include "include/groot/groot.hpp"

#include <catch2/catch_test_macros.hpp>

#include <iostream>

using namespace groot;

TEST_CASE( "create sampler" ) {
  std::println(std::cout, "--- create sampler ---");

  Engine engine;

  RID sampler = engine.create_sampler({});
  CHECK( sampler.is_valid() );
}

TEST_CASE( "destroy sampler" ) {
  std::println(std::cout, "--- destroy sampler ---");

  Engine engine;

  RID sampler = engine.create_sampler({});
  REQUIRE( sampler.is_valid() );

  engine.destroy_sampler( sampler );
  CHECK_FALSE( sampler.is_valid() );
}

TEST_CASE( "invalid sampler operations" ) {
  Engine engine;

  SECTION( "destroy invalid RID" ) {
    std::println(std::cout, "--- destroy invalid sampler RID ---");

    RID rid;
    engine.destroy_sampler(rid);
    CHECK( true );
  }

  SECTION( "destroy non-sampler RID" ) {
    std::println(std::cout, "--- destroy non-sampler RID ---");

    RID buffer = engine.create_uniform_buffer(1024);
    REQUIRE( buffer.is_valid() );

    engine.destroy_sampler(buffer);
    CHECK( buffer.is_valid() );
  }

  SECTION( "destroy while in use" ) {
    std::println(std::cout, "--- destroy busy sampler ---");

    RID sampler = engine.create_sampler({});
    REQUIRE( sampler.is_valid() );

    RID texture = engine.create_texture(std::format("{}/dat/test.png", GROOT_TEST_DIR), sampler);
    REQUIRE( texture.is_valid() );

    engine.destroy_sampler(sampler);
    CHECK( sampler.is_valid() );
  }
}