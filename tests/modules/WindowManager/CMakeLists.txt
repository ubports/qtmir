set(
  MIR_WINDOW_MANAGER_TEST_SOURCES
#  mirsurfaceitem_test.cpp #FIXME - reinstate these tests when functionality there
  mirsurface_test.cpp
  windowmodel_test.cpp
  ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/common
  ${CMAKE_SOURCE_DIR}/src/modules
  ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
  ${CMAKE_SOURCE_DIR}/tests/framework
)

include_directories(
  SYSTEM
  ${MIRAL_INCLUDE_DIRS}
  ${MIRTEST_INCLUDE_DIRS}
  ${MIRSERVER_INCLUDE_DIRS}
  ${Qt5Quick_PRIVATE_INCLUDE_DIRS}
)

add_executable(windowmanager_test ${MIR_WINDOW_MANAGER_TEST_SOURCES})

target_link_libraries(
  windowmanager_test

  unityapplicationplugin

  Qt5::Test

  -L${CMAKE_BINARY_DIR}/miral-qt/tests/framework
  qtmir-test-framework-static

  ${MIRAL_LDFLAGS}
  ${MIRTEST_LDFLAGS}
  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(WindowManager windowmanager_test)
