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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "qtmir_test.h"

#include <fake_mirsurface.h>
#include <fake_application_info.h>
#include <fake_session.h>
#include <mock_application_info.h>
#include <mock_session.h>

#include <Unity/Application/session.h>
#include <Unity/Application/timesource.h>

#include <QScopedPointer>
#include <QSignalSpy>

using namespace qtmir;

class ApplicationTests : public ::testing::QtMirTest
{
public:
    ApplicationTests()
        : fakeTimeSource(new FakeTimeSource)
    {
    }

    inline void suspend(QScopedPointer<Application> &application)
    {
        suspend(application.data());
    }

    inline void suspend(Application *application)
    {
        application->setRequestedState(Application::RequestedSuspended);
        auto session = dynamic_cast<Session*>(application->sessions()[0]);

        ASSERT_EQ(Application::InternalState::SuspendingWaitSession, application->internalState());
        ASSERT_EQ(Session::Suspending, session->state());

        QSignalSpy suspendProcessRequestedSpy(application, &Application::suspendProcessRequested);

        passTimeUntilTimerTimesOut(session->suspendTimer());

        ASSERT_EQ(Session::Suspended, session->state());
        ASSERT_EQ(Application::InternalState::SuspendingWaitProcess, application->internalState());
        ASSERT_EQ(1, suspendProcessRequestedSpy.count());

        application->setProcessState(Application::ProcessSuspended);

        ASSERT_EQ(Application::InternalState::Suspended, application->internalState());
    }

    inline Application *createApplicationWithFakes()
    {
        Application *application = new Application(
                QSharedPointer<MockSharedWakelock>(&sharedWakelock, [](MockSharedWakelock *){}),
                QSharedPointer<FakeApplicationInfo>::create());

        application->setStopTimer(new FakeTimer(fakeTimeSource));

        return application;
    }

    inline Session *createSessionWithFakes()
    {
        using namespace ::testing;
        const QString appId("test-app");
        const pid_t procId = 1234;

        auto mirSession = std::make_shared<NiceMock<mir::scene::MockSession>>(appId.toStdString(), procId);
        Session* session = new Session(mirSession, promptSessionManager);

        FakeTimer *fakeTimer(new FakeTimer(fakeTimeSource));
        session->setSuspendTimer(fakeTimer);

        return session;
    }

    inline void passTimeUntilTimerTimesOut(AbstractTimer *timer)
    {
        FakeTimer *fakeTimer = dynamic_cast<FakeTimer*>(timer);
        if (fakeTimer->isRunning()) {
            fakeTimeSource->m_msecsSinceReference = fakeTimer->nextTimeoutTime() + 1;
            fakeTimer->update();
        }
    }

    QSharedPointer<FakeTimeSource> fakeTimeSource;
};

TEST_F(ApplicationTests, acquiresWakelockWhenRunningAndReleasesWhenSuspended)
{
    using namespace ::testing;

    QScopedPointer<Application> application(createApplicationWithFakes());

    application->setProcessState(Application::ProcessRunning);

    FakeSession *session = new FakeSession;

    application->addSession(session);

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

    QScopedPointer<Application> application(createApplicationWithFakes());
    FakeSession *session = new FakeSession;

    // Get it running and then suspend it
    application->setProcessState(Application::ProcessRunning);
    application->addSession(session);
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

    QScopedPointer<Application> application(createApplicationWithFakes());
    FakeSession *session = new FakeSession;

    // Get it running, suspend it, and finally stop it
    application->setProcessState(Application::ProcessRunning);
    application->addSession(session);
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

    auto applicationInfo = QSharedPointer<FakeApplicationInfo>::create();
    applicationInfo->m_appId = QString("unity8-dash");

    QScopedPointer<Application> application(new Application(
                QSharedPointer<MockSharedWakelock>(&sharedWakelock, [](MockSharedWakelock *){}),
                applicationInfo));

    application->setProcessState(Application::ProcessRunning);

    FakeSession *session = new FakeSession;

    application->addSession(session);

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

    QScopedPointer<Application> application(createApplicationWithFakes());

    application->setProcessState(Application::ProcessRunning);

    FakeSession *session = new FakeSession;
    application->addSession(session);

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

    QScopedPointer<Application> application(createApplicationWithFakes());

    application->setProcessState(Application::ProcessRunning);

    Session *session = createSessionWithFakes();
    application->addSession(session);

    QSignalSpy spyAppStopped(application.data(), SIGNAL(stopped()));

    FakeMirSurface *surface = new FakeMirSurface;
    session->registerSurface(surface);
    surface->setReady();

    ASSERT_EQ(Application::InternalState::Running, application->internalState());

    application->setRequestedState(Application::RequestedSuspended);

    ASSERT_EQ(Application::InternalState::SuspendingWaitSession, application->internalState());

    // Now the application closes itself (ie, ending its own process)
    // Session always responds before the process state.
    session->setLive(false);
    application->setProcessState(Application::ProcessStopped);

    ASSERT_EQ(Application::InternalState::Stopped, application->internalState());
    ASSERT_EQ(1, spyAppStopped.count());

    // clean up
    delete surface;
}

