find_package(Vulkan REQUIRED)
find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

add_subdirectory(src/gfx)

add_library(MoltenCore "src/engine.cpp" "src/image.cpp" "include/image.h")

target_link_libraries(MoltenCore PUBLIC MoltenGfx)

target_include_directories(MoltenCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(MoltenCore PUBLIC ${CMAKE_SOURCE_DIR}/third_party/glm-1.0)
target_include_directories(MoltenCore PUBLIC ${CMAKE_SOURCE_DIR}/third_party/stb_image)
target_include_directories(MoltenCore PUBLIC ${CMAKE_SOURCE_DIR}/third_party/fastgltf-0.7.1/include)

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