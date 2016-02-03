set(
  TASK_CONTROLLER_TEST_SOURCES
  taskcontroller_test.cpp
  ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
)

include_directories(
  ${APPLICATION_API_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/modules
  ${CMAKE_SOURCE_DIR}/tests/framework
  ${MIRSERVER_INCLUDE_DIRS}
)

add_executable(taskcontroller_test ${TASK_CONTROLLER_TEST_SOURCES})

add_dependencies(taskcontroller_test qtmir-test-framework-static)

target_link_libraries(
  taskcontroller_test

  unityapplicationplugin

  -L${CMAKE_BINARY_DIR}/tests/framework
  qtmir-test-framework-static

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(TaskController, taskcontroller_test)
