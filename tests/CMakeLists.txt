find_package(GMock)

add_subdirectory(framework)
add_subdirectory(mirserver)
add_subdirectory(modules)

pkg_check_modules(MIRTEST mirtest>=0.24 REQUIRED)

add_test(
    NAME convert-to-gles
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/convert-to-gles.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
