set(
  MIR_SURFACE_MANAGER_TEST_SOURCES
  mirsurfaceitem_test.cpp
  mirsurface_test.cpp
  ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/modules
  ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
  ${CMAKE_SOURCE_DIR}/tests/framework
)

include_directories(
  SYSTEM
  ${MIRSERVER_INCLUDE_DIRS}
  ${Qt5Quick_PRIVATE_INCLUDE_DIRS}
)

add_executable(surfacemanager_test ${MIR_SURFACE_MANAGER_TEST_SOURCES})

target_link_libraries(
  surfacemanager_test

  unityapplicationplugin-nolttng

  Qt5::Test

  -L${CMAKE_BINARY_DIR}/tests/framework
  qtmir-test-framework-static

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(SurfaceManager surfacemanager_test)
