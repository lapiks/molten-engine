project(
    molten-gfx
    VERSION 0.1.0
    LANGUAGES CXX C
)

add_library(MoltenGfx STATIC "src/renderer.cpp"  "src/gl_renderer.cpp" "src/gl_utils.h" "src/vk_renderer.h" "src/vk_renderer.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/gfx)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/glm)

add_subdirectory(third_party/glad)
add_subdirectory(third_party/VulkanMemoryAllocator)

target_link_libraries(MoltenGfx PRIVATE Glad)
target_link_libraries(MoltenGfx PRIVATE GPUOpen::VulkanMemoryAllocator)

set_target_properties(MoltenGfx PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)