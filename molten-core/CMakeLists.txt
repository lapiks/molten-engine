find_package(Vulkan REQUIRED)
find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

add_subdirectory(src/gfx)

add_library(MoltenCore 
  "src/engine.cpp" 
  "src/image.cpp" 
  "src/shader.cpp" 
  "src/deferred_voxel_renderer.cpp"
  "src/deferred_voxel_renderer.h"
  "src/transform_3d.cpp" 
  "src/shapes.h" 
  "include/image.h" 
  "include/shader.h"    
  "include/transform_3d.h" 
 "src/vox_scene.h" "src/vox_scene.cpp" "src/asset_manager.h" "src/asset_manager.cpp" "src/pool.h" "src/pool.cpp")

target_link_libraries(MoltenCore PUBLIC MoltenGfx stb_image glm ogt_vox)

target_include_directories(MoltenCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

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