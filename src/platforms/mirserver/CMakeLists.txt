# Qt5PlatformSupport is not available as a cmake module.
# And also not anymore as pkgconfig module since Qt 5.6.
#pkg_check_modules(QT5PLATFORM_SUPPORT Qt5PlatformSupport REQUIRED)
include(FindQt5PlatformSupport)

set(QTMIRSERVER_ABI 1)

add_subdirectory(miral)

# Hacks for the QPA privates monster.
pkg_check_modules(FONTCONFIG fontconfig REQUIRED)
add_definitions(-DQ_FONTCONFIGDATABASE)

if (QGL_DEBUG)
    message(STATUS "Qt's OpenGL debug logging enabled.")
    add_definitions(-DQGL_DEBUG)
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/common
)

include_directories(
    SYSTEM
    ${MIRAL_INCLUDE_DIRS}
    ${MIRCOMMON_INCLUDE_DIRS}
    ${MIRSERVER_INCLUDE_DIRS}
    ${MIRRENDERERGLDEV_INCLUDE_DIRS}

    ${URL_DISPATCHER_INCLUDE_DIRS}
    ${EGL_INCLUDE_DIRS}
    ${LTTNG_INCLUDE_DIRS}

    ${Qt5PlatformSupport_INCLUDE_DIRS}

    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}

    ${Qt5Quick_INCLUDE_DIRS}
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS}

    ${APPLICATION_API_INCLUDE_DIRS}

    ${CONTENT_HUB_INCLUDE_DIRS}
)

# We have to remove -pedantic for tracepoints.c
string (REPLACE " -pedantic " " " CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
# Needed to compile tracepoints in C99 mode.
add_definitions(-DBYTE_ORDER=__BYTE_ORDER)

# These files will compile without mirserver-dev
add_library(qtmirserver-nomirserver OBJECT
    ${CMAKE_SOURCE_DIR}/src/common/timestamp.cpp
    logging.cpp
    shelluuid.cpp
    ubuntutheme.cpp
    clipboard.cpp
    openglcontextfactory.cpp openglcontextfactory.h
    initialsurfacesizes.cpp initialsurfacesizes.h
    mircursorimages.cpp
    displayconfigurationpolicy.cpp
    mirsingleton.cpp
    promptsessionlistener.cpp
    mirserverstatuslistener.cpp
    screenscontroller.cpp
    nativeinterface.cpp
    qtcompositor.cpp
    services.cpp
    windowcontroller.cpp
    wrappedwindowmanagementpolicy.cpp
    wrappedsessionauthorizer.cpp
    mirserverhooks.cpp mirserverhooks.h
    setqtcompositor.cpp setqtcompositor.h
    displayconfigurationstorage.cpp
    tracepoints.c
# We need to run moc on these headers
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/Mir.h
    ${CMAKE_SOURCE_DIR}/src/common/windowcontrollerinterface.h

    ${CMAKE_SOURCE_DIR}/include/qtmir/displayconfigurationpolicy.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/sessionauthorizer.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/windowmanagementpolicy.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/windowmodelnotifier.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/appnotifier.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/displayconfigurationstorage.h
)
set_source_files_properties(tracepoints.c PROPERTIES COMPILE_FLAGS "${CMAKE_CFLAGS} -fPIC")

include(UseLttngGenTp)
add_lttng_gen_tp(NAME tracepoints)

set(QTMIRSERVER_PLUGIN_SRC
    ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
    cursor.cpp
    eventbuilder.cpp
    guiserverapplication.cpp
    inputdeviceobserver.cpp
    miropenglcontext.cpp
    mirserverintegration.cpp
    offscreensurface.cpp
    promptsessionmanager.cpp promptsessionmanager.h promptsession.h
    qmirserver.cpp
    qmirserver_p.cpp
    qteventfeeder.cpp
    screen.cpp
    screensmodel.cpp
    screenwindow.cpp
    surfaceobserver.cpp
    $<TARGET_OBJECTS:miral-prototypes>

# We need to run moc on these headers
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/MirMousePointerInterface.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/guiserverapplication.h
)

add_library(qtmirserver SHARED
    ${QTMIRSERVER_PLUGIN_SRC}
    $<TARGET_OBJECTS:qtmirserver-nomirserver>
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
        ${Qt5PlatformSupport_LIBRARIES}
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
    ubuntutheme.cpp
)

target_link_libraries(qpa-mirserver
PRIVATE
    qtmirserver
    ${Qt5PlatformSupport_LIBRARIES}

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
)

install(TARGETS qpa-mirserver LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/qt5/plugins/platforms")
