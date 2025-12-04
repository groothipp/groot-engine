#include "include/groot.hpp"

#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <numbers>

#define TOLERANCE 1e-6

using namespace groot;

TEST_CASE( "operator[] access" ) {
  SECTION( "vec2" ) {
    std::println(std::cout, "--- vec2 access ---");

    vec2 u(1.0f);

    REQUIRE( u.x == u[0] );
    REQUIRE( u.y == u[1] );

    CHECK( u.x == 1.0f );
    CHECK( u.y == 1.0f );

    bool caught_out_of_range = false;
    try {
      static_cast<void>(u[4]);
    }
    catch (const std::exception& e) {
      caught_out_of_range = true;
    }

    CHECK( caught_out_of_range );
  }

  SECTION( "vec3" ) {
    std::println(std::cout, "--- vec3 access ---");

    vec3 u(1.0f);

    REQUIRE( u.x == u[0] );
    REQUIRE( u.y == u[1] );
    REQUIRE( u.z == u[2] );

    CHECK( u.x == 1.0f );
    CHECK( u.y == 1.0f );
    CHECK( u.z == 1.0f );

    bool caught_out_of_range = false;
    try {
      static_cast<void>(u[4]);
    }
    catch (const std::exception& e) {
      caught_out_of_range = true;
    }

    CHECK( caught_out_of_range );
  }

  SECTION( "vec4" ) {
    std::println(std::cout, "--- vec4 access ---");

    vec4 u(1.0f);

    REQUIRE( u.x == u[0] );
    REQUIRE( u.y == u[1] );

    CHECK( u.x == 1.0f );
    CHECK( u.y == 1.0f );

    bool caught_out_of_range = false;
    try {
      static_cast<void>(u[4]);
    }
    catch (const std::exception& e) {
      caught_out_of_range = true;
    }

    CHECK( caught_out_of_range );
  }

  SECTION( "mat2" ) {
    std::println(std::cout, "--- mat2 access ---");

    mat2 m = mat2::identity();

    CHECK( m[0] == vec2(1.0f, 0.0f) );
    CHECK( m[1] == vec2(0.0f, 1.0f) );

    bool caught_out_of_range = false;
    try {
      static_cast<void>(m[4]);
    }
    catch (const std::exception& e) {
      caught_out_of_range = true;
    }

    CHECK( caught_out_of_range );
  }

  SECTION( "mat3" ) {
    std::println(std::cout, "--- mat3 access ---");

    mat3 m = mat3::identity();

    CHECK( m[0] == vec3(1.0f, 0.0f, 0.0f) );
    CHECK( m[1] == vec3(0.0f, 1.0f, 0.0f) );
    CHECK( m[2] == vec3(0.0f, 0.0f, 1.0f) );

    bool caught_out_of_range = false;
    try {
      static_cast<void>(m[4]);
    }
    catch (const std::exception& e) {
      caught_out_of_range = true;
    }

    CHECK( caught_out_of_range );
  }

  SECTION( "mat4" ) {
    std::println(std::cout, "--- mat4 access ---");

    mat4 m = mat4::identity();

    CHECK( m[0] == vec4(1.0f, 0.0f, 0.0f, 0.0f) );
    CHECK( m[1] == vec4(0.0f, 1.0f, 0.0f, 0.0f) );
    CHECK( m[2] == vec4(0.0f, 0.0f, 1.0f, 0.0f) );
    CHECK( m[3] == vec4(0.0f, 0.0f, 0.0f, 1.0f) );

    bool caught_out_of_range = false;
    try {
      static_cast<void>(m[4]);
    }
    catch (const std::exception& e) {
      caught_out_of_range = true;
    }

    CHECK( caught_out_of_range );
  }
}

