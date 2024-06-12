add_executable(MoltenEditor "src/main.cpp" )

target_link_libraries(MoltenEditor PRIVATE MoltenCore SDL2::SDL2)

set_target_properties(MoltenEditor PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

if (WIN32)
    add_custom_command(
        TARGET MoltenEditor POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL2::SDL2>" "$<TARGET_FILE_DIR:MoltenEditor>"
        VERBATIM
    )
endif()