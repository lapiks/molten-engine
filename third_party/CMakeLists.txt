find_package(Vulkan REQUIRED)

add_library(vkbootstrap STATIC)
add_library(glm INTERFACE)
add_library(vma INTERFACE)
add_library(stb_image INTERFACE)

add_subdirectory(glad)
add_subdirectory(fastgltf-0.7.1)
add_subdirectory(SDL-release-2.30.3)
add_subdirectory(VulkanMemoryAllocator-3.1.0)

target_sources(vkbootstrap PRIVATE 
  vkbootstrap/VkBootstrap.h
  vkbootstrap/VkBootstrap.cpp
  )
  
target_include_directories(vkbootstrap PUBLIC vkbootstrap)
target_link_libraries(vkbootstrap PUBLIC Vulkan::Vulkan $<$<BOOL:UNIX>:${CMAKE_DL_LIBS}>)

target_include_directories(vma INTERFACE VulkanMemoryAllocator-3.1.0)
target_include_directories(stb_image INTERFACE stb_image)
target_include_directories(glm INTERFACE glm-1.0)

add_library(imgui STATIC)

target_include_directories(imgui PUBLIC imgui-1.90.7)

target_sources(imgui PRIVATE 
  imgui-1.90.7/imgui.h
  imgui-1.90.7/imgui.cpp
  imgui-1.90.7/imgui_demo.cpp
  imgui-1.90.7/imgui_draw.cpp
  imgui-1.90.7/imgui_widgets.cpp
  imgui-1.90.7/imgui_tables.cpp
  imgui-1.90.7/backends/imgui_impl_opengl3.cpp
  imgui-1.90.7/backends/imgui_impl_vulkan.cpp
  imgui-1.90.7/backends/imgui_impl_sdl2.cpp
  )

target_link_libraries(imgui PUBLIC Vulkan::Vulkan SDL2::SDL2)
