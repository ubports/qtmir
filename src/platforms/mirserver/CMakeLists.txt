# Qt5PlatformSupport is not available as a cmake module.
# And also not anymore as pkgconfig module since Qt 5.6.
#pkg_check_modules(QT5PLATFORM_SUPPORT Qt5PlatformSupport REQUIRED)

add_subdirectory(miral)
add_subdirectory(wayland-generated)

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
    ${CMAKE_SOURCE_DIR}/src/common
)

set(MIRCOMMON_INCLUDE_DIRS /usr/include/mircommon/)
set(MIRCOMMON_LDFLAGS "-lmircommon")

include_directories(
    SYSTEM
    ${MIRAL_INCLUDE_DIRS}
    ${MIRRENDERERGLDEV_INCLUDE_DIRS}
    ${MIRSERVER_INCLUDE_DIRS}
    ${MIRCOMMON_INCLUDE_DIRS}
    ${MIRWAYLAND_INCLUDE_DIRS}

    ${URL_DISPATCHER_INCLUDE_DIRS}
    ${EGL_INCLUDE_DIRS}
    ${LTTNG_INCLUDE_DIRS}

    ${QT5PLATFORM_SUPPORT_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}

    ${Qt5Quick_PRIVATE_INCLUDE_DIRS}

    ${APPLICATION_API_INCLUDE_DIRS}

if(WITH_CONTENTHUB)
    ${CONTENT_HUB_INCLUDE_DIRS}
endif()

    ${VALGRIND_INCLUDE_DIRS}
)

# Files the depend on mirserver-dev
set(MIRSERVER_DEPENDANTS
    eventbuilder.cpp
    eventdispatch.cpp
    inputdeviceobserver.cpp
    mircursorimages.cpp
    mirdisplayconfigurationpolicy.cpp
    miropenglcontext.cpp
    mirserverhooks.cpp
    mirserverintegration.cpp
    openglcontextfactory.cpp
    promptsessionmanager.cpp
    qmirserver.cpp
    qmirserver_p.cpp
    qtwindowmanager.cpp
    qteventfeeder.cpp
    screen.cpp
    screenscontroller.cpp
    screensmodel.cpp
    screenwindow.cpp
    setqtcompositor.cpp
    wayland-generated/qt_windowmanager_wrapper.cpp
    )

if(WITH_CONTENTHUB)
    set(CLIPBOARD_SRC clipboard.cpp)
endif()

add_library(qpa-mirserver SHARED
    ${MIRSERVER_DEPENDANTS}
    ${CLIPBOARD_SRC}
    cursor.cpp
    initialsurfacesizes.cpp
    inputdeviceobserver.cpp
    logging.cpp
    mirsingleton.cpp
    nativeinterface.cpp
    offscreensurface.cpp
    plugin.cpp
    promptsessionlistener.cpp
    qtcompositor.cpp
    services.cpp
    sessionauthorizer.cpp
    shelluuid.cpp
    surfaceobserver.cpp
    tracepoints.c
    urlbuilderdispatcher.cpp
    windowcontroller.cpp
    windowmanagementpolicy.cpp
    orientationsensor.cpp

    ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
    ${CMAKE_SOURCE_DIR}/src/common/timestamp.cpp

    # We need to run moc on these headers
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/Mir.h
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/MirMousePointerInterface.h
    ${CMAKE_SOURCE_DIR}/src/common/appnotifier.h
    ${CMAKE_SOURCE_DIR}/src/common/windowcontrollerinterface.h
    ${CMAKE_SOURCE_DIR}/src/common/windowmodelnotifier.h
    $<TARGET_OBJECTS:miral-prototypes>
    # We need to run moc on these headers
)

# We have to remove -pedantic for tracepoints.c
string (REPLACE " -pedantic " " " TRACEPOINTS_CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
# BYTE_ORDER needed to compile tracepoints in C99 mode.
set(TRACEPOINTS_CMAKE_C_FLAGS "${TRACEPOINTS_CMAKE_C_FLAGS} -DBYTE_ORDER=__BYTE_ORDER -fPIC ")
set_source_files_properties(tracepoints.c PROPERTIES COMPILE_FLAGS "${TRACEPOINTS_CMAKE_C_FLAGS}")

target_link_libraries(
    qpa-mirserver
    ${QT5PLATFORMSUPPORT_LIBS}

    ${MIRAL_LDFLAGS}
    ${MIRCOMMON_LDFLAGS}
    ${MIRWAYLAND_LDFLAGS}
    ${MIRSERVER_LDFLAGS}
    ${URL_DISPATCHER_LDFLAGS}
    ${WAYLAND_CLIENT_LIBRARIES}
    ${WAYLAND_SERVER_LDFLAGS}
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
