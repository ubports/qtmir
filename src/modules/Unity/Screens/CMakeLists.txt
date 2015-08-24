set(SCREENSPLUGIN_SRC
    plugin.cpp
    screens.cpp
    )

add_library(unityscreensplugin SHARED
    ${SCREENSPLUGIN_SRC}
)

target_link_libraries(
    unityscreensplugin

    Qt5::Gui
    Qt5::Qml
)

# install
add_qml_plugin(Unity.Screens 0.1 Unity/Screens TARGETS unityscreensplugin)
