set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fPIC")

add_library(experimentalwindowmanager STATIC
  server_example_basic_window_manager.cpp
  server_example_basic_window_manager.h
  server_example_window_management_info.cpp
  server_example_window_management_info.h
)


if (EXAMPLE_MIR_WINDOW_MANAGERS)

add_library(examplewindowmanager STATIC
  server_example_canonical_window_manager.h
  server_example_tiling_window_manager.h
  server_example_window_management.h
  server_example_canonical_window_manager.cpp
  server_example_tiling_window_manager.cpp
  server_example_window_management.cpp
)

endif()
