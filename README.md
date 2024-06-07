# Molten Engine
A work in progress C++ game engine.

## Goals
- OpenGL support
- Vulkan support with raytracing
- Editor
- Code hot-reload
- Customizable render pipeline

## Build
This project uses CMake as build system.
```
cmake . -B build
cmake --build build 
```

## Dependencies
- SDL2 for windowing, input, sound...
- stb_image for loading images
- GLM for linear algebra
- GLAD for loading OpenGL functions
- vk-bootstrap for simplifying the Vulkan initialization
- VulkanMemoryAllocator for gpu memory allocators
- fastgltf for gltf files loading
- ImGui for user interface
