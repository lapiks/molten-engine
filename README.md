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
git clone --recurse-submodules https://github.com/lapiks/molten-engine.git
cmake -S molten-engine -B build
cmake --build build 
```

## Dependencies
- SDL2 for windowing, input, sound...
- GLM for linear algebra
- GLAD for loading OpenGL functions
- vk-bootstrap for simplifying the initialization of Vulkan
