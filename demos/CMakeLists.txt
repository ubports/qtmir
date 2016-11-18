configure_file(paths.h.in ${CMAKE_CURRENT_BINARY_DIR}/paths.h @ONLY)

add_subdirectory(qml-demo-client)
add_subdirectory(qml-demo-shell)
add_subdirectory(api-demo-shell)
