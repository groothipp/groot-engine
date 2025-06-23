project(GrootEngine::Tests)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Catch2 3 REQUIRED)
find_program(GLSLC glslc REQUIRED)

set(TESTS_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/u_engine.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/u_linalg.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/u_parsers.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/utility.hpp
)

set(TESTS_SHADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert
)

foreach(SHADER ${TESTS_SHADERS})
  get_filename_component(FILE_NAME ${SHADER} NAME)
  set(SPV ${CMAKE_BINARY_DIR}/shaders/${FILE_NAME}.spv)

  add_custom_command(OUTPUT ${SPV}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/shaders
    COMMAND ${GLSLC} -o ${SPV} ${SHADER}
    DEPENDS ${SHADER}
    COMMENT "compiling ${SHADER}..."
  )

  list(APPEND SPVS ${SPV})
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPVS})
add_executable(test ${TESTS_SOURCES})
add_dependencies(test shaders)

target_link_libraries(test PRIVATE
  groot
  Catch2::Catch2
)