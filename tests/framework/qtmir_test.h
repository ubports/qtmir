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

#include <QSharedPointer>

#include <Unity/Application/application.h>
#include <Unity/Application/application_manager.h>
#include <Unity/Application/mirsurfacemanager.h>
#include <Unity/Application/sessionmanager.h>
#include <Unity/Application/session_interface.h>
#include <Unity/Application/sharedwakelock.h>
#include <Unity/Application/proc_info.h>
#include <mirserver.h>

#include "mock_proc_info.h"
#include "mock_mir_session.h"
#include "mock_prompt_session_manager.h"
#include "mock_prompt_session.h"
#include "mock_shared_wakelock.h"
#include "mock_settings.h"
#include "mock_task_controller.h"

namespace ms = mir::scene;
using namespace qtmir;

namespace qtmir {

typedef testing::NiceMock<mir::scene::MockPromptSessionManager> StubPromptSessionManager;

// For better output in ASSERT_* and EXPECT_* error messages
void PrintTo(const Application::InternalState& state, ::std::ostream* os);
void PrintTo(const SessionInterface::State& state, ::std::ostream* os);

} // namespace qtmir

namespace testing {

class QtMirTest : public ::testing::Test
{
public:
    QtMirTest();
    virtual ~QtMirTest();

    Application* startApplication(pid_t procId, QString const& appId);

    QSharedPointer<qtmir::TaskController> taskControllerSharedPointer{new testing::NiceMock<qtmir::MockTaskController>};
    testing::NiceMock<qtmir::MockTaskController> *taskController{static_cast<testing::NiceMock<qtmir::MockTaskController>*>(taskControllerSharedPointer.data())};
    testing::NiceMock<MockProcInfo> procInfo;
    testing::NiceMock<MockSharedWakelock> sharedWakelock;
    testing::NiceMock<MockSettings> settings;
    std::shared_ptr<StubPromptSessionManager> promptSessionManager;
    QSharedPointer<MirServer> mirServer;

    mir::shell::Shell *mirShell{nullptr};
    ApplicationManager applicationManager;
    SessionManager sessionManager;
    MirSurfaceManager surfaceManager;
};
} // namespace testing

#endif // QT_MIR_TEST_FRAMEWORK_H
