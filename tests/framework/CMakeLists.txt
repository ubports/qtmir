include_directories(
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
    ${CMAKE_SOURCE_DIR}/src/modules
)

include_directories(
    SYSTEM
    ${APPLICATION_API_INCLUDE_DIRS}
    ${MIRAL_INCLUDE_DIRS}
    ${MIRSERVER_INCLUDE_DIRS}
    ${MIRRENDERERGLDEV_INCLUDE_DIRS}
    ${MIRTEST_INCLUDE_DIRS}

    ${Qt5Quick_INCLUDE_DIRS}
    ${Qt5DBus_INCLUDE_DIRS}
)

set(QTMIR_TEST_PRIVATE_SRC
    fake_application_info.cpp
    fake_mirsurface.cpp
    fake_session.cpp
    mock_application_info.cpp
    mock_main_loop.cpp
    mock_mir_session.cpp
    mock_proc_info.cpp
    mock_prompt_session.cpp
    mock_prompt_session_manager.cpp
    mock_renderable.cpp
    mock_session.cpp
    mock_settings.cpp
    mock_shared_wakelock.cpp
    mock_surface.cpp
    mock_task_controller.cpp
    mock_persistent_surface_store.cpp
    stub_input_channel.cpp
    fake_surface.h
    fake_displayconfigurationoutput.h
    qtmir_test.cpp
)

add_library(qtmir-test-framework-static STATIC
    ${QTMIR_TEST_PRIVATE_SRC}
)

target_link_libraries(
    qtmir-test-framework-static

    unityapplicationplugin

    ${GTEST_BOTH_LIBRARIES}
    ${GMOCK_LIBRARIES}
)
