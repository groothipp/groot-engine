project(GrootEngine::Tests)

find_package(Catch2 3 REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(TESTS_SOURCES
  ${CMAKE_SOURCE_DIR}/include/groot.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/buffers.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/descriptor_sets.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/dispatch.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/images.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/linalg.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/mesh.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/objects.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pipelines.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/render.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/shader_compilation.cpp
)

add_executable(test ${TESTS_SOURCES})

target_compile_definitions(test PRIVATE
  "GROOT_TEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\""
)

target_include_directories(test PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(test PRIVATE
  groot
  Catch2::Catch2WithMain
)