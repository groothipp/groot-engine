cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)

set(GE_VERSION_MAJOR 1)
set(GE_VERSION_MINOR 0)
set(GE_VERSION_PATCH 0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fsanitize=address")

project(GrootEngine
  VERSION ${GE_VERSION_MAJOR}.${GE_VERSION_MINOR}.${GE_VERSION_PATCH}
  LANGUAGES CXX
)

add_subdirectory(${CMAKE_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_SOURCE_DIR}/tests)