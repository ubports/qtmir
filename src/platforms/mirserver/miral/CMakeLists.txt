include_directories(SYSTEM ${MIRSERVER_INCLUDE_DIRS} ${MIRRENDERERGLDEV_INCLUDE_DIRS})

add_library(miral-prototypes OBJECT
    edid.cpp edid.h
    persist_display_config.cpp persist_display_config.h
)
