add_executable(GfxMultiRenderTargets "main.cpp" )

target_link_libraries(GfxMultiRenderTargets PRIVATE MoltenCore SDL2::SDL2)

set_target_properties(GfxMultiRenderTargets PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

if (WIN32)
    add_custom_command(
        TARGET GfxMultiRenderTargets POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL2::SDL2>" "$<TARGET_FILE_DIR:GfxMultiRenderTargets>"
        VERBATIM
    )
endif()

add_custom_command(TARGET GfxMultiRenderTargets POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_CURRENT_SOURCE_DIR}/assets/ $<TARGET_FILE_DIR:GfxMultiRenderTargets>/assets/)