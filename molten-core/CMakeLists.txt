project(
    molten-core
    VERSION 0.1.0
    LANGUAGES CXX C
)

add_library(MoltenCore STATIC "src/engine.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include/gfx)

set_target_properties(MoltenCore PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)