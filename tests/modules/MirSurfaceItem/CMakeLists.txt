set(
  MIR_SURFACE_ITEM_TEST_SOURCES
  mirsurfaceitem_test.cpp
  ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/modules
  ${CMAKE_SOURCE_DIR}/tests/framework
  ${MIRSERVER_INCLUDE_DIRS}
)

add_executable(mirsurfaceitem_test ${MIR_SURFACE_ITEM_TEST_SOURCES})

add_dependencies(mirsurfaceitem_test qtmir-test-framework-static)

target_link_libraries(
  mirsurfaceitem_test

  unityapplicationplugin

  Qt5::Test

  -L${CMAKE_BINARY_DIR}/tests/framework
  qtmir-test-framework-static

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(MirSurfaceItem mirsurfaceitem_test)