TEST_F(ApplicationTests, doesNotEmitStoppedWhenKilledWhileSuspended)
{
    using namespace ::testing;

    QScopedPointer<Application> application(createApplicationWithFakes());

    application->setProcessState(Application::ProcessRunning);

    FakeSession *session = new FakeSession;
    application->addSession(session);

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

TEST_F(ApplicationTests, passesIsTouchAppThrough)
{
    using namespace ::testing;

    auto mockApplicationInfo = QSharedPointer<MockApplicationInfo>(new NiceMock<MockApplicationInfo>("foo-app"));
    QScopedPointer<Application> application(new Application(
            QSharedPointer<MockSharedWakelock>(&sharedWakelock, [](MockSharedWakelock *){}),
            mockApplicationInfo, QStringList(), nullptr));

    ON_CALL(*mockApplicationInfo, isTouchApp()).WillByDefault(Return(true));
    ASSERT_TRUE(application->isTouchApp());

    ON_CALL(*mockApplicationInfo, isTouchApp()).WillByDefault(Return(false));
    ASSERT_FALSE(application->isTouchApp());
}

/*
 * A suspended application resumes itself while any of its surfaces is being closed.
 */
TEST_F(ApplicationTests, suspendedApplicationResumesWhileSurfaceBeingClosed)
{
    using namespace ::testing;

    QScopedPointer<Application> application(createApplicationWithFakes());

    application->setProcessState(Application::ProcessRunning);

    Session *session = createSessionWithFakes();

    application->addSession(session);

    FakeMirSurface *surface = new FakeMirSurface;
    session->registerSurface(surface);
    surface->setReady();

    ASSERT_EQ(Application::InternalState::Running, application->internalState());

    // Add a second surface to ensure the application doesn't kill itself after it loses
    // one surface.
    FakeMirSurface *secondSurface = new FakeMirSurface;
    session->registerSurface(secondSurface);
    secondSurface->setReady();

    suspend(application.data());

    QSignalSpy resumeProcessRequestedSpy(application.data(), &Application::resumeProcessRequested);

    surface->close();

    ASSERT_EQ(Application::InternalState::Running, application->internalState());
    ASSERT_EQ(1, resumeProcessRequestedSpy.count());
    ASSERT_EQ(Session::Running, session->state());

    // And goes back to sleep after the closing surface is gone

    QSignalSpy suspendProcessRequestedSpy(application.data(), &Application::suspendProcessRequested);

    delete surface;

    ASSERT_EQ(Application::InternalState::SuspendingWaitSession, application->internalState());
    ASSERT_EQ(Session::Suspending, session->state());

    passTimeUntilTimerTimesOut(session->suspendTimer());

    ASSERT_EQ(Session::Suspended, session->state());
    ASSERT_EQ(Application::InternalState::SuspendingWaitProcess, application->internalState());
    ASSERT_EQ(1, suspendProcessRequestedSpy.count());

    application->setProcessState(Application::ProcessSuspended);

    ASSERT_EQ(Application::InternalState::Suspended, application->internalState());

    // clean up
    delete secondSurface;
}

TEST_F(ApplicationTests, quitsAfterLastSurfaceIsClosed)
{
    using namespace ::testing;

    QScopedPointer<Application> application(createApplicationWithFakes());

    application->setProcessState(Application::ProcessRunning);

    Session *session = createSessionWithFakes();

    application->addSession(session);

    FakeMirSurface *surface = new FakeMirSurface;
    session->registerSurface(surface);
    surface->setReady();

    ASSERT_EQ(Application::InternalState::Running, application->internalState());
    ASSERT_EQ(Session::Running, session->state());

    FakeMirSurface *secondSurface = new FakeMirSurface;
    session->registerSurface(secondSurface);
    secondSurface->setReady();

    delete surface;

    passTimeUntilTimerTimesOut(application->stopTimer());

    // All fine as there's still one surface left
    ASSERT_EQ(Application::InternalState::Running, application->internalState());
    ASSERT_EQ(Session::Running, session->state());

    delete secondSurface;

    QSignalSpy stopProcessRequestedSpy(application.data(), &Application::stopProcessRequested);

    passTimeUntilTimerTimesOut(application->stopTimer());

    // Ok, now the application should go way
    ASSERT_EQ(1, stopProcessRequestedSpy.count());
}

/*
 * Test that an application that is suspended after its session is stopped is closed
 *
 * Regression test for bug LP#1536133
 * (https://bugs.launchpad.net/canonical-devices-system-image/+bug/1536133)
 */
TEST_F(ApplicationTests, sessionStopsWhileBeingSuspended)
{
    using namespace ::testing;

    int argc = 0;
    char* argv[0];
    QCoreApplication qtApp(argc, argv); // app for deleteLater event

    QScopedPointer<Application> application(createApplicationWithFakes());

    application->setProcessState(Application::ProcessRunning);

    QPointer<Session> session(createSessionWithFakes());

    application->addSession(session);

    FakeMirSurface *surface = new FakeMirSurface;
    session->registerSurface(surface);
    surface->setReady();
    ASSERT_EQ(Application::InternalState::Running, application->internalState());

    application->setRequestedState(Application::RequestedSuspended);

    ASSERT_EQ(Application::InternalState::SuspendingWaitSession, application->internalState());

    passTimeUntilTimerTimesOut(session->suspendTimer());

    ASSERT_EQ(Application::InternalState::SuspendingWaitProcess, application->internalState());
    QSignalSpy stopProcessRequestedSpy(application.data(), &Application::stopProcessRequested);

    // surface dies, followed by session
    surface->setLive(false);
    delete surface;
    surface = nullptr;
    session->setLive(false);

    // Session should have called deleteLater() on itself, as it's zombie and doesn't hold any surface
    // But DeferredDelete is special: likes to be called out specifically or it won't come out
    qtApp.sendPostedEvents(session.data(), QEvent::DeferredDelete);
    qtApp.sendPostedEvents();

    EXPECT_EQ(true, session.isNull());
    EXPECT_EQ(1, stopProcessRequestedSpy.count());
    EXPECT_EQ(Application::InternalState::Stopped, application->internalState());
}

/*
 * Test that an application that fails while suspended will stop on close request
 */
TEST_F(ApplicationTests, closeWhenSuspendedProcessFailed)
{
    using namespace ::testing;

    QScopedPointer<Application> application(createApplicationWithFakes());

    application->setProcessState(Application::ProcessRunning);

    QPointer<Session> session(createSessionWithFakes());
    application->addSession(session);

    FakeMirSurface *surface = new FakeMirSurface;
    session->registerSurface(surface);
    surface->setReady();
    ASSERT_EQ(Application::InternalState::Running, application->internalState());

    suspend(application.data());

    // Process failed
    surface->setLive(false);
    session->setLive(false);
    application->setProcessState(Application::ProcessFailed);

    ASSERT_EQ(Application::InternalState::StoppedResumable, application->internalState());

    application->close();

    ASSERT_EQ(Application::InternalState::Stopped, application->internalState());

    // clean up
    delete surface;
}

/*
 If an application gets stopped (ie, loses its surfaces, session and process) while it's in Suspended
 state, it goes into StoppedResumable state.
 */
TEST_F(ApplicationTests, stoppedWhileSuspendedTurnsIntoStoppeResumable)
{
    using namespace ::testing;

    QScopedPointer<Application> application(createApplicationWithFakes());

    application->setProcessState(Application::ProcessRunning);

    Session *session = createSessionWithFakes();

    application->addSession(session);

    FakeMirSurface *surface = new FakeMirSurface;
    session->registerSurface(surface);
    surface->setReady();

    ASSERT_EQ(Application::InternalState::Running, application->internalState());

    suspend(application);

    // Process gets killed. Mir objects respond first
    surface->setLive(false);
    session->setLive(false);
    delete surface; surface = nullptr;

    EXPECT_EQ(Application::InternalState::StoppedResumable, application->internalState());

    // And later comes upstart telling us about it
    application->setProcessState(Application::ProcessFailed);

    EXPECT_EQ(Application::InternalState::StoppedResumable, application->internalState());
}

TEST_F(ApplicationTests, surfaceCountPropertyUpdates)
{
    using namespace ::testing;

    QScopedPointer<Application> application(createApplicationWithFakes());

    application->setProcessState(Application::ProcessRunning);
    Session *session = createSessionWithFakes();

    application->addSession(session);

    QSignalSpy surfaceCountChangedSpy(application.data(), &Application::surfaceCountChanged);

    EXPECT_EQ(application->surfaceCount(), 0);
    EXPECT_EQ(surfaceCountChangedSpy.count(), 0);

    FakeMirSurface *surface = new FakeMirSurface;
    session->registerSurface(surface);
    surface->setReady();

    EXPECT_EQ(application->surfaceCount(), 1);
    EXPECT_EQ(surfaceCountChangedSpy.count(), 1);

    FakeMirSurface *surface2 = new FakeMirSurface;
    session->registerSurface(surface2);
    surface2->setReady();

    EXPECT_EQ(application->surfaceCount(), 2);
    EXPECT_EQ(surfaceCountChangedSpy.count(), 2);

    delete surface;
    delete surface2;
}

/*
   Regression test for bug "App respawns if manually closed while it's launching"
   https://bugs.launchpad.net/ubuntu/+source/qtmir/+bug/1575577
 */
TEST_F(ApplicationTests, dontRespawnIfClosedWhileStillStartingUp)
{
    using namespace ::testing;

    QScopedPointer<Application> application(createApplicationWithFakes());

    application->setProcessState(Application::ProcessRunning);

    FakeSession *session = new FakeSession;

    application->addSession(session);

    QSignalSpy spyStartProcess(application.data(), SIGNAL(startProcessRequested()));

    // Close the application before it even gets a surface (it's still in "starting" state)
    application->close();

    session->setState(SessionInterface::Stopped);

    EXPECT_EQ(Application::InternalState::Stopped, application->internalState());
    EXPECT_EQ(0, spyStartProcess.count());
}
