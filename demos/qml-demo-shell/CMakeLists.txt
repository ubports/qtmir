set(DEMO_SHELL qtmir-demo-shell)

include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

add_executable(${DEMO_SHELL}
    main.cpp
)

include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Qml_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS}
)

target_link_libraries(
    ${DEMO_SHELL}
    Qt5::Core
    Qt5::DBus
    Qt5::Qml
    Qt5::Quick
)

file(GLOB QML_JS_FILES *.qml *.js *.png)

# install binaries
install(TARGETS ${DEMO_SHELL}
     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
 )

install(FILES
    ${QML_JS_FILES}
    DESTINATION ${QTMIR_DATA_DIR}/${DEMO_SHELL}
)
