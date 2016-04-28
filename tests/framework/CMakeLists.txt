include_directories(
    ${APPLICATION_API_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
    ${CMAKE_SOURCE_DIR}/src/modules
    ${MIRSERVER_INCLUDE_DIRS}
    ${MIRRENDERERGLDEV_INCLUDE_DIRS}

    ${Qt5Quick_INCLUDE_DIRS}
    ${Qt5DBus_INCLUDE_DIRS}
)

set(QTMIR_TEST_PRIVATE_SRC
    fake_application_info.cpp
    fake_mirsurface.cpp
    fake_session.cpp
    mock_application_info.cpp
    mock_display.cpp
    mock_display_configuration.cpp
    mock_gl_display_buffer.cpp
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
    stub_input_channel.cpp
    stub_scene_surface.cpp
    qtmir_test.cpp
)

add_library(qtmir-test-framework-static STATIC
    ${QTMIR_TEST_PRIVATE_SRC}
)

target_link_libraries(
    qtmir-test-framework-static

    ${GTEST_BOTH_LIBRARIES}
    ${GMOCK_LIBRARIES}
)
