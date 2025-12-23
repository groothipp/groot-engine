#include "include/groot/groot.hpp"

#include <catch2/catch_test_macros.hpp>

#include <iostream>

using namespace groot;

TEST_CASE( "buffer creation" ) {
  Engine engine;

  SECTION( "uniform buffer" ) {
    std::println(std::cout, "--- create uniform buffer ---");
    RID buffer = engine.create_uniform_buffer(1024);
    CHECK( buffer.is_valid() );
  }

  SECTION( "storage buffer" ) {
    std::println(std::cout, "--- create storage buffer ---");
    RID buffer = engine.create_storage_buffer(1024);
    CHECK( buffer.is_valid() );
  }
}

TEST_CASE( "buffer destruction" ) {
  std::println(std::cout, "--- buffer destruction ---");

  Engine engine;

  RID buffer = engine.create_uniform_buffer(1024);
  REQUIRE( buffer.is_valid() );

  engine.destroy_buffer(buffer);

  CHECK_FALSE( buffer.is_valid() );
}

TEST_CASE( "buffer read and write" ) {
  Engine engine;

  SECTION( "read/write vector" ) {
    std::println(std::cout, "--- read/write vector ---");

    std::vector<int> data(256);
    RID buffer = engine.create_uniform_buffer(sizeof(int) * data.size());
    REQUIRE( buffer.is_valid() );

    engine.write_buffer(buffer, data);

    std::vector<int> out = engine.read_buffer<int>(buffer);
    CHECK( out == data );
  }

  SECTION( "read/write value" ) {
    std::println(std::cout, "--- read/write value ---");

    int val = 4;
    RID buffer = engine.create_uniform_buffer(sizeof(int));
    REQUIRE( buffer.is_valid() );

    engine.write_buffer(buffer, val);

    int out = engine.read_buffer<int>(buffer, -1);
    CHECK( val == out );
  }
}

TEST_CASE( "invalid buffer operations" ) {
  Engine engine;

  SECTION( "invalid RID" ) {
    std::println(std::cout, "--- destroy invalid buffer RID ---");

    RID rid;
    engine.destroy_buffer(rid);
    CHECK( true );
  }

  SECTION( "size 0 creation" ) {
    std::println(std::cout, "--- size 0 buffer creation ---");

    RID uniform = engine.create_uniform_buffer(0);
    CHECK_FALSE( uniform.is_valid() );

    RID storage = engine.create_storage_buffer(0);
    CHECK_FALSE( storage.is_valid() );
  }

  SECTION( "destroy non-buffer RID" ) {
    std::println(std::cout, "--- destroy non-buffer RID ---");

    RID image = engine.create_storage_image(1024, 1024);
    REQUIRE( image.is_valid() );

    engine.destroy_buffer(image);
    CHECK( image.is_valid() );
  }

  SECTION( "read invalid buffer RID" ) {
    std::println(std::cout, "--- read invalid buffer RID ---");

    RID rid;

    std::vector<int> data = engine.read_buffer<int>(rid);
    CHECK( data.empty() );

    int val = engine.read_buffer<int>(rid, -1);
    CHECK( val == -1);
  }

  SECTION( "read non-buffer RID" ) {
    std::println(std::cout, "--- read non-buffer RID ---");

    RID image = engine.create_storage_image(1024, 1024);
    REQUIRE( image.is_valid() );

    std::vector<int> data = engine.read_buffer<int>(image);
    CHECK( data.empty() );

    int val = engine.read_buffer<int>(image, -1);
    CHECK( val == -1 );
  }

  SECTION( "write invalid buffer RID" ) {
    std::println(std::cout, "--- buffer write invalid buffer RID ---");

    RID rid;

    std::vector<int> data(256);
    engine.write_buffer(rid, data);

    int val = 4;
    engine.write_buffer(rid, 4);

    CHECK( true );
  }

  SECTION( "write non-buffer RID" ) {
    std::println(std::cout, "--- write non-buffer RID ---");

    RID image = engine.create_storage_image(1024, 1024);
    REQUIRE( image.is_valid() );

    std::vector<int> data(256);
    engine.write_buffer(image, data);

    int val = 4;
    engine.write_buffer(image, 4);

    CHECK( true );
  }

  SECTION( "write empty vector" ) {
    std::println(std::cout, "--- buffer write empty vector ---");

    RID buffer = engine.create_uniform_buffer(1024);
    REQUIRE( buffer.is_valid() );

    engine.write_buffer(buffer, std::vector<int>{});

    CHECK( true );
  }
}