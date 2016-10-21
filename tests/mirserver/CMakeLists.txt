add_subdirectory(ArgvHelper)
add_subdirectory(EventBuilder)
add_subdirectory(QtEventFeeder)
add_subdirectory(Screen)
add_subdirectory(ScreensModel)
add_subdirectory(WindowManager)


#####
# What lies below was copied from src/platforms/mirserver/CMakeLists.txt with minor modifications
# (essentially stripping out the lttng-related parts)

# Hacks for the QPA privates monster.
pkg_check_modules(FONTCONFIG fontconfig REQUIRED)
add_definitions(-DQ_FONTCONFIGDATABASE)

# Dirty hack: The private QtPlatformSupport include dirs are not made available via
# a cmake module or the pkgconfig file (which only exposes the necessary linker flags)
# So we generate the paths ourselves from the Qt5Gui private dirs!
set(QT5_PLATFORMSUPPORT_INCLUDE_DIRS)
foreach(item ${Qt5Gui_PRIVATE_INCLUDE_DIRS})
  set(newitem "")
   string(REGEX REPLACE "QtGui" "QtPlatformSupport" newitem ${item})
   list(APPEND QT5_PLATFORMSUPPORT_INCLUDE_DIRS ${newitem})
endforeach(item ${Qt5Gui_PRIVATE_INCLUDE_DIRS})

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
    ${QT5_PLATFORMSUPPORT_INCLUDE_DIRS}
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS}

    ${APPLICATION_API_INCLUDE_DIRS}
)

set(MIRSERVER_QPA_PLUGIN_SRC
    ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
    ${CMAKE_SOURCE_DIR}/src/common/timestamp.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/cursor.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/logging.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/mircursorimages.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/mirdisplayconfigurationpolicy.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/mirwindowmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/mirsingleton.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/qteventfeeder.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/plugin.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/qmirserver.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/qmirserver_p.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/sessionauthorizer.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/sessionlistener.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/shelluuid.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/surfaceobserver.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/promptsessionlistener.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/mirserver.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/mirserverstatuslistener.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/screen.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/screenwindow.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/screenscontroller.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/screensmodel.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/mirserverintegration.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/miropenglcontext.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/nativeinterface.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/offscreensurface.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/qtcompositor.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/services.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/ubuntutheme.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/clipboard.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/creationhints.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver/eventbuilder.cpp
# We need to run moc on these headers
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/Mir.h
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/MirMousePointerInterface.h
    )

add_library(qpa-mirserver-nolttng SHARED
    ${MIRSERVER_QPA_PLUGIN_SRC}
)

target_link_libraries(
    qpa-mirserver-nolttng
    Qt5PlatformSupport

    ${MIRSERVER_LDFLAGS}
    ${MIRCLIENT_LDFLAGS}
    ${URL_DISPATCHER_LDFLAGS}
    ${EGL_LDFLAGS}
    ${GL_LIBRARIES}

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
