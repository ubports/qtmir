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
    ${Qt5Qml_INCLUDE_DIRS}

    ${APPLICATION_API_INCLUDE_DIRS}

    ${CONTENT_HUB_INCLUDE_DIRS}
)

# We have to remove -pedantic for tracepoints.c
string (REPLACE " -pedantic " " " CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
# Needed to compile tracepoints in C99 mode.
add_definitions(-DBYTE_ORDER=__BYTE_ORDER)

# These files will compile without mirserver-dev
add_library(qpa-mirserver-nomirserver OBJECT
    ${CMAKE_SOURCE_DIR}/src/common/timestamp.cpp
    logging.cpp
    plugin.cpp
    shelluuid.cpp
    ubuntutheme.cpp
    clipboard.cpp
    mirsingleton.cpp
    wrappedsessionauthorizer.cpp
    promptsessionlistener.cpp
    displayconfigurationstorage.cpp
    nativeinterface.cpp
    services.cpp
    windowcontroller.cpp
    tracepoints.c
    surfaceobserver.cpp
    initialsurfacesizes.cpp
)

set_source_files_properties(tracepoints.c PROPERTIES COMPILE_FLAGS "${CMAKE_CFLAGS} -fPIC")

include_directories(SYSTEM ${MIRSERVER_INCLUDE_DIRS})

# These files will only compile with mirserver-dev
add_library(qpa-mirserver-mirserver OBJECT
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
set(MIRSERVER_QPA_PLUGIN_SRC
    ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
    cursor.cpp
    eventbuilder.cpp
    qteventfeeder.cpp
    qmirserver.cpp
    qmirserver_p.cpp
    platformscreen.cpp
    screenplatformwindow.cpp
    screensmodel.cpp
    mirserverintegration.cpp
    miropenglcontext.cpp
    offscreensurface.cpp
    inputdeviceobserver.cpp
    mirserverapplication.cpp

# We need to run moc on these headers
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/MirMousePointerInterface.h
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/Mir.h

    ${CMAKE_SOURCE_DIR}/include/qtmir/mirserverapplication.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/displayconfigurationpolicy.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/sessionauthorizer.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/windowmanagementpolicy.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/windowmodelnotifier.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/appnotifier.h
    ${CMAKE_SOURCE_DIR}/include/qtmir/displayconfigurationstorage.h

    ${CMAKE_SOURCE_DIR}/src/common/windowcontrollerinterface.h
)

add_library(qtmirserver SHARED
    ${MIRSERVER_QPA_PLUGIN_SRC}
    $<TARGET_OBJECTS:qpa-mirserver-nomirserver>
    $<TARGET_OBJECTS:qpa-mirserver-mirserver>
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

include(UseLttngGenTp)
add_lttng_gen_tp(NAME tracepoints)

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
    Qt5::Qml
)

install(TARGETS qpa-mirserver LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/qt5/plugins/platforms")
