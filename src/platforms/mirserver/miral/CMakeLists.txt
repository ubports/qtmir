include_directories(SYSTEM ${MIRSERVER_INCLUDE_DIRS})

add_library(miral-prototypes OBJECT
    persist_display_config.cpp persist_display_config.h
)
