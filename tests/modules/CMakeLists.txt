add_subdirectory(Application)
add_subdirectory(ApplicationManager)
add_subdirectory(General)
add_subdirectory(SharedWakelock)
add_subdirectory(SessionManager)
add_subdirectory(SurfaceManager)

#####
# What lies below was copied from src/modules/Unity/Application/CMakeLists.txt with minor modifications
# (essentially stripping out the lttng-related parts)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
    ${CMAKE_SOURCE_DIR}/src/common
)

include_directories(
    SYSTEM
    ${GLIB_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    ${GIO_UNIX_INCLUDE_DIRS}
    ${MIRSERVER_INCLUDE_DIRS}
    ${MIRRENDERERGLDEV_INCLUDE_DIRS}
    ${PROCESS_CPP_INCLUDE_DIRS}
    ${UBUNTU_PLATFORM_API_INCLUDE_DIRS}
    ${UBUNTU_APP_LAUNCH_INCLUDE_DIRS}
    ${GSETTINGS_QT_INCLUDE_DIRS}
    ${CGMANAGER_INCLUDE_DIRS}

    ${LTTNG_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Qml_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS}
)

set(QMLMIRPLUGIN_SRC
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/application_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/application.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/cgmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/common/abstractdbusservicemonitor.cpp
    ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/dbusfocusinfo.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/plugin.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/mirsurfacemanager.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/mirfocuscontroller.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/mirsurface.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/mirsurfaceinterface.h
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/mirsurfaceitem.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/mirsurfacelistmodel.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/mirbuffersgtexture.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/proc_info.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/session.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/sessionmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/sharedwakelock.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/upstart/applicationinfo.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/upstart/taskcontroller.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/timer.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/timesource.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/settings.cpp
# We need to run moc on these headers
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/ApplicationInfoInterface.h
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/ApplicationManagerInterface.h
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/Mir.h
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/MirFocusControllerInterface.h
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/MirSurfaceInterface.h
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/MirSurfaceItemInterface.h
    ${APPLICATION_API_INCLUDEDIR}/unity/shell/application/MirSurfaceListInterface.h
# Feed the automoc monster
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/session_interface.h
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/applicationinfo.h
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/taskcontroller.h
    ${CMAKE_SOURCE_DIR}/src/modules/Unity/Application/settings_interface.h
    )

add_library(unityapplicationplugin-nolttng SHARED
    ${QMLMIRPLUGIN_SRC}
)

target_link_libraries(
    unityapplicationplugin-nolttng

    ${CMAKE_THREAD_LIBS_INIT}

    ${GLIB_LDFLAGS}
    ${UBUNTU_PLATFORM_API_LDFLAGS}
    ${MIRSERVER_LDFLAGS}
    ${PROCESS_CPP_LDFLAGS}
    ${UBUNTU_APP_LAUNCH_LDFLAGS}
    ${GSETTINGS_QT_LDFLAGS}

    ${GL_LIBRARIES}

    Qt5::Core
    Qt5::DBus
    Qt5::Qml
    Qt5::Quick

    qpa-mirserver-nolttng
)
