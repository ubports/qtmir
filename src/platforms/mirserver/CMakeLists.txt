# Qt5PlatformSupport is not available as a cmake module.
# And also not anymore as pkgconfig module since Qt 5.6.
#pkg_check_modules(QT5PLATFORM_SUPPORT Qt5PlatformSupport REQUIRED)
include(FindQt5PlatformSupport)
include(UseLttngGenTp)

set(QTMIRSERVER_ABI 1)

add_subdirectory(miral)

# Hacks for the QPA privates monster.
pkg_check_modules(FONTCONFIG fontconfig REQUIRED)

if (QGL_DEBUG)
    message(STATUS "Qt's OpenGL debug logging enabled.")
    add_definitions(-DQGL_DEBUG)
endif()

if (WITH_CONTENTHUB)
    add_definitions(-DWITH_CONTENTHUB)
endif()

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -fPIC")

find_package(Qt5FontDatabaseSupport REQUIRED)
find_package(Qt5ThemeSupport REQUIRED)
find_package(Qt5EventDispatcherSupport REQUIRED)
find_package(Qt5EglSupport REQUIRED)
find_package(Qt5ServiceSupport REQUIRED)
set(QT5PLATFORMSUPPORT_LIBS
    Qt5FontDatabaseSupport::Qt5FontDatabaseSupport
    Qt5ThemeSupport::Qt5ThemeSupport
    Qt5EventDispatcherSupport::Qt5EventDispatcherSupport
    Qt5EglSupport::Qt5EglSupport
    Qt5ServiceSupport::Qt5ServiceSupport
)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_BINARY_DIR} # for tracepoints.h
)

include_directories(
    SYSTEM
    ${MIRAL_INCLUDE_DIRS}
    ${MIRCOMMON_INCLUDE_DIRS}
    ${MIRSERVER_INCLUDE_DIRS}
    ${MIRRENDERERGLDEV_INCLUDE_DIRS}
    ${MIRSERVER_INCLUDE_DIRS}

    ${URL_DISPATCHER_INCLUDE_DIRS}
    ${EGL_INCLUDE_DIRS}
    ${LTTNG_INCLUDE_DIRS}

    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}


    ${Qt5Quick_INCLUDE_DIRS}
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS}
    ${Qt5Qml_INCLUDE_DIRS}

    ${APPLICATION_API_INCLUDE_DIRS}

if(WITH_CONTENTHUB)
    ${CONTENT_HUB_INCLUDE_DIRS}
endif()

    ${VALGRIND_INCLUDE_DIRS}
)

