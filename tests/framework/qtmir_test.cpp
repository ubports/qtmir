/*
 * Copyright (C) 2015-2016 Canonical, Ltd.
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
    case Application::InternalState::Stopped:
        *os << "Stopped";
        break;
    default:
        *os << "???";
        break;
    }
}

void PrintTo(const SessionInterface::State& state, ::std::ostream* os)
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
} // namespace qtmir

void PrintTo(const QString &text, ::std::ostream *os)
{
    *os << text.toUtf8().constData();
}

void PrintTo(const QSize &size, ::std::ostream *os)
{
    QString output;
    QDebug debug(&output);

    debug << size;

    *os << output.toUtf8().constData();
}

void PrintTo(const QPoint &point, ::std::ostream *os)
{
    QString output;
    QDebug debug(&output);

    debug << point;

    *os << output.toUtf8().constData();
}

namespace testing
{

QtMirTest::QtMirTest()
    : persistentSurfaceStore(std::make_shared<StubPersistentSurfaceStore>())
    , applicationManager(taskControllerSharedPointer,
                         QSharedPointer<MockSharedWakelock>(&sharedWakelock, [](MockSharedWakelock *){}),
                         QSharedPointer<ProcInfo>(&procInfo,[](ProcInfo *){}),
                         QSharedPointer<MockSettings>(&settings,[](MockSettings *){}))
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

    auto application = applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);

    bool authed = false;
    applicationManager.authorizeSession(procId, authed);
    EXPECT_EQ(authed, true);

    auto appSession = std::make_shared<mir::scene::MockSession>(appId.toStdString(), procId);
    miral::ApplicationInfo appInfo(appSession);
    taskController->onSessionStarting(appInfo);

    Mock::VerifyAndClearExpectations(taskController);
    return application;
}

} // namespace testing