TEST_CASE( "comparison" ) {
  SECTION( "vec2") {
    std::println(std::cout, "--- vec2 comparisons ---");

    vec2 u(1.0f);
    vec2 v(1.0f);
    vec2 w(1.0f, 2.0f);

    CHECK( u == v );
    CHECK( u != w );
  }

  SECTION( "vec3" ) {
    std::println(std::cout, "--- vec3 comparison ---");

    vec3 u(1.0f);
    vec3 v(1.0f);
    vec3 w(1.0f, 2.0f, 3.0f);

    CHECK( u == v );
    CHECK( u != w );
  }

  SECTION( "vec4" ) {
    std::println(std::cout, "--- vec4 comparison ---");

    vec4 u(1.0f);
    vec4 v(1.0f);
    vec4 w(1.0f, 2.0f, 3.0f, 4.0f);

    CHECK( u == v );
    CHECK( u != w );
  }

  SECTION( "mat2" ) {
    std::println(std::cout, "--- mat2 comparison ---");

    mat2 a = mat2::identity();
    mat2 b = mat2::identity();
    mat2 c;

    CHECK( a == b );
    CHECK( a != c );
  }

  SECTION( "mat3" ) {
    std::println(std::cout, "--- mat3 comparison ---");

    mat3 a = mat3::identity();
    mat3 b = mat3::identity();
    mat3 c;

    CHECK( a == b );
    CHECK( a != c );
  }

  SECTION( "mat4" ) {
    std::println(std::cout, "--- mat4 comparison ---");

    mat4 a = mat4::identity();
    mat4 b = mat4::identity();
    mat4 c;

    CHECK( a == b );
    CHECK( a != c );
  }
}

TEST_CASE( "type casting" ) {
  SECTION( "vec2" ) {
    std::println(std::cout, "--- vec2 type casting ---");

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
    std::println(std::cout, "--- vec3 type casting ---");

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
    std::println(std::cout, "--- vec4 type casting ---");

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
    std::println(std::cout, "--- vec2 addition ---");

    vec2 u(1.0f);
    vec2 v(2.0f);
    vec2 w(3.0f);

    CHECK( u + v == w);
  }

  SECTION( "vec3" ) {
    std::println(std::cout, "--- vec3 addition ---");

    vec3 u(1.0f);
    vec3 v(2.0f);
    vec3 w(3.0f);

    CHECK( u + v == w );
  }

  SECTION( "vec4" ) {
    std::println(std::cout, "--- vec4 addition ---");

    vec4 u(1.0f);
    vec4 v(2.0f);
    vec4 w(3.0f);

    CHECK( u + v == w );
  }

  SECTION( "mat2" ) {
    std::println(std::cout, "--- mat2 addition ---");

    mat2 a(1.0f);
    mat2 b(1.0f);
    mat2 c(2.0f);

    CHECK( a + b == c );
  }

  SECTION( "mat3" ) {
    std::println(std::cout, "--- mat3 addition ---");

    mat3 a(1.0f);
    mat3 b(1.0f);
    mat3 c(2.0f);

    CHECK( a + b == c );
  }

  SECTION( "mat4" ) {
    std::println(std::cout, "--- mat4 addition ---");

    mat4 a(1.0f);
    mat4 b(1.0f);
    mat4 c(2.0f);

    CHECK( a + b == c );
  }
}

TEST_CASE( "subtraction" ) {
  SECTION( "vec2" ) {
    std::println(std::cout, "--- vec2 subtraction ---");

    vec2 u(3.0f);
    vec2 v(2.0f);
    vec2 w(1.0f);

    CHECK( u - v == w );
    CHECK( u - v == -(v - u));
  }

  SECTION( "vec3" ) {
    std::println(std::cout, "--- vec3 subtraction ---");

    vec3 u(3.0f);
    vec3 v(2.0f);
    vec3 w(1.0f);

    CHECK( u - v == w );
    CHECK( u - v == -(v - u));
  }

  SECTION( "vec4" ) {
    std::println(std::cout, "--- vec4 subtraction ---");

    vec4 u(3.0f);
    vec4 v(2.0f);
    vec4 w(1.0f);

    CHECK( u - v == w );
    CHECK( u - v == -(v - u));
  }

  SECTION( "mat2" ) {
    std::println(std::cout, "--- mat2 subtraction ---");

    mat2 a(2.0f);
    mat2 b(1.0f);

    CHECK( a - b == b );
  }

  SECTION( "mat3" ) {
    std::println(std::cout, "--- mat3 subtraction ---");

    mat3 a(2.0f);
    mat3 b(1.0f);

    CHECK( a - b == b );
  }

  SECTION( "mat4" ) {
    std::println(std::cout, "--- mat4 subtraction ---");

    mat4 a(2.0f);
    mat4 b(1.0f);

    CHECK( a - b == b );
  }
}

