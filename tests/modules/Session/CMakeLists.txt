set(
  SESSION_TEST_SOURCES
  taskcontroller_test.cpp
  session_test.cpp
  ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
  ${CMAKE_SOURCE_DIR}/src/modules
  ${CMAKE_SOURCE_DIR}/tests/framework
)

include_directories(
  SYSTEM
  ${APPLICATION_API_INCLUDE_DIRS}
  ${MIRAL_INCLUDE_DIRS}
  ${MIRSERVER_INCLUDE_DIRS}
)

add_executable(session_test ${SESSION_TEST_SOURCES})

add_dependencies(session_test qtmir-test-framework-static)

target_link_libraries(
  session_test

  unityapplicationplugin

  Qt5::Test

  -L${CMAKE_BINARY_DIR}/tests/framework
  qtmir-test-framework-static

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(Session, session_test)