# We have to remove -pedantic for tracepoints.c
string (REPLACE " -pedantic " " " TRACEPOINTS_CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
# BYTE_ORDER needed to compile tracepoints in C99 mode.
set(TRACEPOINTS_CMAKE_C_FLAGS "${TRACEPOINTS_CMAKE_C_FLAGS} -DBYTE_ORDER=__BYTE_ORDER -fPIC ")
set_source_files_properties(tracepoints.c PROPERTIES COMPILE_FLAGS "${TRACEPOINTS_CMAKE_C_FLAGS}")

# Generate tracepoints.c and .h from tracepoints.tp
add_lttng_gen_tp(NAME tracepoints)
add_custom_target(MirServer_LTTNG
    DEPENDS tracepoints.h tracepoints.c
)

if(WITH_CONTENTHUB)
    set(CLIPBOARD_SRC clipboard.cpp)
endif()

# These files will compile without mirserver-dev
add_library(qtmirserver-nomirserver OBJECT
    ${CMAKE_SOURCE_DIR}/src/common/timestamp.cpp
    ${CLIPBOARD_SRC}
    logging.cpp
    shelluuid.cpp
    mirsingleton.cpp
    wrappedsessionauthorizer.cpp
    promptsessionlistener.cpp
    displayconfigurationstorage.cpp
    nativeinterface.cpp
    services.cpp
    windowcontroller.cpp
    workspacecontroller.cpp
    tracepoints.c
    surfaceobserver.cpp
    initialsurfacesizes.cpp
    orientationsensor.cpp
    screenadaptor.cpp
    screenadaptormodel.cpp
    qtmir.cpp
)
add_dependencies(qtmirserver-nomirserver MirServer_LTTNG)

include_directories(SYSTEM ${MIRSERVER_INCLUDE_DIRS})

# These files will only compile with mirserver-dev
add_library(qtmirserver-mirserver OBJECT
    openglcontextfactory.cpp openglcontextfactory.h
    mircursorimages.cpp
    screenscontroller.cpp
    qtcompositor.cpp
    displayconfigurationpolicy.cpp
    wrappedwindowmanagementpolicy.cpp
    mirdisplayconfigurationobserver.cpp
    mirserverhooks.cpp mirserverhooks.h
    setqtcompositor.cpp setqtcompositor.h
    eventdispatch.cpp eventdispatch.h
    promptsessionmanager.cpp promptsessionmanager.h promptsession.h
)

# These files get entangled by automoc so they need to go together. And some depend on mirserver-dev
set(QMIRSERVER_PLUGIN_SRC
    ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
    cursor.cpp
    eventbuilder.cpp
    qteventfeeder.cpp
    qmirserver.cpp
    qmirserver_p.cpp
    platformscreen.cpp
    screenplatformwindow.cpp
    screensmodel.cpp
    miropenglcontext.cpp
    offscreensurface.cpp
    inputdeviceobserver.cpp
    mirserverapplication.cpp
    windowmodelnotifier.cpp

# We need to run moc on these headers
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/MirMousePointerInterface.h
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/Mir.h

    ${CMAKE_SOURCE_DIR}/include/qtmir/mirserverapplication.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/displayconfigurationpolicy.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/sessionauthorizer.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/windowmanagementpolicy.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/displayconfigurationstorage.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/screen.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/screens.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/types.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/qtmir.h

    ${CMAKE_SOURCE_DIR}/src/common/windowcontrollerinterface.h
    ${CMAKE_SOURCE_DIR}/src/common/workspacecontrollerinterface.h
    ${CMAKE_SOURCE_DIR}/src/common/windowmodelnotifier.h
    ${CMAKE_SOURCE_DIR}/src/common/appnotifier.h
)

add_library(qtmirserver SHARED
    ${QMIRSERVER_PLUGIN_SRC}
    $<TARGET_OBJECTS:qtmirserver-nomirserver>
    $<TARGET_OBJECTS:qtmirserver-mirserver>
    $<TARGET_OBJECTS:miral-prototypes>
)

target_link_libraries(qtmirserver
    PUBLIC
        ${MIRAL_LDFLAGS}
        ${MIRSERVER_LDFLAGS}
        Qt5::Core
        Qt5::Gui

    PRIVATE
        ${URL_DISPATCHER_LDFLAGS}
        ${EGL_LDFLAGS}
        ${GL_LIBRARIES}
        ${LTTNG_LIBRARIES}
        ${GIO_LDFLAGS}
        ${FONTCONFIG_LDFLAGS}
        ${XKBCOMMON_LIBRARIES}

        ${CONTENT_HUB_LIBRARIES}

        Qt5::DBus
        Qt5::Quick
        Qt5::Sensors
        ${QT5PLATFORMSUPPORT_LIBS}
)

set_target_properties(qtmirserver
  PROPERTIES
  SOVERSION ${QTMIRSERVER_ABI}
)

set(LIBDIR "${CMAKE_INSTALL_FULL_LIBDIR}")
set(INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include/qtmir")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qtmirserver.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/qtmirserver.pc
    @ONLY
)

install(
  TARGETS qtmirserver
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
  DIRECTORY ${CMAKE_SOURCE_DIR}/include/qtmir
  DESTINATION "include/qtmir"
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/qtmirserver.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)


add_library(qpa-mirserver SHARED
    mirserverintegration.cpp
    plugin.cpp
)

target_link_libraries(qpa-mirserver
PRIVATE
    qtmirserver
    ${QT5PLATFORMSUPPORT_LIBS}

    ${MIRSERVER_LDFLAGS}
    ${URL_DISPATCHER_LDFLAGS}
    ${EGL_LDFLAGS}
    ${GL_LIBRARIES}
    ${LTTNG_LIBRARIES}
    ${GIO_LDFLAGS}
    ${FONTCONFIG_LDFLAGS}
    ${XKBCOMMON_LIBRARIES}

    ${CONTENT_HUB_LIBRARIES}

    Qt5::Core
    Qt5::DBus
    Qt5::Quick
    Qt5::Sensors
    Qt5::Qml
)

install(TARGETS qpa-mirserver LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/qt5/plugins/platforms")
