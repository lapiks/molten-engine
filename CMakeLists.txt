cmake_minimum_required(VERSION 3.5)
project(
    molten-engine
    VERSION 0.1.0
    LANGUAGES CXX C
)

find_package(Vulkan REQUIRED)
find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

add_subdirectory(third_party/SDL)
add_subdirectory(third_party/fastgltf)
add_subdirectory(molten-gfx)

add_executable(MoltenEngine "src/main.cpp" )

include_directories(${CMAKE_SOURCE_DIR}/molten-gfx/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/stb_image)

target_link_libraries(MoltenEngine PRIVATE SDL2::SDL2)
target_link_libraries(MoltenEngine PRIVATE fastgltf)
target_link_libraries(MoltenEngine PRIVATE MoltenGfx)

add_library(imgui STATIC)
target_include_directories(imgui PUBLIC third_party/imgui)
target_sources(imgui PRIVATE 
  third_party/imgui/imgui.h
  third_party/imgui/imgui.cpp
  third_party/imgui/imgui_demo.cpp
  third_party/imgui/imgui_draw.cpp
  third_party/imgui/imgui_widgets.cpp
  third_party/imgui/imgui_tables.cpp
  third_party/imgui/backends/imgui_impl_opengl3.cpp
  third_party/imgui/backends/imgui_impl_vulkan.cpp
  third_party/imgui/backends/imgui_impl_sdl2.cpp
)
target_link_libraries(imgui PUBLIC Vulkan::Vulkan SDL2::SDL2)

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