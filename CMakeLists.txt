cmake_minimum_required(VERSION 3.0...3.5)
project(
    molten-engine
    VERSION 0.1.0
    LANGUAGES CXX C
)

add_executable(MoltenEngine main.cpp)
add_subdirectory(third_party/SDL-release-2.30.3)
target_link_libraries(MoltenEngine PRIVATE SDL2::SDL2)

set_target_properties(MoltenEngine PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

if (MSVC)
    target_compile_options(MoltenEngine PRIVATE /W4)
else()
    target_compile_options(MoltenEngine PRIVATE -Wall -Wextra -pedantic)
endif()

if(XCODE)
    set_target_properties(App PROPERTIES
        XCODE_GENERATE_SCHEME ON
        XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal")
endif()

if (WIN32)
    add_custom_command(
        TARGET MoltenEngine POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL2::SDL2>" "$<TARGET_FILE_DIR:MoltenEngine>"
        VERBATIM
    )
endif()