TEST_CASE( "negation" ) {
  SECTION( "vec2") {
    std::println(std::cout, "--- vec2 negation ---");

    vec2 u(1.0f);
    vec2 v(-1.0f);

    CHECK( -u == v );
    CHECK( -v == u );
  }

  SECTION( "vec3" ) {
    std::println(std::cout, "--- vec3 negation ---");

    vec3 u(1.0f);
    vec3 v(-1.0f);

    CHECK( -u == v );
    CHECK( -v == u );
  }

  SECTION( "vec4" ) {
    std::println(std::cout, "--- vec4 negation ---");

    vec4 u(1.0f);
    vec4 v(-1.0f);

    CHECK( -u == v );
    CHECK( -v == u );
  }

  SECTION( "mat2" ) {
    std::println(std::cout, "--- mat2 negation ---");

    mat2 a(1.0f);
    mat2 b(-1.0f);

    CHECK( -a == b );
    CHECK( -b == a );
  }

  SECTION( "mat3" ) {
    std::println(std::cout, "--- mat3 negation ---");

    mat3 a(1.0f);
    mat3 b(-1.0f);

    CHECK( -a == b );
    CHECK( -b == a );
  }

  SECTION( "mat4" ) {
    std::println(std::cout, "--- mat4 negation ---");

    mat4 a(1.0f);
    mat4 b(-1.0f);

    CHECK( -a == b );
    CHECK( -b == a );
  }
}

TEST_CASE( "multiplication" ) {
  SECTION( "vec2 scalar mult" ) {
    std::println(std::cout, "--- vec2 scalar multiplication ---");

    float s = 1.0f;
    vec2 u(2.0f);

    CHECK( u * s == u );
    CHECK( u * s == s * u );
  }

  SECTION ("vec2 vector mult") {
    std::println(std::cout, "--- vec2 vector multiplication ---");

    vec2 u(1.0f);
    vec2 v(2.0f);

    CHECK( u * v == v );
  }

  SECTION( "vec3 scalar mult" ) {
    std::println(std::cout, "--- vec3 scalar multiplication ---");

    float s = 1.0f;
    vec3 u(2.0f);

    CHECK( u * s == u );
    CHECK( u * s == s * u );
  }

  SECTION( "vec3 vector mult" ) {
    std::println(std::cout, "--- vec3 vector multiplication ---");

    vec3 u(1.0f);
    vec3 v(2.0f);

    CHECK( u * v == v );
  }

  SECTION( "vec4 scalar mult" ) {
    std::println(std::cout, "--- vec4 scalar multiplication ---");

    float s = 1.0f;
    vec4 u(2.0f);

    CHECK( u * s == u );
    CHECK( u * s == s * u );
  }

  SECTION( "vec4 vector mult" ) {
    std::println(std::cout, "--- vec4 vector multiplication ---");

    vec4 u(1.0f);
    vec4 v(2.0f);

    CHECK( u * v == v );
  }

  SECTION( "mat2 mat mult" ) {
    std::println(std::cout, "--- mat2 matrix multiplication ---");

    mat2 a(2.0f);
    mat2 b = mat2::identity();

    CHECK( a * b == a );
    CHECK( b * a == a );
  }

  SECTION( "mat2 vec mult" ) {
    std::println(std::cout, "--- mat2 vector multiplication ---");

    mat2 m = 2.0f * mat2::identity();
    vec2 u(1.0f);
    vec2 v(2.0f);

    CHECK( m * u == v );
  }

  SECTION( "mat2 scalar mult" ) {
    std::println(std::cout, "--- mat2 scalar multiplication ---");

    mat2 a = 2.0f * mat2::identity();
    mat2 b = mat2(vec2(2.0f, 0.0f), vec2(0.0f, 2.0f));

    CHECK( a == b );
  }

  SECTION( "mat3 mat mult" ) {
    std::println(std::cout, "--- mat3 matrix multiplication ---");

    mat3 a(2.0f);
    mat3 b = mat3::identity();

    CHECK( a * b == a );
    CHECK( b * a == a );
  }

  SECTION( "mat3 vec mult" ) {
    std::println(std::cout, "--- mat3 vector multiplication ---");

    mat3 m = 2.0f * mat3::identity();
    vec3 u = vec3(1.0f);
    vec3 v = vec3(2.0f);

    CHECK( m * u == v );
  }

  SECTION( "mat3 scalar mult" ) {
    std::println(std::cout, "--- mat3 scalar mult ---");

    mat3 m(2.0f);

    CHECK( 2.0f * m == mat3(4.0f) );
  }

  SECTION( "mat4 mat mult" ) {
    std::println(std::cout, "--- mat4 matrix multiplication ---");

    mat4 a(2.0f);
    mat4 b = mat4::identity();

    CHECK( a * b == a );
    CHECK( b * a == a );
  }

  SECTION( "mat4 vec mult" ) {
    std::println(std::cout, "--- mat4 vector multiplication ---");

    mat4 m = 2.0f * mat4::identity();
    vec4 u(1.0f);
    vec4 v(2.0f);

    CHECK( m * u == v );
  }

  SECTION( "mat4 scalar mult" ) {
    std::println(std::cout, "--- mat4 scalar mult ---");

    mat4 m(2.0f);

    CHECK( 2.0f * m == mat4(4.0f) );
  }
}

