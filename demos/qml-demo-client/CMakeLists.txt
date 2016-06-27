set(DEMO_CLIENT qtmir-demo-client)
configure_file(${DEMO_CLIENT}.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/${DEMO_CLIENT}.desktop @ONLY)

include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

add_executable(${DEMO_CLIENT}
    main.cpp
)

include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Qml_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS}
)

target_link_libraries(
    ${DEMO_CLIENT}
    Qt5::Core
    Qt5::DBus
    Qt5::Qml
    Qt5::Quick
)

file(GLOB QML_JS_FILES *.qml *.js *.png)

add_custom_target(${DEMO_CLIENT}-qmlfiles
                  SOURCES ${QML_JS_FILES})

# install binaries
install(TARGETS ${DEMO_CLIENT}
     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
 )

install(FILES
    ${QML_JS_FILES}
    DESTINATION ${QTMIR_DATA_DIR}/${DEMO_CLIENT}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${DEMO_CLIENT}.desktop
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications
)
