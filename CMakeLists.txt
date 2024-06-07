cmake_minimum_required(VERSION 3.5)
project(
    molten-engine
    VERSION 0.1.0
    LANGUAGES CXX C
)

find_package(Vulkan REQUIRED)

add_subdirectory(third_party/glad)
add_subdirectory(third_party/fastgltf-0.7.1)
add_subdirectory(third_party/SDL-release-2.30.3)
add_subdirectory(third_party/VulkanMemoryAllocator-3.1.0)
add_subdirectory(molten-core)
add_subdirectory(molten-editor)
add_subdirectory(molten-runtime)

add_library(vkbootstrap STATIC)

target_sources(vkbootstrap PRIVATE 
  ${CMAKE_SOURCE_DIR}/third_party/vkbootstrap/VkBootstrap.h
  ${CMAKE_SOURCE_DIR}/third_party/vkbootstrap/VkBootstrap.cpp
  )

target_include_directories(vkbootstrap PUBLIC ${CMAKE_SOURCE_DIR}/third_party/vkbootstrap)
target_link_libraries(vkbootstrap PUBLIC Vulkan::Vulkan)

set_target_properties(MoltenCore PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

if (MSVC)
    target_compile_options(MoltenCore PRIVATE /W4)
else()
    target_compile_options(MoltenCore PRIVATE -Wall -Wextra -pedantic)
endif()