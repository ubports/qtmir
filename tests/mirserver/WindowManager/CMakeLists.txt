include_directories(
  ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
)

include_directories(
  SYSTEM
  ${MIRSERVER_INCLUDE_DIRS}
)

add_executable(WindowManagerTest
  window_manager.cpp
  stub_surface.cpp
  stub_session.cpp
)

target_link_libraries(
  WindowManagerTest

  Qt5::Test

  qpa-mirserver-nolttng

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(WindowManager, WindowManagerTest)
