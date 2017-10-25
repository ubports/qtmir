# Qt5PlatformSupport is not available as a cmake module.
# And also not anymore as pkgconfig module since Qt 5.6.
#pkg_check_modules(QT5PLATFORM_SUPPORT Qt5PlatformSupport REQUIRED)

# Hacks for the QPA privates monster.
pkg_check_modules(FONTCONFIG fontconfig REQUIRED)

if (QGL_DEBUG)
    message(STATUS "Qt's OpenGL debug logging enabled.")
    add_definitions(-DQGL_DEBUG)
endif()

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -fPIC")

if(Qt5Core_VERSION VERSION_LESS "5.8.0")
    add_definitions(-DQ_FONTCONFIGDATABASE)
    find_package(Qt5PlatformSupport REQUIRED)
    set(QT5PLATFORMSUPPORT_LIBS Qt5PlatformSupport::Qt5PlatformSupport)
else()
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
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/src/common

)

include_directories(
  SYSTEM
    ${MIRCOMMON_INCLUDE_DIRS}
    ${MIRSERVER_INCLUDE_DIRS}
    ${MIRRENDERERGLDEV_INCLUDE_DIRS}

    ${URL_DISPATCHER_INCLUDE_DIRS}
    ${EGL_INCLUDE_DIRS}
    ${LTTNG_INCLUDE_DIRS}

    ${QT5PLATFORM_SUPPORT_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}

    ${Qt5Quick_PRIVATE_INCLUDE_DIRS}

    ${APPLICATION_API_INCLUDE_DIRS}

    ${CONTENT_HUB_INCLUDE_DIRS}
)

# We have to remove -pedantic for tracepoints.c
string (REPLACE " -pedantic " " " CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
# Needed to compile tracepoints in C99 mode.
add_definitions(-DBYTE_ORDER=__BYTE_ORDER)

set(MIRSERVER_QPA_PLUGIN_SRC
    ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
    ${CMAKE_SOURCE_DIR}/src/common/timestamp.cpp
    cursor.cpp
    eventbuilder.cpp
    logging.cpp
    mircursorimages.cpp
    mirdisplayconfigurationpolicy.cpp
    mirwindowmanager.cpp
    mirsingleton.cpp
    qteventfeeder.cpp
    plugin.cpp
    qmirserver.cpp
    qmirserver_p.cpp
    sessionauthorizer.cpp
    sessionlistener.cpp
    shelluuid.cpp
    surfaceobserver.cpp
    promptsessionlistener.cpp
    mirserver.cpp
    mirserverstatuslistener.cpp
    screen.cpp
    screenwindow.cpp
    screenscontroller.cpp
    screensmodel.cpp
    mirserverintegration.cpp
    miropenglcontext.cpp
    nativeinterface.cpp
    offscreensurface.cpp
    qtcompositor.cpp
    services.cpp
    ubuntutheme.cpp
    clipboard.cpp
    creationhints.cpp
    tracepoints.c
# We need to run moc on these headers
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/Mir.h
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/MirMousePointerInterface.h
    )

add_library(qpa-mirserver SHARED
    ${MIRSERVER_QPA_PLUGIN_SRC}
)

target_link_libraries(
    qpa-mirserver
    ${QT5PLATFORMSUPPORT_LIBS}

    ${MIRSERVER_LDFLAGS}
    ${MIRCLIENT_LDFLAGS}
    ${URL_DISPATCHER_LDFLAGS}
    ${EGL_LDFLAGS}
    ${GL_LIBRARIES}
    ${LTTNG_LIBRARIES}

    ${QT5PLATFORM_SUPPORT_LDFLAGS}
    # TODO Qt5Platform support LDFLAGS dont provide actual required ldflags...
    # I found these were needed...perhaps there is some way to query qmake/qconfig?
    -lfreetype
    ${GIO_LDFLAGS}
    ${FONTCONFIG_LDFLAGS}
    ${XKBCOMMON_LIBRARIES}

    ${CONTENT_HUB_LIBRARIES}

    Qt5::Core
    Qt5::DBus
    Qt5::Quick
    Qt5::Sensors
)

include(UseLttngGenTp)
add_lttng_gen_tp(NAME tracepoints)

install(TARGETS qpa-mirserver LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/qt5/plugins/platforms")
