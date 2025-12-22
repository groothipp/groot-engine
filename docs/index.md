# Groot Engine

A small and simple Vulkan abstraction library meant to give access to GPU Rendering and Compute without any extra uneeded features of modern game engines.

## Features
- **Simple API** - Small and straightforward to use.
- **Resource Management** - Automatic resource lifetime tracking through the use of a resource ID system
- **Built in Abstractions** - No need to bring in vulkan types as the library is entirely self contained
- **GPU Memory Management** - Uses AMD's Vulkan Memory Allocator to efficiently manage GPU memory 
- **Input System** - Per-Frame tracked Keyboard and Mouse input state

## Getting Started
- [Installation](getting_started/installation.md)
- [Rendering an Object](getting_started/graphics.md)
- [Dispatching Compute Passes](getting_started/compute.md)
- [Keyboard and Mouse Input](getting_started/input.md)