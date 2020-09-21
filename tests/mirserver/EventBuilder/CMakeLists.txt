set(
  EVENT_BUILDER_TEST_SOURCES
  eventbuilder_test.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
  ${CMAKE_SOURCE_DIR}/src/common
)

include_directories(
  SYSTEM
  ${MIRSERVER_INCLUDE_DIRS}
)

add_executable(EventBuilderTest ${EVENT_BUILDER_TEST_SOURCES})

target_link_libraries(
  EventBuilderTest
  qtmirserver
  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(EventBuilder, EventBuilderTest)
