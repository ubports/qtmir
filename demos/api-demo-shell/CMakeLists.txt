set(API_DEMO_SHELL api-demo-shell)

include_directories(
    SYSTEM

    ${CMAKE_SOURCE_DIR}/include

    ${MIRSERVER_INCLUDE_DIRS}
    ${MIRAL_INCLUDE_DIRS}

    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Qml_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS}
)

add_executable(${API_DEMO_SHELL}
    main.cpp
    ../qml-demo-shell/pointerposition.cpp
)

target_link_libraries(${API_DEMO_SHELL}

    qtmirserver

    Qt5::Core
    Qt5::DBus
    Qt5::Qml
    Qt5::Quick
)

file(GLOB QML_JS_FILES *.qml *.js *.png)

add_custom_target(${API_DEMO_SHELL}-qmlfiles
                  SOURCES ${QML_JS_FILES})

# install binaries
install(TARGETS ${API_DEMO_SHELL}
     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
 )

install(FILES
    ${QML_JS_FILES}
    DESTINATION ${QTMIR_DATA_DIR}/${API_DEMO_SHELL}
)
