set(
  APPLICATION_TEST_SOURCES
  application_test.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
  ${CMAKE_SOURCE_DIR}/src/modules
  ${CMAKE_SOURCE_DIR}/tests/framework
)

include_directories(
  SYSTEM
  ${MIRAL_INCLUDE_DIRS}
  ${MIRSERVER_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5GUI_INCLUDE_DIRS}
  ${Qt5Quick_INCLUDE_DIRS}
  ${Qt5DBus_INCLUDE_DIRS}
)

add_executable(application_test ${APPLICATION_TEST_SOURCES})

add_dependencies(application_test qtmir-test-framework-static)

target_link_libraries(
  application_test

  Qt5::Test

  unityapplicationplugin

  -L${CMAKE_BINARY_DIR}/tests/framework
  qtmir-test-framework-static

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(Application, application_test)
