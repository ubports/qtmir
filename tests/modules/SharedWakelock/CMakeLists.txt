set(
  SHARED_WAKELOCK_TEST_SOURCES
  sharedwakelock_test.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/modules
)

include_directories(
  SYSTEM
  ${QTDBUSTEST_INCLUDE_DIRS}
  ${QTDBUSMOCK_INCLUDE_DIRS}
)

add_executable(sharedwakelock_test ${SHARED_WAKELOCK_TEST_SOURCES})

target_link_libraries(
  sharedwakelock_test

  unityapplicationplugin-nolttng

  Qt5::Test

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
  ${QTDBUSTEST_LIBRARIES}
  ${QTDBUSMOCK_LIBRARIES}
)

add_test(SharedWakelock, sharedwakelock_test)
