add_executable(MoltenEditor "src/main.cpp" )

find_package(Vulkan REQUIRED)
find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

target_link_libraries(MoltenEditor PRIVATE SDL2::SDL2)
target_link_libraries(MoltenEditor PRIVATE MoltenCore)

add_library(imgui STATIC)
target_include_directories(imgui PUBLIC ${CMAKE_SOURCE_DIR}/third_party/imgui)
target_sources(imgui PRIVATE 
  ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui.h
  ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui.cpp
  ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_demo.cpp
  ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_draw.cpp
  ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_widgets.cpp
  ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_tables.cpp
  ${CMAKE_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_opengl3.cpp
  ${CMAKE_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_vulkan.cpp
  ${CMAKE_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_sdl2.cpp
)
target_link_libraries(imgui PUBLIC Vulkan::Vulkan SDL2::SDL2)

if (WIN32)
    add_custom_command(
        TARGET MoltenEditor POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL2::SDL2>" "$<TARGET_FILE_DIR:MoltenEditor>"
        VERBATIM
    )
endif()