project(GrootEngine::Engine)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_library(SHADERC_LIB shaderc_combined REQUIRED)

set(ENGINE_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/include/allocator.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/engine.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/enums.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/log.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/rid.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/shader_compiler.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/stb_image.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/structs.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan_context.hpp
)

set(ENGINE_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/allocator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/engine.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/linalg.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/log.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/rid.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/shader_compiler.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/stb_image.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vma.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan_context.cpp
)

set_source_files_properties(
  ${CMAKE_CURRENT_SOURCE_DIR}/vma.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-nullability-completeness"
)

add_library(groot ${ENGINE_SOURCES})

target_include_directories(groot PRIVATE ${CMAKE_SOURCE_DIR})

target_compile_definitions(groot PRIVATE
  "GLFW_INCLUDE_NONE"
  "VULKAN_HPP_NO_CONSTRUCTORS"
  "GROOT_VERSION_MAJOR=${GROOT_VERSION_MAJOR}"
  "GROOT_VERSION_MINOR=${GROOT_VERSION_MINOR}"
  "GROOT_VERSION_PATCH=${GROOT_VERSION_PATCH}"
)

target_link_libraries(groot PRIVATE
  Vulkan::Vulkan
  glfw
  ${SHADERC_LIB}
)