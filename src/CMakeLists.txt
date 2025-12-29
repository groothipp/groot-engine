project(GrootEngine::Engine)

find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)
find_package(glfw3 REQUIRED)

# Find shaderc dependencies
find_library(GLSLANG_LIB NAMES glslang REQUIRED)
find_library(SPIRV_LIB NAMES SPIRV REQUIRED)
find_library(SPIRV_TOOLS_LIB NAMES SPIRV-Tools REQUIRED)
find_library(SPIRV_TOOLS_OPT_LIB NAMES SPIRV-Tools-opt REQUIRED)

# ImGui setup
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui)
set(IMGUI_SOURCES
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
  ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)
set(IMGUI_HEADERS
  ${IMGUI_DIR}/imgui.h
  ${IMGUI_DIR}/imconfig.h
  ${IMGUI_DIR}/backends/imgui_impl_glfw.h
  ${IMGUI_DIR}/backends/imgui_impl_vulkan.h
)

set(ENGINE_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/include/allocator.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/engine.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/enums.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/gui.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/input_manager.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/log.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/object.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/renderer.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/rid.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/shader_compiler.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/stb_image.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/structs.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/tiny_obj_loader.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan_context.hpp
)

set(ENGINE_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/allocator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/engine.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/gui.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/input_manager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/linalg.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/log.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/object.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/renderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/rid.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/shader_compiler.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/stb_image.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/structs.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tiny_obj_loader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vma.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan_context.cpp
  ${IMGUI_SOURCES}
)

set(PUBLIC_HEADERS
  ${CMAKE_SOURCE_DIR}/include/groot/engine.hpp
  ${CMAKE_SOURCE_DIR}/include/groot/enums.hpp
  ${CMAKE_SOURCE_DIR}/include/groot/groot.hpp
  ${CMAKE_SOURCE_DIR}/include/groot/gui.hpp
  ${CMAKE_SOURCE_DIR}/include/groot/linalg.hpp
  ${CMAKE_SOURCE_DIR}/include/groot/log.hpp
  ${CMAKE_SOURCE_DIR}/include/groot/object.hpp
  ${CMAKE_SOURCE_DIR}/include/groot/rid.hpp
  ${CMAKE_SOURCE_DIR}/include/groot/structs.hpp
)

set_source_files_properties(
  ${CMAKE_CURRENT_SOURCE_DIR}/vma.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-nullability-completeness"
)

add_library(groot ${ENGINE_SOURCES})

target_include_directories(groot PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${IMGUI_DIR}
  ${IMGUI_DIR}/backends
)

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
  Vulkan::shaderc_combined
  ${GLSLANG_LIB}
  ${SPIRV_LIB}
  ${SPIRV_TOOLS_LIB}
  ${SPIRV_TOOLS_OPT_LIB}
)

install(TARGETS groot
  EXPORT GrootEngineTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${PUBLIC_HEADERS}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/groot
)