TEST_CASE( "division" ) {
  SECTION( "vec2 scalar div" ) {
    std::println(std::cout, "--- vec2 scalar division ---");

    float s = 2.0f;
    vec2 u(2.0f);
    vec2 v(1.0f);

    CHECK( u / s == v );
  }

  SECTION( "vec2 vector div") {
    std::println(std::cout, "--- vec2 vector division ---");

    vec2 u(2.0f);
    vec2 v(1.0f);

    CHECK( u / v == u );
  }

  SECTION( "vec3 scalar div" ) {
    std::println(std::cout, "--- vec3 scalar div ---");

    float s = 2.0f;
    vec3 u(2.0f);
    vec3 v(1.0f);

    CHECK( u / s == v );
  }

  SECTION( "vec3 vector div") {
    std::println(std::cout, "--- vec3 vector division ---");

    vec3 u(2.0f);
    vec3 v(1.0f);

    CHECK( u / v == u );
  }

  SECTION( "vec4 scalar div" ) {
    std::println(std::cout, "--- vec4 scalar division ---");

    float s = 2.0f;
    vec4 u(2.0f);
    vec4 v(1.0f);

    CHECK( u / s == v );
  }

  SECTION( "vec4 vector div" ) {
    std::println(std::cout, "--- vec4 vector division ---");

    vec4 u(2.0f);
    vec4 v(1.0f);

    CHECK( u / v == u );
  }

  SECTION( "mat2 scalar div" ) {
    std::println(std::cout, "--- mat2 scalar division ---");

    mat2 m = 2.0f * mat2::identity();

    CHECK( m / 2.0f == mat2::identity() );
  }

  SECTION( "mat3 scalar div" ) {
    std::println(std::cout, "--- mat3 scalar division ---");

    mat3 m(2.0f);

    CHECK( m / 2.0f == mat3(1.0f) );
  }

  SECTION( "mat4 scalar div" ) {
    std::println(std::cout, "--- mat4 scalar division ---");

    mat4 m(2.0f);

    CHECK( m / 2.0f == mat4(1.0f) );
  }
}

TEST_CASE( "dot product" ) {
  SECTION( "vec2" ) {
    std::println(std::cout, "--- vec2 dot product ---");

    vec2 u(2.0f);
    vec2 v(1.0f);

    CHECK( u.dot(v) == 4.0 );
  }

  SECTION( "vec3" ) {
    std::println(std::cout, "--- vec3 dot product ---");

    vec3 u(2.0f);
    vec3 v(1.0f);

    CHECK( u.dot(v) == 6.0 );
  }

  SECTION( "vec4" ) {
    std::println(std::cout, "--- vec4 dot product ---");

    vec4 u(2.0f);
    vec4 v(1.0f);

    CHECK( u.dot(v) == 8.0 );
  }
}

