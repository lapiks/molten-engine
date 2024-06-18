# Molten Engine
A work in progress C++ voxel engine.

## Goals
- Voxel rendering with ray-tracing
- Each voxel model can have its own orientation
- Gameplay scripting using an ECS
- OpenGL support
- Vulkan support with raytracing
- Editor to edit scenes
- Gameplay code hot-reload

## Build
This project uses CMake as build system.
To build:
```
cmake . -B build
cmake --build build 
```

## Dependencies
- SDL2 for windowing, input, sound
- Flecs for ECS
- stb_image for loading images
- GLM for linear algebra
- GLAD for loading OpenGL functions
- vk-bootstrap for simplifying the Vulkan initialization
- VulkanMemoryAllocator for vulkan gpu memory allocators
- ImGui for user interface
