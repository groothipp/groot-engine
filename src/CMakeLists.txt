project(GrootEngine::Library)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(PNG REQUIRED)

set(GROOT_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/include/allocator.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/engine.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/materials.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/objects.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/parsers.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/renderer.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/transform.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/vertex.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/vkcontext.hpp
)

set(GROOT_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/allocator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/engine.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/linalg.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/materials.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/objects.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/parsers.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/renderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/transform.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vertex.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vkcontext.cpp
)

add_library(groot SHARED ${GROOT_INCLUDES} ${GROOT_SOURCES})

target_include_directories(groot PUBLIC
  ${CMAKE_SOURCE_DIR}
  /usr/local/include
)

target_link_directories(groot PUBLIC
  /usr/local/lib
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" AND ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64")
  target_include_directories(groot PUBLIC /opt/homebrew/include)
  target_link_directories(groot PUBLIC /opt/homebrew/lib)
endif()

target_compile_definitions(groot PRIVATE
  "GLFW_INCLUDE_NONE"
  "VULKAN_HPP_NO_CONSTRUCTORS"
  "ge_engine_version=ge_make_version(${GE_VERSION_MAJOR}, ${GE_VERSION_MINOR}, ${GE_VERSION_PATCH})"
  "ge_vulkan_version=VK_MAKE_API_VERSION(1, 3, 296, 0)"
  "ge_timeout=1000000000ul"
  "ge_max_descriptors=1024"
)

target_link_libraries(groot PRIVATE
  Vulkan::Vulkan
  glfw
  PNG::PNG
)