TEST_CASE( "cross product" ) {
  std::println(std::cout, "--- cross product ---");

  vec3 u(1.0, 0.0, 0.0);
  vec3 v(0.0, 1.0, 0.0);
  vec3 w(0.0, 0.0, 1.0);

  CHECK( u.cross(v) == w );
}

TEST_CASE( "magnitude" ) {
  SECTION( "vec2" ) {
    std::println(std::cout, "--- vec2 magnitude ---");

    vec2 u(3.0f, 4.0f);

    CHECK( u.mag() == 5.0 );
  }

  SECTION( "vec3" ) {
    std::println(std::cout, "--- vec3 magnitude ---");

    vec3 u(2.0f, 3.0f, 6.0f);

    CHECK( u.mag() == 7.0 );
  }

  SECTION( "vec4" ) {
    std::println(std::cout, "--- vec4 magnitude ---");

    vec4 u(1.0f, 3.0f, 5.0, 17.0);

    CHECK( u.mag() == 18.0 );
  }
}

TEST_CASE( "magnitude squared" ) {
  SECTION( "vec2" ) {
    std::println(std::cout, "--- vec2 magnitude squared ---");

    vec2 u(3.0f, 4.0f);

    CHECK( u.mag_squared() == 25.0 );
  }

  SECTION( "vec3" ) {
    std::println(std::cout, "--- vec3 magnitude squared ---");

    vec3 u(2.0f, 3.0f, 6.0f);

    CHECK( u.mag_squared() == 49.0 );
  }

  SECTION( "vec4" ) {
    std::println(std::cout, "--- vec4 magnitude squared ---");

    vec4 u(1.0f, 3.0f, 5.0f, 17.0f);

    CHECK( u.mag_squared() == 324.0 );
  }
}

TEST_CASE( "normalized" ) {
  SECTION( "vec2" ) {
    std::println(std::cout, "--- vec2 normalized ---");

    vec2 u(3.0f, 4.0f);
    vec2 v(3.0f / 5.0f, 4.0f / 5.0f);
    vec2 w = u.normalized();
    double err = std::abs(1.0 - w.mag());

    CHECK( w == v );
    CHECK( err < TOLERANCE );
  }

  SECTION( "vec3" ) {
    std::println(std::cout, "--- vec3 normalized ---");

    vec3 u(2.0f, 3.0f, 6.0f);
    vec3 v(2.0f / 7.0f, 3.0f / 7.0f, 6.0f / 7.0f);
    vec3 w = u.normalized();
    double err = std::abs(1.0 - w.mag());

    CHECK( w == v );
    CHECK( err < TOLERANCE );
  }

  SECTION( "vec4" ) {
    std::println(std::cout, "--- vec4 normalized ---");

    vec4 u(1.0f, 3.0f, 5.0f, 17.0f);
    vec4 v(1.0f / 18.0f, 3.0f / 18.0f, 5.0f / 18.0f, 17.0f / 18.0f);
    vec4 w = u.normalized();
    double err = std::abs(1.0 - w.mag());

    CHECK( w == v );
    CHECK( err < TOLERANCE );
  }
}

TEST_CASE( "inverse" ) {
  SECTION( "mat2" ) {
    std::println(std::cout, "--- mat2 inverse ---");

    mat2 a = mat2::identity();
    mat2 b = 2.0f * mat2::identity();

    CHECK( a.inverse() == a );
    CHECK( b.inverse() == 0.5 * mat2::identity() );
  }

  SECTION( "mat3" ) {
    std::println(std::cout, "--- mat3 inverse ---");

    mat3 a = mat3::identity();
    mat3 b = 2.0f * mat3::identity();

    CHECK( a.inverse() == a );
    CHECK( b.inverse() == 0.5 * mat3::identity() );
  }

  SECTION( "mat4" ) {
    std::println(std::cout, "--- mat4 inverse ---");

    mat4 a = mat4::identity();
    mat4 b = 2.0f * mat4::identity();

    CHECK( a.inverse() == a );
    CHECK( b.inverse() == 0.5 * mat4::identity() );
  }
}

