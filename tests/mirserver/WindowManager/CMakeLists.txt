include_directories(
  ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
  ${MIRSERVER_INCLUDE_DIRS}
)

add_executable(WindowManagerTest
  window_manager.cpp
)

target_link_libraries(
  WindowManagerTest
  qpa-mirserver

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(WindowManager, WindowManagerTest)
