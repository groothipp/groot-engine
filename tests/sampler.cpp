#include "include/groot.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace groot;

TEST_CASE( "create sampler" ) {
  Engine engine;

  RID sampler = engine.create_sampler({});
  CHECK( sampler.is_valid() );
}

TEST_CASE( "destroy sampler" ) {
  Engine engine;

  RID sampler = engine.create_sampler({});
  REQUIRE( sampler.is_valid() );

  engine.destroy_sampler( sampler );
  CHECK_FALSE( sampler.is_valid() );
}

TEST_CASE( "invalid sampler operations" ) {
  Engine engine;

  SECTION( "destroy invalid RID" ) {
    RID rid;
    engine.destroy_sampler(rid);
    CHECK( true );
  }

  SECTION( "destroy non-sampler RID" ) {
    RID buffer = engine.create_uniform_buffer(1024);
    REQUIRE( buffer.is_valid() );

    engine.destroy_sampler(buffer);
    CHECK( buffer.is_valid() );
  }

  SECTION( "destroy while in use" ) {
    RID sampler = engine.create_sampler({});
    REQUIRE( sampler.is_valid() );

    RID texture = engine.create_texture(std::format("{}/img/test.png", GROOT_TEST_DIR), sampler);
    REQUIRE( texture.is_valid() );

    engine.destroy_sampler(sampler);
    CHECK( sampler.is_valid() );
  }
}