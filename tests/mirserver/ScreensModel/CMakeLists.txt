set(
  SCREENCONTROLLER_TEST_SOURCES
  screensmodel_test.cpp
  ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
  # to be moc-ed
  stub_screen.h
  testable_screensmodel.h
)

include_directories(
  ${CMAKE_SOURCE_DIR}/tests/framework
  ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
  ${CMAKE_SOURCE_DIR}/src/common
)

include_directories(
  SYSTEM
  ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
  ${MIRSERVER_INCLUDE_DIRS}
  ${MIRRENDERERGLDEV_INCLUDE_DIRS}
  ${MIRTEST_INCLUDE_DIRS}
)

add_executable(ScreensModelTest ${SCREENCONTROLLER_TEST_SOURCES})

set_property(TARGET ScreensModelTest PROPERTY CXX_STANDARD 14)

target_link_libraries(
  ScreensModelTest
  qtmirserver

  -L${CMAKE_BINARY_DIR}/tests/framework
  qtmir-test-framework-static

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(ScreensModel, ScreensModelTest)
