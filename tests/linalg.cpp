#include "include/groot.hpp"

#include <catch2/catch_test_macros.hpp>
#include <print>

#define TOLERANCE 1e-6

using namespace groot;

TEST_CASE( "comparison" ) {
  SECTION( "vec2") {
    std::println("--- vec2 comparisons ---");

    vec2 u(1.0f);
    vec2 v(1.0f);
    vec2 w(1.0f, 2.0f);

    CHECK( u == v );
    CHECK( u != w );
  }

  SECTION( "vec3" ) {
    std::println("--- vec3 comparison ---");

    vec3 u(1.0f);
    vec3 v(1.0f);
    vec3 w(1.0f, 2.0f, 3.0f);

    CHECK( u == v );
    CHECK( u != w );
  }

  SECTION( "vec4" ) {
    std::println("--- vec4 comparison ---");

    vec4 u(1.0f);
    vec4 v(1.0f);
    vec4 w(1.0f, 2.0f, 3.0f, 4.0f);

    CHECK( u == v );
    CHECK( u != w );
  }
}

TEST_CASE( "type casting" ) {
  SECTION( "vec2" ) {
    std::println("--- vec2 type casting ---");

    vec2 u(1.0f);
    ivec2 v(1);
    uvec2 w(1u);

    CHECK( u == vec2(v) );
    CHECK( u == vec2(w) );
    CHECK( v == ivec2(u) );
    CHECK( v == ivec2(w) );
    CHECK( w == uvec2(u) );
    CHECK( w == uvec2(v) );
  }

  SECTION( "vec3" ) {
    std::println("--- vec3 type casting ---");

    vec3 u(1.0f);
    ivec3 v(1);
    uvec3 w(1u);

    CHECK( u == vec3(v) );
    CHECK( u == vec3(w) );
    CHECK( v == ivec3(u) );
    CHECK( v == ivec3(w) );
    CHECK( w == uvec3(u) );
    CHECK( w == uvec3(v) );
  }

  SECTION( "vec4" ) {
    std::println("--- vec4 type casting ---");

    vec4 u(1.0f);
    ivec4 v(1);
    uvec4 w(1u);

    CHECK( u == vec4(v) );
    CHECK( u == vec4(w) );
    CHECK( v == ivec4(u) );
    CHECK( v == ivec4(w) );
    CHECK( w == uvec4(u) );
    CHECK( w == uvec4(v) );
  }
}

TEST_CASE( "addition" ) {
  SECTION( "vec2" ) {
    std::println("--- vec2 addition ---");

    vec2 u(1.0f);
    vec2 v(2.0f);
    vec2 w(3.0f);

    CHECK( u + v == w);
  }

  SECTION( "vec3" ) {
    std::println("--- vec3 addition ---");

    vec3 u(1.0f);
    vec3 v(2.0f);
    vec3 w(3.0f);

    CHECK( u + v == w );
  }

  SECTION( "vec4" ) {
    std::println("--- vec4 addition ---");

    vec4 u(1.0f);
    vec4 v(2.0f);
    vec4 w(3.0f);

    CHECK( u + v == w );
  }
}

TEST_CASE( "subtraction" ) {
  SECTION( "vec2" ) {
    std::println("--- vec2 subtraction ---");

    vec2 u(3.0f);
    vec2 v(2.0f);
    vec2 w(1.0f);

    CHECK( u - v == w );
    CHECK( u - v == -(v - u));
  }

  SECTION( "vec3" ) {
    std::println("--- vec3 subtraction ---");

    vec3 u(3.0f);
    vec3 v(2.0f);
    vec3 w(1.0f);

    CHECK( u - v == w );
    CHECK( u - v == -(v - u));
  }

  SECTION( "vec4" ) {
    std::println("--- vec4 subtraction ---");

    vec4 u(3.0f);
    vec4 v(2.0f);
    vec4 w(1.0f);

    CHECK( u - v == w );
    CHECK( u - v == -(v - u));
  }
}

TEST_CASE( "negation" ) {
  SECTION( "vec2") {
    std::println("--- vec2 negation ---");

    vec2 u(1.0f);
    vec2 v(-1.0f);

    CHECK( -u == v );
    CHECK( -v == u );
  }

  SECTION( "vec3" ) {
    std::println("--- vec3 negation ---");

    vec3 u(1.0f);
    vec3 v(-1.0f);

    CHECK( -u == v );
    CHECK( -v == u );
  }

  SECTION( "vec4" ) {
    std::println("--- vec4 negation ---");

    vec4 u(1.0f);
    vec4 v(-1.0f);

    CHECK( -u == v );
    CHECK( -v == u );
  }
}

TEST_CASE( "multiplication" ) {
  SECTION( "vec2 scalar mult" ) {
    std::println("--- vec2 scalar multiplication ---");

    float s = 1.0f;
    vec2 u(2.0f);

    CHECK( u * s == u );
    CHECK( u * s == s * u );
  }

  SECTION ("vec2 vector mult") {
    std::println("--- vec2 vector multiplication ---");

    vec2 u(1.0f);
    vec2 v(2.0f);

    CHECK( u * v == v );
  }

  SECTION( "vec3 scalar mult" ) {
    std::println("--- vec3 scalar multiplication ---");

    float s = 1.0f;
    vec3 u(2.0f);

    CHECK( u * s == u );
    CHECK( u * s == s * u );
  }

  SECTION( "vec3 vector mult" ) {
    std::println("--- vec3 vector multiplication ---");

    vec3 u(1.0f);
    vec3 v(2.0f);

    CHECK( u * v == v );
  }

  SECTION( "vec4 scalar mult" ) {
    std::println("--- vec4 scalar multiplication ---");

    float s = 1.0f;
    vec4 u(2.0f);

    CHECK( u * s == u );
    CHECK( u * s == s * u );
  }

  SECTION( "vec4 vector mult" ) {
    std::println("--- vec4 vector multiplication ---");

    vec4 u(1.0f);
    vec4 v(2.0f);

    CHECK( u * v == v );
  }
}

