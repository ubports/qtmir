add_definitions(-DTEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

set(
  DESKTOP_FILE_READER_TEST_SOURCES
  desktopfilereader_test.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/modules
)

add_executable(desktop_file_reader_test ${DESKTOP_FILE_READER_TEST_SOURCES})

target_link_libraries(
  desktop_file_reader_test
  
  unityapplicationplugin

  ${GTEST_BOTH_LIBRARIES}
)

add_test(DesktopFileReader desktop_file_reader_test)