TEST_CASE( "transpose" ) {
  SECTION( "mat2" ) {
    std::println(std::cout, "--- mat2 transpose ---");

    mat2 a(vec2(1.0f, 2.0f), vec2(1.0f, 2.0f));
    mat2 b(vec2(1.0f, 1.0), vec2(2.0f, 2.0f));
    mat2 c = mat2::identity();

    CHECK( a.transpose() == b );
    CHECK( c.transpose() == c );
  }

  SECTION( "mat3" ) {
    std::println(std::cout, "--- mat3 transpose ---");

    mat3 a(vec3(1.0f, 2.0f, 3.0f), vec3(1.0f, 2.0f, 3.0f), vec3(1.0f, 2.0f, 3.0f));
    mat3 b(vec3(1.0f, 1.0f, 1.0f), vec3(2.0f, 2.0f, 2.0f), vec3(3.0f, 3.0f, 3.0f));
    mat3 c = mat3::identity();

    CHECK( a.transpose() == b );
    CHECK( c.transpose() == c );
  }

  SECTION( "mat4" ) {
    std::println(std::cout, "--- mat4 transpose ---");

    mat4 a(vec4(1.0f, 2.0f, 3.0f, 4.0f), vec4(1.0f, 2.0f, 3.0f, 4.0f), vec4(1.0f, 2.0f, 3.0f, 4.0f), vec4(1.0f, 2.0f, 3.0f, 4.0f));
    mat4 b(vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(2.0f, 2.0f, 2.0f, 2.0f), vec4(3.0f, 3.0f, 3.0f, 3.0f), vec4(4.0f, 4.0f, 4.0f, 4.0f));
    mat4 c = mat4::identity();

    CHECK( a.transpose() == b );
    CHECK( c.transpose() == c );
  }
}

TEST_CASE( "determinant" ) {
  SECTION( "mat2" ) {
    std::println(std::cout, "--- mat2 determinant ---");

    mat2 a = mat2::identity();
    mat2 b(1.0f);
    mat2 c(vec2(1.0f, 2.0f), vec2(3.0f, 4.0f));

    CHECK( a.determinant() == 1.0f );
    CHECK( b.determinant() == 0.0f );
    CHECK( c.determinant() == -2.0f );
  }

  SECTION( "mat3" ) {
    std::println(std::cout, "--- mat3 determiniant ---");

    mat3 a = mat3::identity();
    mat3 b(1.0f);
    mat3 c(vec3(1.0f, 1.0f, 3.0f), vec3(1.0f, 3.0f, 2.0f), vec3(1.0f, 2.0f, 1.0f));

    CHECK( a.determinant() == 1.0f );
    CHECK( b.determinant() == 0.0f );
    CHECK( c.determinant() == -3.0f );
  }

  SECTION( "mat4" ) {
    std::println(std::cout, "--- mat4 determinant ---");

    mat4 a = mat4::identity();
    mat4 b(1.0f);
    mat4 c(
      vec4(1.0f, 7.0f, 1.0f, 1.0f),
      vec4(4.0f, 6.0f, 1.0f, 4.0f),
      vec4(3.0f, 2.0f, 1.0f, 2.0f),
      vec4(2.0f, 1.0f, 1.0f, 2.0f)
    );

    CHECK( a.determinant() == 1.0f );
    CHECK( b.determinant() == 0.0f );
    CHECK( c.determinant() == -17.0f );
  }
}

TEST_CASE( "trace" ) {
  SECTION( "mat2") {
    std::println(std::cout, "--- mat2 trace ---");
    CHECK( mat2(1.0f).trace() == 2.0f );
  }

  SECTION( "mat3" ) {
    std::println("--- mat3 trace ---");
    CHECK( mat3(1.0f).trace() == 3.0f );
  }

  SECTION( "mat4" ) {
    std::println(std::cout, "--- mat4 trace ---");
    CHECK( mat4(1.0f).trace() == 4.0f );
  }
}

