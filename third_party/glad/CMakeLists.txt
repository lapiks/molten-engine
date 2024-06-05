project(
    glad
    VERSION 0.1.0
    LANGUAGES C
)

add_library(Glad STATIC "src/glad.c" )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)