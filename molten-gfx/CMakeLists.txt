project(
    molten-gfx
    VERSION 0.1.0
    LANGUAGES CXX C
)

add_library(MoltenGfx STATIC "src/renderer.cpp"  "src/gl_renderer.cpp" "src/gl_utils.h")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/gfx)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/include)

add_subdirectory(third_party/glad)
target_link_libraries(MoltenGfx PRIVATE Glad)

set_target_properties(MoltenGfx PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)