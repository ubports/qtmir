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
    QString appId("foo-app");

    auto desktopFileReader = new NiceMock<MockDesktopFileReader>(appId, QFileInfo());
    ON_CALL(*desktopFileReader, loaded()).WillByDefault(Return(true));

    QScopedPointer<Application> application(new Application(
            QSharedPointer<MockSharedWakelock>(&sharedWakelock, [](MockSharedWakelock *){}),
            desktopFileReader, QStringList(), nullptr));

    application->setProcessState(Application::ProcessRunning);

    NiceMock<MockSession> *session = new NiceMock<MockSession>;

    EXPECT_CALL(*session, setApplication(_));
    EXPECT_CALL(*session, fullscreen()).WillRepeatedly(Return(false));

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
    QString appId("foo-app");

    auto desktopFileReader = new NiceMock<MockDesktopFileReader>(appId, QFileInfo());
    ON_CALL(*desktopFileReader, loaded()).WillByDefault(Return(true));

    QScopedPointer<Application> application(new Application(
            QSharedPointer<MockSharedWakelock>(&sharedWakelock, [](MockSharedWakelock *){}),
            desktopFileReader, QStringList(), nullptr));
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
    QString appId("foo-app");

    auto desktopFileReader = new NiceMock<MockDesktopFileReader>(appId, QFileInfo());
    ON_CALL(*desktopFileReader, loaded()).WillByDefault(Return(true));

    QScopedPointer<Application> application(new Application(
            QSharedPointer<MockSharedWakelock>(&sharedWakelock, [](MockSharedWakelock *){}),
            desktopFileReader, QStringList(), nullptr));
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
    application->setProcessState(Application::ProcessStopped);
    ASSERT_EQ(Application::InternalState::StoppedUnexpectedly, application->internalState());

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
    QString appId("unity8-dash");

    EXPECT_CALL(sharedWakelock, acquire(_)).Times(0);
    EXPECT_CALL(sharedWakelock, release(_)).Times(0);

    auto desktopFileReader = new NiceMock<MockDesktopFileReader>(appId, QFileInfo());
    ON_CALL(*desktopFileReader, loaded()).WillByDefault(Return(true));

    QScopedPointer<Application> application(new Application(
            QSharedPointer<MockSharedWakelock>(&sharedWakelock, [](MockSharedWakelock *){}),
            desktopFileReader, QStringList(), nullptr));

    application->setProcessState(Application::ProcessRunning);

    NiceMock<MockSession> *session = new NiceMock<MockSession>;

    EXPECT_CALL(*session, setApplication(_));
    EXPECT_CALL(*session, fullscreen()).WillRepeatedly(Return(false));

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
