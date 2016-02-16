set(
  SESSION_MANAGER_TEST_SOURCES
  session_manager_test.cpp
  session_test.cpp
  ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
)

include_directories(
  ${APPLICATION_API_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
  ${CMAKE_SOURCE_DIR}/src/modules
  ${CMAKE_SOURCE_DIR}/tests/framework
  ${MIRSERVER_INCLUDE_DIRS}
)

add_executable(sessionmanager_test ${SESSION_MANAGER_TEST_SOURCES})

add_dependencies(sessionmanager_test qtmir-test-framework-static)

target_link_libraries(
  sessionmanager_test

  unityapplicationplugin

  Qt5::Test

  -L${CMAKE_BINARY_DIR}/tests/framework
  qtmir-test-framework-static

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(SessionManager, sessionmanager_test)
