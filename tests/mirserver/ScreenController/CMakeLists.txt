set(
  SCREENCONTROLLER_TEST_SOURCES
  screencontroller_test.cpp
  ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
  # to be moc-ed
  stub_screen.h
  testable_screencontroller.h
)

include_directories(
  ${CMAKE_SOURCE_DIR}/tests/common
  ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
  ${CMAKE_SOURCE_DIR}/src/common
  ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
  ${MIRSERVER_INCLUDE_DIRS}
)

add_executable(ScreenControllerTest ${SCREENCONTROLLER_TEST_SOURCES})

target_link_libraries(
  ScreenControllerTest
  qpa-mirserver

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(ScreenController, ScreenControllerTest)
