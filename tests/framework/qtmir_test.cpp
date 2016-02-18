/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include "qtmir_test.h"

namespace qtmir {

void PrintTo(const Application::InternalState& state, ::std::ostream* os)
{
    switch (state) {
    case Application::InternalState::Starting:
        *os << "Starting";
        break;
    case Application::InternalState::Running:
        *os << "Running";
        break;
    case Application::InternalState::RunningInBackground:
        *os << "RunningInBackground";
        break;
    case Application::InternalState::SuspendingWaitSession:
        *os << "SuspendingWaitSession";
        break;
    case Application::InternalState::SuspendingWaitProcess:
        *os << "SuspendingWaitProcess";
        break;
    case Application::InternalState::Suspended:
        *os << "Suspended";
        break;
    case Application::InternalState::StoppedResumable:
        *os << "StoppedResumable";
        break;
    case Application::InternalState::Closing:
        *os << "Closing";
        break;
    case Application::InternalState::Stopped:
        *os << "Stopped";
        break;
    default:
        *os << "???";
        break;
    }
}

void PrintTo(const Session::State& state, ::std::ostream* os)
{
    switch (state) {
    case SessionInterface::Starting:
        *os << "Starting";
        break;
    case SessionInterface::Running:
        *os << "Running";
        break;
    case SessionInterface::Suspending:
        *os << "Suspending";
        break;
    case SessionInterface::Suspended:
        *os << "Suspended";
        break;
    case SessionInterface::Stopped:
        *os << "Stopped";
        break;
    default:
        *os << "???";
        break;
    }
}

// Initialization of mir::Server needed for by tests
class TestMirServerInit : public virtual mir::Server
{
public:
    TestMirServerInit(std::shared_ptr<StubPromptSessionManager> const& promptSessionManager)
        : mock_prompt_session_manager(promptSessionManager)
    {
        override_the_prompt_session_manager(
            [this]{ return the_mock_prompt_session_manager(); });
    }

    std::shared_ptr<mir::scene::MockPromptSessionManager> the_mock_prompt_session_manager()
    {
        return mock_prompt_session_manager;
    }

private:
    std::shared_ptr<StubPromptSessionManager> const mock_prompt_session_manager;
};


namespace {  char const* argv[] = { nullptr }; }

class FakeMirServer: private TestMirServerInit, public MirServer
{
public:
    FakeMirServer(std::shared_ptr<StubPromptSessionManager> const& promptSessionManager)
    : TestMirServerInit(promptSessionManager), MirServer(0, argv, QSharedPointer<ScreenController>())
    {
    }

    using TestMirServerInit::the_mock_prompt_session_manager;
};

} // namespace qtmir

namespace testing
{

QtMirTest::QtMirTest()
    : promptSessionManager(std::make_shared<StubPromptSessionManager>())
    , mirServer(QSharedPointer<MirServer>(new FakeMirServer(promptSessionManager)))
    , applicationManager(mirServer,
                         taskControllerSharedPointer,
                         QSharedPointer<MockSharedWakelock>(&sharedWakelock, [](MockSharedWakelock *){}),
                         QSharedPointer<ProcInfo>(&procInfo,[](ProcInfo *){}),
                         QSharedPointer<MockSettings>(&settings,[](MockSettings *){}))
    , sessionManager(mirServer, &applicationManager)
    , surfaceManager(mirServer, mirShell, &sessionManager)
{
}

QtMirTest::~QtMirTest()
{

}

Application *QtMirTest::startApplication(pid_t procId, const QString &appId)
{
    using namespace testing;

    ON_CALL(*taskController,appIdHasProcessId(appId, procId)).WillByDefault(Return(true));

    // Set up Mocks & signal watcher
    EXPECT_CALL(*taskController, start(appId, _))
            .Times(1)
            .WillOnce(Return(true));

    auto application = applicationManager.startApplication(appId, ApplicationManager::NoFlag);
    applicationManager.onProcessStarting(appId);

    bool authed = false;
    applicationManager.authorizeSession(procId, authed);
    EXPECT_EQ(authed, true);

    auto appSession = std::make_shared<mir::scene::MockSession>(appId.toStdString(), procId);
    applicationManager.onSessionStarting(appSession);
    sessionManager.onSessionStarting(appSession);

    Mock::VerifyAndClearExpectations(taskController);
    return application;
}

} // namespace testing
