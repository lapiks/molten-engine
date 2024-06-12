cmake_minimum_required(VERSION 3.5)
project(
    molten-engine
    VERSION 0.1.0
    LANGUAGES CXX C
)

find_package(Vulkan REQUIRED)

add_subdirectory(third_party)
add_subdirectory(molten-core)
add_subdirectory(molten-editor)
add_subdirectory(molten-runtime)
