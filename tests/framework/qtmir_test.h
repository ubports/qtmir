/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QT_MIR_TEST_FRAMEWORK_H
#define QT_MIR_TEST_FRAMEWORK_H

#include <memory>

#include <gtest/gtest.h>

#include <Unity/Application/application.h>
#include <Unity/Application/application_manager.h>
#include <Unity/Application/applicationcontroller.h>
#include <Unity/Application/mirsurfacemanager.h>
#include <Unity/Application/sessionmanager.h>
#include <Unity/Application/session_interface.h>
#include <Unity/Application/sharedwakelock.h>
#include <Unity/Application/taskcontroller.h>
#include <Unity/Application/proc_info.h>
#include <mirserver.h>

#include "mock_application_controller.h"
#include "mock_desktop_file_reader.h"
#include "mock_proc_info.h"
#include "mock_mir_session.h"
#include "mock_prompt_session_manager.h"
#include "mock_prompt_session.h"
#include "mock_shared_wakelock.h"
#include "mock_settings.h"

namespace ms = mir::scene;
using namespace qtmir;

namespace qtmir {

// For better output in ASSERT_* and EXPECT_* error messages
void PrintTo(const Application::InternalState& state, ::std::ostream* os);
void PrintTo(const SessionInterface::State& state, ::std::ostream* os);

// Initialization of mir::Server needed for by tests
class TestMirServerInit : virtual mir::Server
{
public:
    TestMirServerInit()
    {
        override_the_prompt_session_manager(
            [this]{ return the_mock_prompt_session_manager(); });
    }

    std::shared_ptr<mir::scene::MockPromptSessionManager> the_mock_prompt_session_manager()
    {
        return mock_prompt_session_manager;
    }

private:
    typedef testing::NiceMock<mir::scene::MockPromptSessionManager> StubPromptSessionManager;
    std::shared_ptr<StubPromptSessionManager> const mock_prompt_session_manager
        {std::make_shared<StubPromptSessionManager>()};
};


namespace {  char const* argv[] = { nullptr }; }

class FakeMirServer: private TestMirServerInit, public MirServer
{
public:
    FakeMirServer()
    : MirServer(0, argv)
    {
    }

    using TestMirServerInit::the_mock_prompt_session_manager;
};

} // namespace qtmir

namespace testing {

class QtMirTest : public ::testing::Test
{
public:
    QtMirTest();
    virtual ~QtMirTest();

    Application* startApplication(quint64 procId, QString const& appId);

    testing::NiceMock<MockApplicationController> appController;
    testing::NiceMock<MockProcInfo> procInfo;
    testing::NiceMock<MockDesktopFileReaderFactory> desktopFileReaderFactory;
    testing::NiceMock<MockSharedWakelock> sharedWakelock;
    testing::NiceMock<MockSettings> settings;
    QSharedPointer<FakeMirServer> mirServer;
    MirShell *mirShell{nullptr};
    QSharedPointer<TaskController> taskController;
    ApplicationManager applicationManager;
    SessionManager sessionManager;
    MirSurfaceManager surfaceManager;
};
} // namespace testing

#endif // QT_MIR_TEST_FRAMEWORK_H
