set(
  GENERAL_TEST_SOURCES
  objectlistmodel_test.cpp
  timestamp_test.cpp
  ${CMAKE_SOURCE_DIR}/src/common/timestamp.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/common
  ${CMAKE_SOURCE_DIR}/src/modules
)

add_executable(general_test ${GENERAL_TEST_SOURCES})

target_link_libraries(
  general_test

  Qt5::Gui

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(General general_test)
