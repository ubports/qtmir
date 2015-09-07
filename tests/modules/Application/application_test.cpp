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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "qtmir_test.h"

#include <fake_desktopfilereader.h>
#include <fake_session.h>
#include <mock_session.h>

#include <QScopedPointer>
#include <QSignalSpy>

using namespace qtmir;

class ApplicationTests : public ::testing::QtMirTest
{
public:
    ApplicationTests()
    {}
};

TEST_F(ApplicationTests, acquiresWakelockWhenRunningAndReleasesWhenSuspended)
{
    using namespace ::testing;

    QScopedPointer<Application> application(new Application(
            QSharedPointer<MockSharedWakelock>(&sharedWakelock, [](MockSharedWakelock *){}),
            new FakeDesktopFileReader, QStringList(), nullptr));

    application->setProcessState(Application::ProcessRunning);

    FakeSession *session = new FakeSession;

    application->setSession(session);

    ASSERT_EQ(Application::InternalState::Starting, application->internalState());

    session->setState(SessionInterface::Running);

    EXPECT_TRUE(sharedWakelock.enabled());

    ASSERT_EQ(Application::InternalState::Running, application->internalState());

    application->setRequestedState(Application::RequestedSuspended);

    ASSERT_EQ(SessionInterface::Suspending, session->state());
    ASSERT_EQ(Application::InternalState::SuspendingWaitSession, application->internalState());

    session->setState(SessionInterface::Suspended);

    ASSERT_EQ(Application::InternalState::SuspendingWaitProcess, application->internalState());

    application->setProcessState(Application::ProcessSuspended);

    ASSERT_EQ(Application::InternalState::Suspended, application->internalState());

    EXPECT_FALSE(sharedWakelock.enabled());
}

TEST_F(ApplicationTests, checkResumeAcquiresWakeLock)
{
    using namespace ::testing;

    QScopedPointer<Application> application(new Application(
            QSharedPointer<MockSharedWakelock>(&sharedWakelock, [](MockSharedWakelock *){}),
            new FakeDesktopFileReader, QStringList(), nullptr));
    NiceMock<MockSession> *session = new NiceMock<MockSession>;

    // Get it running and then suspend it
    application->setProcessState(Application::ProcessRunning);
    application->setSession(session);
    session->setState(SessionInterface::Running);
    application->setRequestedState(Application::RequestedSuspended);
    session->setState(SessionInterface::Suspended);
    application->setProcessState(Application::ProcessSuspended);
    ASSERT_EQ(Application::InternalState::Suspended, application->internalState());

    EXPECT_FALSE(sharedWakelock.enabled());

    application->setRequestedState(Application::RequestedRunning);

    ASSERT_EQ(Application::InternalState::Running, application->internalState());

    EXPECT_TRUE(sharedWakelock.enabled());
}

TEST_F(ApplicationTests, checkRespawnAcquiresWakeLock)
{
    using namespace ::testing;

    QScopedPointer<Application> application(new Application(
            QSharedPointer<MockSharedWakelock>(&sharedWakelock, [](MockSharedWakelock *){}),
            new FakeDesktopFileReader, QStringList(), nullptr));
    NiceMock<MockSession> *session = new NiceMock<MockSession>;

    // Get it running, suspend it, and finally stop it
    application->setProcessState(Application::ProcessRunning);
    application->setSession(session);
    session->setState(SessionInterface::Running);
    application->setRequestedState(Application::RequestedSuspended);
    session->setState(SessionInterface::Suspended);
    application->setProcessState(Application::ProcessSuspended);
    ASSERT_EQ(Application::InternalState::Suspended, application->internalState());
    session->setState(SessionInterface::Stopped);
    application->setProcessState(Application::ProcessFailed);
    ASSERT_EQ(Application::InternalState::StoppedResumable, application->internalState());

    EXPECT_FALSE(sharedWakelock.enabled());

    QSignalSpy spyStartProcess(application.data(), SIGNAL(startProcessRequested()));
    application->setRequestedState(Application::RequestedRunning);
    ASSERT_EQ(1, spyStartProcess.count());
    application->setProcessState(Application::ProcessRunning);

    ASSERT_EQ(Application::InternalState::Starting, application->internalState());

    EXPECT_TRUE(sharedWakelock.enabled());
}

