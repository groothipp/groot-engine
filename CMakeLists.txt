cmake_minimum_required(VERSION 4.2)

option(ENABLE_ASAN "Enable Address Sanitizer" OFF)

set(GROOT_VERSION_MAJOR 2)
set(GROOT_VERSION_MINOR 91)
set(GROOT_VERSION_PATCH 20)

project(GrootEngine
  VERSION ${GROOT_VERSION_MAJOR}.${GROOT_VERSION_MINOR}.${GROOT_VERSION_PATCH}
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)

if (ENABLE_ASAN)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_SOURCE_DIR}/tests)