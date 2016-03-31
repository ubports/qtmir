set(
  ARGVHELPER_TEST_SOURCES
  argvHelper_test.cpp
)

add_executable(ArgvHelperTest ${ARGVHELPER_TEST_SOURCES})

include_directories(
  ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
)

target_link_libraries(
  ArgvHelperTest

  ${GTEST_BOTH_LIBRARIES}
)

add_test(ArgvHelper, ArgvHelperTest)
