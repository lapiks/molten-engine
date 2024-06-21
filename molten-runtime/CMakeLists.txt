add_executable(MoltenRuntime "src/main.cpp" )

target_link_libraries(MoltenRuntime PRIVATE MoltenCore SDL2::SDL2)

set_target_properties(MoltenRuntime PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

if (WIN32)
    add_custom_command(
        TARGET MoltenRuntime POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL2::SDL2>" "$<TARGET_FILE_DIR:MoltenRuntime>"
        VERBATIM
    )
endif()

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)
add_dependencies(MoltenRuntime copy_assets)