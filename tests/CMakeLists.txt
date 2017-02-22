find_package(GMock)

pkg_check_modules(MIRTEST mirtest>=0.26 REQUIRED)

add_subdirectory(framework)
add_subdirectory(mirserver)
add_subdirectory(modules)

add_test(
    NAME convert-to-gles
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/convert-to-gles.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_test(
    NAME requires-provides
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/check-requires-provides.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
