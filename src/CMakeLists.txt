project(GrootEngine::Engine)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

set(ENGINE_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/include/allocator.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/engine.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/log.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/rid.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan_context.hpp
)

set(ENGINE_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/allocator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/engine.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/log.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/rid.cpp
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
)

target_link_libraries(groot PRIVATE
  Vulkan::Vulkan
  glfw
)