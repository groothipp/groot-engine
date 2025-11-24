#include "include/groot.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace groot;

TEST_CASE( "descriptor set creation" ) {
  Engine engine;

  SECTION( "uniform buffer" ) {
    RID buffer = engine.create_uniform_buffer(1024);
    REQUIRE( buffer.is_valid() );

    RID set = engine.create_descriptor_set({ buffer });

    CHECK( set.is_valid() );
  }

  SECTION( "storage buffer" ) {
    RID buffer = engine.create_storage_buffer(1024);
    REQUIRE( buffer.is_valid() );

    RID set = engine.create_descriptor_set({ buffer });

    CHECK( set.is_valid() );
  }

  SECTION ("storage image" ) {
    RID image = engine.create_storage_image(1024, 1024, Format::rgba8_unorm);
    REQUIRE( image.is_valid() );

    RID set = engine.create_descriptor_set({ image });

    CHECK( set.is_valid() );
  }

  SECTION( "all descriptor types" ) {
    RID uniformBuffer = engine.create_uniform_buffer(1024);
    RID storageBuffer = engine.create_storage_buffer(1024);
    RID image = engine.create_storage_image(1024, 1024, Format::rgba8_unorm);

    REQUIRE( uniformBuffer.is_valid() );
    REQUIRE( storageBuffer.is_valid() );
    REQUIRE( image.is_valid() );

    RID set = engine.create_descriptor_set({ uniformBuffer, storageBuffer, image });

    CHECK( set.is_valid() );
  }

  SECTION( "empty descriptor array" ) {
    RID set = engine.create_descriptor_set({});
    CHECK( set.is_valid() );
  }
}

TEST_CASE( "descriptor set destruction" ) {
  Engine engine;

  RID buffer = engine.create_uniform_buffer(1024);
  REQUIRE( buffer.is_valid() );

  RID set = engine.create_descriptor_set({ buffer });
  REQUIRE( set.is_valid() );

  engine.destroy_descriptor_set(set);

  CHECK_FALSE( set.is_valid() );
}

TEST_CASE( "invalid descriptor set operations" ) {
  Engine engine;

  SECTION( "destroy invalid RID" ) {
    RID rid;
    engine.destroy_descriptor_set(rid);
    CHECK( true );
  }

  SECTION( "destroy non-descriptor-set RID" ) {
    RID buffer = engine.create_uniform_buffer(1024);
    REQUIRE( buffer.is_valid() );

    engine.destroy_descriptor_set(buffer);
    CHECK( buffer.is_valid() );
  }
}