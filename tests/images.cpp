#include "include/groot.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace groot;

TEST_CASE( "image creation" ) {
  Engine engine;

  SECTION( "storage image" ) {
    RID image = engine.create_storage_image(1024, 1024, Format::rgba8_unorm);
    CHECK( image.is_valid() );
  }

  SECTION( "texture" ) {
    RID sampler = engine.create_sampler({});
    REQUIRE( sampler.is_valid() );

    RID texture = engine.create_texture(std::format("{}/img/test.png", GROOT_TEST_DIR), sampler);

    CHECK( texture.is_valid() );
  }
}

TEST_CASE( "image destruction" ) {
  Engine engine;

  SECTION( "image" ) {
    RID image = engine.create_storage_image(1024, 1024, Format::rgba8_unorm);
    REQUIRE( image.is_valid() );

    engine.destroy_image(image);
    CHECK_FALSE( image.is_valid() );
  }

  SECTION( "texture" ) {
    RID sampler = engine.create_sampler({});
    REQUIRE( sampler.is_valid() );

    RID texture = engine.create_texture(std::format("{}/img/test.png", GROOT_TEST_DIR), sampler);
    REQUIRE( texture.is_valid() );

    engine.destroy_image(texture);
    engine.destroy_sampler(sampler);

    CHECK_FALSE( texture.is_valid() );
    CHECK_FALSE( sampler.is_valid() );
  }
}

TEST_CASE( "invalid image operations" ) {
  Engine engine;

  SECTION( "0 in any dimension" ) {
    RID image = engine.create_storage_image(0, 1024, Format::rgba8_unorm);
    CHECK_FALSE( image.is_valid() );

    image = engine.create_storage_image(1024, 0, Format::rgba8_unorm);
    CHECK_FALSE( image.is_valid() );
  }

  SECTION( "undefined format" ) {
    RID image = engine.create_storage_image(1024, 1024, Format::undefined);
    CHECK_FALSE( image.is_valid() );
  }

  SECTION( "destroy invalid RID" ) {
    RID rid;
    engine.destroy_image(rid);
    CHECK( true );
  }

  SECTION( "destroy non-image RID" ) {
    RID buffer = engine.create_uniform_buffer(1024);
    REQUIRE( buffer.is_valid() );

    engine.destroy_image(buffer);
    CHECK( buffer.is_valid() );
  }

  SECTION( "create texture with invalid sampler RID" ) {
    RID rid;
    RID texture = engine.create_texture(std::format("{}/img/test.png", GROOT_TEST_DIR), rid);
    CHECK_FALSE( texture.is_valid() );
  }

  SECTION( "create texture with non-sampler RID" ) {
    RID buffer = engine.create_uniform_buffer(1024);
    REQUIRE( buffer.is_valid() );

    RID texture = engine.create_texture(std::format("{}/img/test.png", GROOT_TEST_DIR), buffer);
    CHECK_FALSE( texture.is_valid() );
  }

  SECTION( "create texture with invalid image" ) {
    RID sampler = engine.create_sampler({});
    REQUIRE( sampler.is_valid() );

    RID texture = engine.create_texture("", sampler);
    CHECK_FALSE( texture.is_valid() );
  }
}