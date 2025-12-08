cmake_minimum_required(VERSION 4.2)

option(GROOT_MAKE_TESTS "Compiles Groot Engine tests" OFF)

set(GROOT_VERSION_MAJOR 2)
set(GROOT_VERSION_MINOR 91)
set(GROOT_VERSION_PATCH 20)

project(GrootEngine
  VERSION ${GROOT_VERSION_MAJOR}.${GROOT_VERSION_MINOR}.${GROOT_VERSION_PATCH}
  LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)

add_subdirectory(${CMAKE_SOURCE_DIR}/src)

if (GROOT_MAKE_TESTS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
  add_subdirectory(${CMAKE_SOURCE_DIR}/tests)
endif()

install(EXPORT GrootEngineTargets
  FILE GrootEngineTargets.cmake
  NAMESPACE GrootEngine::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GrootEngine
)

configure_package_config_file(
  ${CMAKE_SOURCE_DIR}/GrootEngineConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/GrootEngineConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GrootEngine
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/GrootEngineConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/GrootEngineConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/GrootEngineConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GrootEngine
)
