set(
  URLBUILDERDISPATCHER_TEST_RESOURCES
  urlbuilderdispatcher_test.cpp
  ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/tests/framework
  ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
  ${CMAKE_SOURCE_DIR}/src/common
)

include_directories(
  SYSTEM
  ${QT5PLATFORM_SUPPORT_INCLUDE_DIRS}
  ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
  ${MIRSERVER_INCLUDE_DIRS}
)

add_executable(URLBuilderDispatcherTest ${URLBUILDERDISPATCHER_TEST_RESOURCES})

target_link_libraries(
  URLBuilderDispatcherTest
  qpa-mirserver

  ${QT5PLATFORMSUPPORT_LIBS}
  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(URLBuilderDispatcher, URLBuilderDispatcherTest)
