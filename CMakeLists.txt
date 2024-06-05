cmake_minimum_required(VERSION 3.5)
project(
    molten-engine
    VERSION 0.1.0
    LANGUAGES CXX C
)

add_subdirectory(third_party/glad)
add_subdirectory(third_party/fastgltf-0.7.1)
add_subdirectory(third_party/vk-bootstrap)
add_subdirectory(third_party/SDL-release-2.30.3)
add_subdirectory(third_party/VulkanMemoryAllocator-3.1.0)
add_subdirectory(molten-core)
add_subdirectory(molten-editor)
add_subdirectory(molten-runtime)

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