TEST_CASE( "identity" ) {
  SECTION( "mat2" ) {
    std::println(std::cout, "--- mat2 identity ---");

    mat2 m = mat2::identity();

    for (int i = 0; i < 2; ++i) {
      for (int j = 0; j < 2; ++j)
        CHECK( m[i][j] == (i == j) );
    }
  }

  SECTION( "mat3" ) {
    std::println(std::cout, "--- mat3 identity ---");

    mat3 m = mat3::identity();

    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j)
        CHECK( m[i][j] == (i == j));
    }
  }

  SECTION( "mat4" ) {
    std::println(std::cout, "--- mat4 identity ---");

    mat4 m = mat4::identity();

    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j)
        CHECK( m[i][j] == (i == j));
    }
  }
}

TEST_CASE( "rotation" ) {
  SECTION( "mat2" ) {
    std::println(std::cout, "--- mat2 rotation ---");

    mat2 m = mat2::rotation(std::numbers::pi_v<float> / 2.0f);
    vec2 u(1.0f, 0.0f);
    vec2 result = m * u;
    vec2 expected(0.0f, 1.0f);

    CHECK( mat2::rotation(0.0f) == mat2::identity() );
    CHECK( (result - expected).mag() < TOLERANCE );
  }

  SECTION( "mat3 axis rotations" ) {
    std::println(std::cout, "--- mat3 axis rotations ---");

    mat3 rx = mat3::rotation_x(std::numbers::pi_v<float> / 2.0f);
    vec3 y_axis(0.0f, 1.0f, 0.0f);
    vec3 result_x = rx * y_axis;
    vec3 expected_x(0.0f, 0.0f, 1.0f);

    mat3 ry = mat3::rotation_y(std::numbers::pi_v<float> / 2.0f);
    vec3 z_axis(0.0f, 0.0f, 1.0f);
    vec3 result_y = ry * z_axis;
    vec3 expected_y(1.0f, 0.0f, 0.0f);

    mat3 rz = mat3::rotation_z(std::numbers::pi_v<float> / 2.0f);
    vec3 x_axis(1.0f, 0.0f, 0.0f);
    vec3 result_z = rz * x_axis;
    vec3 expected_z(0.0f, 1.0f, 0.0f);

    CHECK( mat3::rotation_x(0.0f) == mat3::identity() );
    CHECK( mat3::rotation_y(0.0f) == mat3::identity() );
    CHECK( mat3::rotation_z(0.0f) == mat3::identity() );

    CHECK( (result_x - expected_x).mag() < TOLERANCE );
    CHECK( (result_y - expected_y).mag() < TOLERANCE );
    CHECK( (result_z - expected_z).mag() < TOLERANCE );
  }

  SECTION( "mat3 axis-angle rotation" ) {
    std::println(std::cout, "--- mat3 axis-angle rotation ---");

    vec3 z_axis(0.0f, 0.0f, 1.0f);
    mat3 rot_z = mat3::rotation(z_axis, std::numbers::pi_v<float> / 2.0f);
    mat3 expected_z = mat3::rotation_z(std::numbers::pi_v<float> / 2.0f);

    vec3 diagonal(1.0f, 1.0f, 1.0f);
    mat3 rot_diag = mat3::rotation(diagonal, std::numbers::pi_v<float> / 4.0f);
    vec3 test_vec(1.0f, 0.0f, 0.0f);
    vec3 result = rot_diag * test_vec;

    mat3 identity_rot = mat3::rotation(vec3(1.0f, 0.0f, 0.0f), 0.0f);

    CHECK( (rot_z[0] - expected_z[0]).mag() < TOLERANCE );
    CHECK( (rot_z[1] - expected_z[1]).mag() < TOLERANCE );
    CHECK( (rot_z[2] - expected_z[2]).mag() < TOLERANCE );
    CHECK( (identity_rot[0] - mat3::identity()[0]).mag() < TOLERANCE );
    CHECK( (identity_rot[1] - mat3::identity()[1]).mag() < TOLERANCE );
    CHECK( (identity_rot[2] - mat3::identity()[2]).mag() < TOLERANCE );
    CHECK( result.mag() - 1.0f < TOLERANCE );
  }

  SECTION( "mat3 euler rotation" ) {
    std::println(std::cout, "--- mat3 euler rotation ---");

    float pitch = std::numbers::pi_v<float> / 6.0f;
    float yaw = std::numbers::pi_v<float> / 4.0f;
    float roll = std::numbers::pi_v<float> / 3.0f;

    mat3 euler = mat3::euler_rotation(pitch, yaw, roll);
    mat3 composed = mat3::rotation_z(roll) * mat3::rotation_x(pitch) * mat3::rotation_y(yaw);

    CHECK( (euler[0] - composed[0]).mag() < TOLERANCE );
    CHECK( (euler[1] - composed[1]).mag() < TOLERANCE );
    CHECK( (euler[2] - composed[2]).mag() < TOLERANCE );

    mat3 zero_euler = mat3::euler_rotation(0.0f, 0.0f, 0.0f);
    CHECK( (zero_euler[0] - mat3::identity()[0]).mag() < TOLERANCE );
    CHECK( (zero_euler[1] - mat3::identity()[1]).mag() < TOLERANCE );
    CHECK( (zero_euler[2] - mat3::identity()[2]).mag() < TOLERANCE );

    mat3 pitch_only = mat3::euler_rotation(pitch, 0.0f, 0.0f);
    mat3 expected_pitch = mat3::rotation_x(pitch);
    CHECK( (pitch_only[0] - expected_pitch[0]).mag() < TOLERANCE );
    CHECK( (pitch_only[1] - expected_pitch[1]).mag() < TOLERANCE );
    CHECK( (pitch_only[2] - expected_pitch[2]).mag() < TOLERANCE );
  }

  SECTION( "mat4 axis-angle rotation" ) {
    std::println(std::cout, "--- mat4 axis-angle rotation ---");

    vec3 z_axis(0.0f, 0.0f, 1.0f);
    mat4 rot_z = mat4::rotation(z_axis, std::numbers::pi_v<float> / 2.0f);
    vec4 test_vec(1.0f, 0.0f, 0.0f, 1.0f);
    vec4 result = rot_z * test_vec;
    vec4 expected(0.0f, 1.0f, 0.0f, 1.0f);

    CHECK( (result - expected).mag() < TOLERANCE );

    mat4 identity_rot = mat4::rotation(vec3(1.0f, 0.0f, 0.0f), 0.0f);
    mat4 identity_expected = mat4::identity();

    CHECK( (identity_rot[0] - identity_expected[0]).mag() < TOLERANCE );
    CHECK( (identity_rot[1] - identity_expected[1]).mag() < TOLERANCE );
    CHECK( (identity_rot[2] - identity_expected[2]).mag() < TOLERANCE );
    CHECK( (identity_rot[3] - identity_expected[3]).mag() < TOLERANCE );
  }
}

