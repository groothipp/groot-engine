#include "include/groot/groot.hpp"

#include <catch2/catch_test_macros.hpp>

#include <iostream>

using namespace groot;

TEST_CASE( "descriptor set creation" ) {
  Engine engine;

  SECTION( "uniform buffer" ) {
    std::println(std::cout, "--- uniform buffer descriptor set --- ");

    RID buffer = engine.create_uniform_buffer(1024);
    REQUIRE( buffer.is_valid() );

    RID set = engine.create_descriptor_set({ buffer });

    CHECK( set.is_valid() );
  }

  SECTION( "storage buffer" ) {
    std::println(std::cout, "--- storage buffer descriptor set --- ");

    RID buffer = engine.create_storage_buffer(1024);
    REQUIRE( buffer.is_valid() );

    RID set = engine.create_descriptor_set({ buffer });

    CHECK( set.is_valid() );
  }

  SECTION( "storage image" ) {
    std::println(std::cout, "--- storage image descriptor set --- ");

    RID image = engine.create_storage_image(1024, 1024);
    REQUIRE( image.is_valid() );

    RID set = engine.create_descriptor_set({ image });

    CHECK( set.is_valid() );
  }

  SECTION( "texture" ) {
    std::println(std::cout, "--- texture descriptor set --- ");

    RID sampler = engine.create_sampler({});
    REQUIRE( sampler.is_valid() );

    RID texture = engine.create_texture(std::format("{}/dat/test.png", GROOT_TEST_DIR), sampler);
    REQUIRE( sampler.is_valid() );

    RID set = engine.create_descriptor_set({ texture });

    CHECK( set.is_valid() );
  }

  SECTION( "storage texture" ) {
    std::println(std::cout, "--- storage texture descriptor set --- ");

    RID sampler = engine.create_sampler({});
    REQUIRE( sampler.is_valid() );

    RID storageTexture = engine.create_storage_texture(1024, 1024, sampler);
    REQUIRE( sampler.is_valid() );

    RID set = engine.create_descriptor_set({ storageTexture });

    CHECK( set.is_valid() );
  }

  SECTION( "all descriptor types" ) {
    std::println(std::cout, "--- complete descriptor set --- ");

    RID uniformBuffer = engine.create_uniform_buffer(1024);
    RID storageBuffer = engine.create_storage_buffer(1024);
    RID image = engine.create_storage_image(1024, 1024);
    RID sampler = engine.create_sampler({});

    REQUIRE( uniformBuffer.is_valid() );
    REQUIRE( storageBuffer.is_valid() );
    REQUIRE( image.is_valid() );
    REQUIRE( sampler.is_valid() );

    RID texture = engine.create_texture(std::format("{}/dat/test.png", GROOT_TEST_DIR), sampler);
    RID storageTexture = engine.create_storage_texture(1024, 1024, sampler);

    REQUIRE( texture.is_valid() );
    REQUIRE( storageTexture.is_valid() );

    RID set = engine.create_descriptor_set({ uniformBuffer, storageBuffer, image, texture, storageTexture });

    CHECK( set.is_valid() );
  }

  SECTION( "empty descriptor array" ) {
    std::println(std::cout, "--- empty descriptor array ---");
    RID set = engine.create_descriptor_set({});
    CHECK( set.is_valid() );
  }
}

TEST_CASE( "descriptor set destruction" ) {
  std::println(std::cout, "--- descriptor set destruction --- ");

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
    std::println(std::cout, "--- destroy invalid descriptor set RID ---");

    RID rid;
    engine.destroy_descriptor_set(rid);
    CHECK( true );
  }

  SECTION( "destroy non-descriptor-set RID" ) {
    std::println(std::cout, "--- destroy non-descriptor-set RID ---");

    RID buffer = engine.create_uniform_buffer(1024);
    REQUIRE( buffer.is_valid() );

    engine.destroy_descriptor_set(buffer);
    CHECK( buffer.is_valid() );
  }

  SECTION( "invalid descriptor type" ) {
    std::println(std::cout, "--- invalid descriptor type ---");

    RID sampler = engine.create_sampler({});
    RID goodSet = engine.create_descriptor_set({});
    RID shader = engine.compile_shader(ShaderType::Compute, std::format("{}/dat/shader.glsl", GROOT_TEST_DIR));
    REQUIRE( goodSet.is_valid() );
    REQUIRE( shader.is_valid() );

    RID pipeline = engine.create_compute_pipeline(shader, goodSet);
    REQUIRE( pipeline.is_valid() );

    RID set = engine.create_descriptor_set({ sampler, shader, goodSet, pipeline });

    CHECK_FALSE( set.is_valid() );
  }
}