find_package(Vulkan REQUIRED)
find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

add_subdirectory(src/gfx)

add_library(MoltenCore "src/engine.cpp" )

include_directories(${CMAKE_SOURCE_DIR}/third_party/stb_image)

target_link_libraries(MoltenCore PRIVATE fastgltf)