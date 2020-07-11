find_package(GMock)

pkg_check_modules(GTEST gtest)
if(NOT GTEST_FOUND)
    message(WARNING "'gtest' not found with pkg-config, assuming v1.7.0")
    set(GTEST_VERSION "1.7.0")
endif()
string(REGEX MATCH
    "([0-9])\\.([0-9]+)\\.([0-9])" GTEST_VERSION_PARSED
    ${GTEST_VERSION})
if (GTEST_VERSION_PARSED)
    set(GTEST_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(GTEST_VERSION_MINOR ${CMAKE_MATCH_2})
    set(GTEST_VERSION_PATCH ${CMAKE_MATCH_3})
    message("Using GTest v${GTEST_VERSION_MAJOR}.${GTEST_VERSION_MINOR}.${GTEST_VERSION_PATCH}")
else()
    set(GTEST_VERSION_MAJOR 1)
    set(GTEST_VERSION_MINOR 8)
    set(GTEST_VERSION_PATCH 0)
    message(WARNING "Could not parse GTest version: ${GTEST_VERSION}, assuming v1.8.0 (or compatible) and hoping for the best")
endif()

add_definitions(-DGTEST_VERSION_MAJOR=${GTEST_VERSION_MAJOR})
add_definitions(-DGTEST_VERSION_MINOR=${GTEST_VERSION_MINOR})
add_definitions(-DGTEST_VERSION_PATCH=${GTEST_VERSION_PATCH})

include_directories(
    include
)

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
