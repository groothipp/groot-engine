#include "include/groot.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace groot;

TEST_CASE( "buffer creation" ) {
  Engine engine;

  SECTION( "uniform buffer" ) {
    RID buffer = engine.create_uniform_buffer(1024);
    CHECK( buffer.is_valid() );
  }

  SECTION( "storage buffer" ) {
    RID buffer = engine.create_storage_buffer(1024);
    CHECK( buffer.is_valid() );
  }
}

TEST_CASE( "buffer destruction" ) {
  Engine engine;

  RID buffer = engine.create_uniform_buffer(1024);
  REQUIRE( buffer.is_valid() );

  engine.destroy_buffer(buffer);

  CHECK_FALSE( buffer.is_valid() );
}

TEST_CASE( "invalid buffer operations" ) {
  Engine engine;

  SECTION( "invalid RID" ) {
    RID rid;
    engine.destroy_buffer(rid);
    CHECK( true );
  }

  SECTION( "size 0 creation" ) {
    RID uniform = engine.create_uniform_buffer(0);
    CHECK_FALSE( uniform.is_valid() );

    RID storage = engine.create_storage_buffer(0);
    CHECK_FALSE( storage.is_valid() );
  }

  SECTION( "destroy non-buffer RID" ) {
    RID image = engine.create_storage_image(1024, 1024, Format::rgba8_unorm);
    REQUIRE( image.is_valid() );

    engine.destroy_buffer(image);
    CHECK( image.is_valid() );
  }
}