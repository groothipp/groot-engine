# Groot Engine Installation

> Groot Engine was configured using CMake and will assume you are using it as well to simplify things. If you are not using CMake, be sure to link to Vulkan, GLFW, ShaderC and its dependencies, and of course groot!

Make sure you have the following dependencies installed:

- **Vulkan**
- **GLFW**
- **ShaderC**
- **Catch2** (only if building tests)

## Building and Installing

1. Navigate to a directory you want groot engine to be in and clone the Groot Engine repository: `cd <dir> && git clone https://github.com/groothipp/groot-engine.git`

2. Generate the makefiles with CMake: `cmake . -B build`
> If you need `compile_commands.json` for your code intelligence, make sure to use `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`

3. Compile Groot Engine: `cmake --build build`

4. Once Groot Engine is built install it to your system with: `cmake --install build`
> If you want to change the install prefix from your native library and include directories, you can use `-DCMAKE_INSTALL_PREFIX=<path/to/dir>` during step 2

## Using Groot Engine

Installing GrootEngine to your system allows CMake to find the package automatically. When setting up your CMakeLists.txt for your project, add the following:

```
find_package(GrootEngine REQUIRED)

target_link_libraries(<target_name> PRIVATE GrootEngine::groot)
```