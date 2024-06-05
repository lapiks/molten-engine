cmake_minimum_required(VERSION 3.5)
project(
    molten-engine
    VERSION 0.1.0
    LANGUAGES CXX C
)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

find_package(Vulkan REQUIRED)
find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

add_subdirectory(third_party/SDL)
add_subdirectory(molten-gfx)

add_executable(MoltenEngine "src/main.cpp" )

include_directories(${CMAKE_SOURCE_DIR}/molten-gfx/include)

target_link_libraries(MoltenEngine PRIVATE SDL2::SDL2)
target_link_libraries(MoltenEngine PRIVATE MoltenGfx)

set_target_properties(MoltenEngine PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

if (MSVC)
    target_compile_options(MoltenEngine PRIVATE /W4)
else()
    target_compile_options(MoltenEngine PRIVATE -Wall -Wextra -pedantic)
endif()

if(XCODE)
    set_target_properties(MoltenEngine PROPERTIES
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