TEST_F(ApplicationTests, checkDashDoesNotImpactWakeLock)
{
    using namespace ::testing;

    EXPECT_CALL(sharedWakelock, acquire(_)).Times(0);
    EXPECT_CALL(sharedWakelock, release(_)).Times(0);

    FakeDesktopFileReader *desktopFileReader = new FakeDesktopFileReader;
    desktopFileReader->m_appId = QString("unity8-dash");

    QScopedPointer<Application> application(new Application(
            QSharedPointer<MockSharedWakelock>(&sharedWakelock, [](MockSharedWakelock *){}),
            desktopFileReader, QStringList(), nullptr));

    application->setProcessState(Application::ProcessRunning);

    FakeSession *session = new FakeSession;

    application->setSession(session);

    ASSERT_EQ(Application::InternalState::Starting, application->internalState());

    session->setState(SessionInterface::Running);

    ASSERT_EQ(Application::InternalState::Running, application->internalState());

    application->setRequestedState(Application::RequestedSuspended);

    ASSERT_EQ(SessionInterface::Suspending, session->state());
    ASSERT_EQ(Application::InternalState::SuspendingWaitSession, application->internalState());

    session->setState(SessionInterface::Suspended);

    ASSERT_EQ(Application::InternalState::SuspendingWaitProcess, application->internalState());

    application->setProcessState(Application::ProcessSuspended);

    ASSERT_EQ(Application::InternalState::Suspended, application->internalState());

    application->setRequestedState(Application::RequestedRunning);

    ASSERT_EQ(Application::InternalState::Running, application->internalState());
}

TEST_F(ApplicationTests, emitsStoppedWhenRunningAppStops)
{
    using namespace ::testing;

    QScopedPointer<Application> application(new Application(
            QSharedPointer<MockSharedWakelock>(&sharedWakelock, [](MockSharedWakelock *){}),
            new FakeDesktopFileReader, QStringList(), nullptr));

    application->setProcessState(Application::ProcessRunning);

    FakeSession *session = new FakeSession;
    application->setSession(session);

    QSignalSpy spyAppStopped(application.data(), SIGNAL(stopped()));

    session->setState(SessionInterface::Running);

    ASSERT_EQ(Application::InternalState::Running, application->internalState());

    ////
    // Simulate a running application closing itself (ie, ending its own process)

    session->setState(SessionInterface::Stopped);

    ASSERT_EQ(Application::InternalState::Stopped, application->internalState());

    application->setProcessState(Application::ProcessStopped);

    ASSERT_EQ(1, spyAppStopped.count());
}

/**
 * Regression test for https://bugs.launchpad.net/qtmir/+bug/1485608
 * In that case, the camera-app closes itself right after unity8 unfocus it (and thus requests it to be suspended).
 */
TEST_F(ApplicationTests, emitsStoppedWhenAppStopsWhileSuspending)
{
    using namespace ::testing;

    QScopedPointer<Application> application(new Application(
            QSharedPointer<MockSharedWakelock>(&sharedWakelock, [](MockSharedWakelock *){}),
            new FakeDesktopFileReader, QStringList(), nullptr));

    application->setProcessState(Application::ProcessRunning);

    FakeSession *session = new FakeSession;
    application->setSession(session);

    QSignalSpy spyAppStopped(application.data(), SIGNAL(stopped()));

    session->setState(SessionInterface::Running);

    ASSERT_EQ(Application::InternalState::Running, application->internalState());

    application->setRequestedState(Application::RequestedSuspended);

    ASSERT_EQ(Application::InternalState::SuspendingWaitSession, application->internalState());

    // Now the application closes itself (ie, ending its own process)
    // Session always responds before the process state.
    session->setState(SessionInterface::Stopped);
    application->setProcessState(Application::ProcessStopped);

    ASSERT_EQ(Application::InternalState::Stopped, application->internalState());
    ASSERT_EQ(1, spyAppStopped.count());
}

TEST_F(ApplicationTests, doesNotEmitStoppedWhenKilledWhileSuspended)
{
    using namespace ::testing;

    QScopedPointer<Application> application(new Application(
            QSharedPointer<MockSharedWakelock>(&sharedWakelock, [](MockSharedWakelock *){}),
            new FakeDesktopFileReader, QStringList(), nullptr));

    application->setProcessState(Application::ProcessRunning);

    FakeSession *session = new FakeSession;
    application->setSession(session);

    QSignalSpy spyAppStopped(application.data(), SIGNAL(stopped()));

    session->setState(SessionInterface::Running);

    ASSERT_EQ(Application::InternalState::Running, application->internalState());

    application->setRequestedState(Application::RequestedSuspended);

    ASSERT_EQ(SessionInterface::Suspending, session->state());
    ASSERT_EQ(Application::InternalState::SuspendingWaitSession, application->internalState());

    session->setState(SessionInterface::Suspended);

    ASSERT_EQ(Application::InternalState::SuspendingWaitProcess, application->internalState());

    application->setProcessState(Application::ProcessSuspended);

    ///
    // Now simulate the process getting killed. Mir session always ends before
    // we get notified about the process state

    session->setState(SessionInterface::Stopped);

    application->setProcessState(Application::ProcessFailed);

    ASSERT_EQ(Application::InternalState::StoppedResumable, application->internalState());

    ASSERT_EQ(0, spyAppStopped.count());

}
