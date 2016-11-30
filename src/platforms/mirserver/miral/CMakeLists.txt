include_directories(SYSTEM ${MIRSERVER_INCLUDE_DIRS})

include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

add_library(miral-prototypes OBJECT
    persist_display_config.cpp persist_display_config.h
    display_configuration_policy.cpp ${CMAKE_SOURCE_DIR}/include/qtmir/miral/display_configuration_policy.h
)
