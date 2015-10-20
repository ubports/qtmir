set(
  MIR_SURFACE_MANAGER_TEST_SOURCES
  mirsurfaceitem_test.cpp
  mirsurface_test.cpp
  ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
  ${CMAKE_SOURCE_DIR}/tests/modules/common/fake_mirsurface.h
  ${CMAKE_SOURCE_DIR}/tests/modules/common/fake_session.h
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/modules
  ${CMAKE_SOURCE_DIR}/tests/modules/common
  ${MIRSERVER_INCLUDE_DIRS}
)

add_executable(surfacemanager_test ${MIR_SURFACE_MANAGER_TEST_SOURCES})

target_link_libraries(
  surfacemanager_test

  qpa-mirserver
  unityapplicationplugin

  Qt5::Test

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(SurfaceManager surfacemanager_test)
