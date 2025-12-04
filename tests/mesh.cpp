#include "include/groot.hpp"

#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <exception>

using namespace groot;

TEST_CASE( "load mesh" ) {
  std::println(std::cout, "--- load mesh ---");

  Engine engine;

  RID mesh = engine.load_mesh(std::format("{}/dat/mesh.obj", GROOT_TEST_DIR));

  CHECK( mesh.is_valid() );
}

TEST_CASE( "destroy mesh" ) {
  std::println(std::cout, "--- destroy mesh ---");

  Engine engine;

  RID mesh = engine.load_mesh(std::format("{}/dat/mesh.obj", GROOT_TEST_DIR));
  REQUIRE( mesh.is_valid() );

  engine.destroy_mesh(mesh);
  CHECK_FALSE( mesh.is_valid() );
}

TEST_CASE( "invalid mesh operations" ) {
  Engine engine;

  SECTION( "invalid path" ) {
    std::println(std::cout, "--- invalid mesh path ---");

    bool caught = false;
    try {
      RID mesh = engine.load_mesh("");
    }
    catch (const std::exception&) {
      caught = true;
    }

    CHECK( caught );
  }

  SECTION( "destroy invalid mesh RID" ) {
    std::println(std::cout, "--- destroy invalid mesh RID ---");

    RID rid;
    engine.destroy_mesh(rid);
    CHECK( true );
  }

  SECTION( "destroy non-mesh RID" ) {
    std::println(std::cout, "--- destroy non-mesh RID ---");

    RID buffer = engine.create_storage_buffer(1024);
    REQUIRE( buffer.is_valid() );

    engine.destroy_mesh(buffer);
    CHECK( buffer.is_valid() );
  }
}