TEST_CASE( "scale" ) {
  SECTION( "mat2" ) {
    std::println(std::cout, "--- mat2 scale ---");

    CHECK( mat2::scale(1.0f, 1.0f) == mat2::identity() );
    CHECK( mat2::scale(2.0f, 2.0f) * vec2(1.0f, 1.0f) == vec2(2.0f, 2.0f) );
  }

  SECTION( "mat3" ) {
    std::println(std::cout, "--- mat3 scale ---");

    CHECK( mat3::scale(1.0f, 1.0f, 1.0f) == mat3::identity() );
    CHECK( mat3::scale(2.0f, 3.0f, 4.0f) * vec3(1.0f, 1.0f, 1.0f) == vec3(2.0f, 3.0f, 4.0f) );
  }

  SECTION( "mat4" ) {
    std::println(std::cout, "--- mat4 scale ---");

    CHECK( mat4::scale(1.0f, 1.0f, 1.0f) == mat4::identity() );
    CHECK( mat4::scale(2.0f, 3.0f, 4.0f) * vec4(1.0f, 1.0f, 1.0f, 1.0f) == vec4(2.0f, 3.0f, 4.0f, 1.0f) );
  }
}

TEST_CASE( "translation" ) {
  SECTION( "mat4" ) {
    std::println(std::cout, "--- mat4 translation ---");

    vec3 offset(1.0f, 2.0f, 3.0f);
    mat4 trans = mat4::translation(offset);
    vec4 point(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 result = trans * point;
    vec4 expected(1.0f, 2.0f, 3.0f, 1.0f);

    CHECK( result == expected );
    CHECK( mat4::translation(vec3(0.0f, 0.0f, 0.0f)) == mat4::identity() );
  }
}