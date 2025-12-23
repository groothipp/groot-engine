#include "include/groot/groot.hpp"

#include <catch2/catch_test_macros.hpp>

#include <iostream>

using namespace groot;

TEST_CASE( "image creation" ) {
  Engine engine;

  SECTION( "storage image" ) {
    std::println(std::cout, "--- create storage image ---");
    RID image = engine.create_storage_image(1024, 1024);
    CHECK( image.is_valid() );
  }

  SECTION( "texture" ) {
    std::println(std::cout, "--- create texture ---");
    RID sampler = engine.create_sampler({});
    REQUIRE( sampler.is_valid() );

    RID texture = engine.create_texture(std::format("{}/dat/test.png", GROOT_TEST_DIR), sampler);

    CHECK( texture.is_valid() );
  }

  SECTION( "create storage texture") {
    std::println(std::cout, "--- create storage texture ---");
    RID sampler = engine.create_sampler({});
    REQUIRE( sampler.is_valid() );

    RID storageTexture = engine.create_storage_texture(1024, 1024, sampler);

    CHECK( storageTexture.is_valid() );
  }
}

TEST_CASE( "image destruction" ) {
  Engine engine;

  SECTION( "image" ) {
    std::println(std::cout, "--- destroy image ---");

    RID image = engine.create_storage_image(1024, 1024);
    REQUIRE( image.is_valid() );

    engine.destroy_image(image);
    CHECK_FALSE( image.is_valid() );
  }

  SECTION( "texture" ) {
    std::println(std::cout, "--- destroy sampled image ---");

    RID sampler = engine.create_sampler({});
    REQUIRE( sampler.is_valid() );

    RID texture = engine.create_texture(std::format("{}/dat/test.png", GROOT_TEST_DIR), sampler);
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
    std::println(std::cout, "--- create image with 0 in dimension ---");

    RID image = engine.create_storage_image(0, 1024);
    CHECK_FALSE( image.is_valid() );

    image = engine.create_storage_image(1024, 0);
    CHECK_FALSE( image.is_valid() );
  }

  SECTION( "undefined format" ) {
    std::println(std::cout, "--- create image with undefined format ---");

    RID image = engine.create_storage_image(1024, 1024, ImageType::two_dim, Format::undefined);
    CHECK_FALSE( image.is_valid() );
  }

  SECTION( "destroy invalid RID" ) {
    std::println(std::cout, "--- destroy image with invalid RID ---");

    RID rid;
    engine.destroy_image(rid);
    CHECK( true );
  }

  SECTION( "destroy non-image RID" ) {
    std::println(std::cout, "--- destroy non-image RID ---");

    RID buffer = engine.create_uniform_buffer(1024);
    REQUIRE( buffer.is_valid() );

    engine.destroy_image(buffer);
    CHECK( buffer.is_valid() );
  }

  SECTION( "create texture with invalid sampler RID" ) {
    std::println(std::cout, "--- create texture with invalid sampler RID ---");

    RID rid;
    RID texture = engine.create_texture(std::format("{}/dat/test.png", GROOT_TEST_DIR), rid);
    CHECK_FALSE( texture.is_valid() );
  }

  SECTION( "create texture with non-sampler RID" ) {
    std::println(std::cout, "--- create texture with non-sampler RID ---");

    RID buffer = engine.create_uniform_buffer(1024);
    REQUIRE( buffer.is_valid() );

    RID texture = engine.create_texture(std::format("{}/dat/test.png", GROOT_TEST_DIR), buffer);
    CHECK_FALSE( texture.is_valid() );
  }

  SECTION( "create texture with invalid image" ) {
    std::println(std::cout, "--- create texture with invalid image ---");

    RID sampler = engine.create_sampler({});
    REQUIRE( sampler.is_valid() );

    RID texture = engine.create_texture("", sampler);
    CHECK_FALSE( texture.is_valid() );
  }

  SECTION( "create storage texture with invalid sampler RID" ) {
    std::println(std::cout, "--- create storage texture with invalid sampler RID ---");

    RID rid;
    RID storageTexture = engine.create_storage_texture(1024, 1024, rid);

    CHECK_FALSE( storageTexture.is_valid() );
  }

  SECTION( "create storage texture with non-sampler RID" ) {
    std::println(std::cout, "--- create storage texture with non-sampler RID" );

    RID buffer = engine.create_uniform_buffer(1024);
    REQUIRE( buffer.is_valid() );

    RID storageTexture = engine.create_storage_texture(1024, 1024, buffer);
    CHECK_FALSE( storageTexture.is_valid() );
  }

  SECTION( "create storage texture with undefined format" ) {
    std::println(std::cout, "--- create storage texture with undefined format ---");

    RID sampler = engine.create_sampler({});
    REQUIRE( sampler.is_valid() );

    RID storageTexture = engine.create_storage_texture(1024, 1024, sampler, ImageType::two_dim, Format::undefined);
    CHECK_FALSE( storageTexture.is_valid() );
  }
}