TEST_CASE( "division" ) {
  SECTION( "vec2 scalar div" ) {
    std::println("--- vec2 scalar division ---");

    float s = 2.0f;
    vec2 u(2.0f);
    vec2 v(1.0f);

    CHECK( u / s == v );
  }

  SECTION( "vec2 vector div") {
    std::println("--- vec2 vector division ---");

    vec2 u(2.0f);
    vec2 v(1.0f);

    CHECK( u / v == u );
  }

  SECTION( "vec3 scalar div" ) {
    std::println("--- vec3 scalar div ---");

    float s = 2.0f;
    vec3 u(2.0f);
    vec3 v(1.0f);

    CHECK( u / s == v );
  }

  SECTION( "vec3 vector div") {
    std::println("--- vec3 vector division ---");

    vec3 u(2.0f);
    vec3 v(1.0f);

    CHECK( u / v == u );
  }

  SECTION( "vec4 scalar div" ) {
    std::println("--- vec4 scalar division ---");

    float s = 2.0f;
    vec4 u(2.0f);
    vec4 v(1.0f);

    CHECK( u / s == v );
  }

  SECTION( "vec4 vector div" ) {
    std::println("--- vec4 vector division ---");

    vec4 u(2.0f);
    vec4 v(1.0f);

    CHECK( u / v == u );
  }
}

TEST_CASE( "dot product" ) {
  SECTION( "vec2" ) {
    std::println("--- vec2 dot product ---");

    vec2 u(2.0f);
    vec2 v(1.0f);

    CHECK( u.dot(v) == 4.0 );
  }

  SECTION( "vec3" ) {
    std::println("--- vec3 dot product ---");

    vec3 u(2.0f);
    vec3 v(1.0f);

    CHECK( u.dot(v) == 6.0 );
  }

  SECTION( "vec4" ) {
    std::println("--- vec4 dot product ---");

    vec4 u(2.0f);
    vec4 v(1.0f);

    CHECK( u.dot(v) == 8.0 );
  }
}

TEST_CASE( "cross product" ) {
  std::println("--- cross product ---");

  vec3 u(1.0, 0.0, 0.0);
  vec3 v(0.0, 1.0, 0.0);
  vec3 w(0.0, 0.0, 1.0);

  CHECK( u.cross(v) == w );
}

TEST_CASE( "magnitude" ) {
  SECTION( "vec2" ) {
    std::println("--- vec2 magnitude ---");

    vec2 u(3.0f, 4.0f);

    CHECK( u.mag() == 5.0 );
  }

  SECTION( "vec3" ) {
    std::println("--- vec3 magnitude ---");

    vec3 u(2.0f, 3.0f, 6.0f);

    CHECK( u.mag() == 7.0 );
  }

  SECTION( "vec4" ) {
    std::println("--- vec4 magnitude ---");

    vec4 u(1.0f, 3.0f, 5.0, 17.0);

    CHECK( u.mag() == 18.0 );
  }
}

TEST_CASE( "magnitude squared" ) {
  SECTION( "vec2" ) {
    std::println("--- vec2 magnitude squared ---");

    vec2 u(3.0f, 4.0f);

    CHECK( u.mag_squared() == 25.0 );
  }

  SECTION( "vec3" ) {
    std::println("--- vec3 magnitude squared ---");

    vec3 u(2.0f, 3.0f, 6.0f);

    CHECK( u.mag_squared() == 49.0 );
  }

  SECTION( "vec4" ) {
    std::println("--- vec4 magnitude squared ---");

    vec4 u(1.0f, 3.0f, 5.0f, 17.0f);

    CHECK( u.mag_squared() == 324.0 );
  }
}

TEST_CASE( "normalized" ) {
  SECTION( "vec2" ) {
    vec2 u(3.0f, 4.0f);
    vec2 v(3.0f / 5.0f, 4.0f / 5.0f);
    vec2 w = u.normalized();
    double err = std::abs(1.0 - w.mag());

    CHECK( w == v );
    CHECK( err < TOLERANCE );
  }

  SECTION( "vec3" ) {
    vec3 u(2.0f, 3.0f, 6.0f);
    vec3 v(2.0f / 7.0f, 3.0f / 7.0f, 6.0f / 7.0f);
    vec3 w = u.normalized();
    double err = std::abs(1.0 - w.mag());

    CHECK( w == v );
    CHECK( err < TOLERANCE );
  }

  SECTION( "vec4" ) {
    vec4 u(1.0f, 3.0f, 5.0f, 17.0f);
    vec4 v(1.0f / 18.0f, 3.0f / 18.0f, 5.0f / 18.0f, 17.0f / 18.0f);
    vec4 w = u.normalized();
    double err = std::abs(1.0 - w.mag());

    CHECK( w == v );
    CHECK( err < TOLERANCE );
  }
}