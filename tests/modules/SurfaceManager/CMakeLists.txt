set(
  SURFACE_MANAGER_TEST_SOURCES
  surface_manager_test.cpp
  ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/common
  ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
  ${CMAKE_SOURCE_DIR}/src/modules
  ${CMAKE_SOURCE_DIR}/tests/framework
)

include_directories(
  SYSTEM
  ${APPLICATION_API_INCLUDE_DIRS}
  ${MIRAL_INCLUDE_DIRS}
  ${MIRTEST_INCLUDE_DIRS}
)

add_executable(surfacemanager_test ${SURFACE_MANAGER_TEST_SOURCES})

add_dependencies(surfacemanager_test qtmir-test-framework-static)

target_link_libraries(
  surfacemanager_test

  unityapplicationplugin

  Qt5::Test

  -L${CMAKE_BINARY_DIR}/tests/framework
  qtmir-test-framework-static

  ${MIRTEST_LDFLAGS}
  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(SurfaceManager, surfacemanager_test)
