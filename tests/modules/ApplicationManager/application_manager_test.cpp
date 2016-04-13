/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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

#define MIR_INCLUDE_DEPRECATED_EVENT_HEADER

#include <condition_variable>
#include <QSignalSpy>

#include <Unity/Application/mirfocuscontroller.h>
#include <Unity/Application/timer.h>

#include <fake_desktopfilereader.h>
#include <fake_mirsurface.h>
#include <mock_surface.h>
#include <qtmir_test.h>

using namespace qtmir;
using mir::scene::MockSession;

namespace ms = mir::scene;
namespace unityapi = unity::shell::application;

class ApplicationManagerTests : public ::testing::QtMirTest
{
public:
    ApplicationManagerTests()
    {}

    ~ApplicationManagerTests() {
    }

    inline void onSessionStarting(const std::shared_ptr<mir::scene::Session> &session) {
        applicationManager.onSessionStarting(session);
        sessionManager.onSessionStarting(session);
    }
    inline void onSessionStopping(const std::shared_ptr<mir::scene::Session> &session) {
        applicationManager.onSessionStopping(session);
        sessionManager.onSessionStopping(session);
    }
    inline void onSessionCreatedSurface(const mir::scene::Session *mirSession,
            MirSurfaceInterface *qmlSurface) {

        SessionInterface* qmlSession = sessionManager.findSession(mirSession);
        if (qmlSession) {
            qmlSession->registerSurface(qmlSurface);
        }

        // I assume that applicationManager ignores the mirSurface parameter, so sending
        // a null shared pointer must suffice
        std::shared_ptr<mir::scene::Surface> mirSurface(nullptr);
        applicationManager.onSessionCreatedSurface(mirSession, mirSurface);
    }

    inline void suspend(Application *application) {
        application->setRequestedState(Application::RequestedSuspended);
        ASSERT_EQ(Application::InternalState::SuspendingWaitSession, application->internalState());
        static_cast<qtmir::Session*>(application->session())->doSuspend();
        ASSERT_EQ(Application::InternalState::SuspendingWaitProcess, application->internalState());
        applicationManager.onProcessSuspended(application->appId());
        ASSERT_EQ(Application::InternalState::Suspended, application->internalState());
    }

    static inline qtmir::DesktopFileReader* createMockDesktopFileReader(const QString &appId, const QFileInfo &fi) {
        using namespace ::testing;
        auto mockDesktopFileReader = new NiceMock<MockDesktopFileReader>(appId, fi);
        ON_CALL(*mockDesktopFileReader, loaded()).WillByDefault(Return(true));
        return mockDesktopFileReader;
    }

protected:
    virtual void SetUp() override {
        if (m_tempDir.isValid()) qputenv("XDG_CACHE_HOME", m_tempDir.path().toUtf8());
    }

private:
    const QTemporaryDir m_tempDir;
};

TEST_F(ApplicationManagerTests,bug_case_1240400_second_dialer_app_fails_to_authorize_and_gets_mixed_up_with_first_one)
{
    using namespace ::testing;
    const pid_t firstProcId = 5921;
    const pid_t secondProcId = 5922;
    const char dialer_app_id[] = "dialer-app";
    QByteArray cmdLine( "/usr/bin/dialer-app --desktop_file_hint=dialer-app");
    QByteArray secondcmdLine( "/usr/bin/dialer-app");

    FakeMirSurface *surface = new FakeMirSurface;

    EXPECT_CALL(procInfo,command_line(firstProcId))
        .Times(1)
        .WillOnce(Return(cmdLine));
    EXPECT_CALL(procInfo,command_line(secondProcId))
        .Times(1)
        .WillOnce(Return(secondcmdLine));

    bool authed = true;

    std::shared_ptr<mir::scene::Session> mirSession = std::make_shared<MockSession>(dialer_app_id, firstProcId);
    applicationManager.authorizeSession(firstProcId, authed);
    ASSERT_EQ(true, authed);
    onSessionStarting(mirSession);
    onSessionCreatedSurface(mirSession.get(), surface);
    surface->drawFirstFrame();
    Application * application = applicationManager.findApplication(dialer_app_id);
    ASSERT_NE(nullptr,application);
    ASSERT_EQ(Application::InternalState::Running, application->internalState());

    // now a second session without desktop file is launched:
    applicationManager.authorizeSession(secondProcId, authed);
    applicationManager.onProcessStarting(dialer_app_id);

    EXPECT_FALSE(authed);
    EXPECT_EQ(application, applicationManager.findApplication(dialer_app_id));
}

TEST_F(ApplicationManagerTests,application_dies_while_starting)
{
    using namespace ::testing;
    const pid_t procId = 5921;
    const char app_id[] = "my-app";
    QByteArray cmdLine( "/usr/bin/my-app --desktop_file_hint=my-app");

    EXPECT_CALL(procInfo,command_line(procId))
        .Times(1)
        .WillOnce(Return(cmdLine));

    bool authed = true;

    std::shared_ptr<mir::scene::Session> mirSession = std::make_shared<MockSession>(app_id, procId);
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(mirSession);
    Application * beforeFailure = applicationManager.findApplication(app_id);
    applicationManager.onProcessStarting(app_id);
    onSessionStopping(mirSession);
    applicationManager.onProcessFailed(app_id, TaskController::Error::APPLICATION_FAILED_TO_START);
    Application * afterFailure = applicationManager.findApplication(app_id);

    EXPECT_EQ(true, authed);
    EXPECT_NE(nullptr, beforeFailure);
    EXPECT_EQ(nullptr, afterFailure);
}

TEST_F(ApplicationManagerTests,startApplicationSupportsShortAppId)
{
    using namespace ::testing;

    const QString shortAppId("com.canonical.test_test");

    EXPECT_CALL(*taskController, start(_, _)).Times(1);
    EXPECT_CALL(*taskController, findDesktopFileForAppId(shortAppId)).Times(1);

    EXPECT_CALL(desktopFileReaderFactory, createInstance(_, _)).Times(1);

    auto application = applicationManager.startApplication(
                shortAppId,
                QStringList());

    EXPECT_EQ(shortAppId, application->appId());
}

TEST_F(ApplicationManagerTests,startApplicationSupportsLongAppId)
{
    using namespace ::testing;

    const QString longAppId("com.canonical.test_test_0.1.235");
    const QString shortAppId("com.canonical.test_test");

    EXPECT_CALL(*taskController, start(_, _)).Times(1);
    EXPECT_CALL(*taskController, findDesktopFileForAppId(shortAppId)).Times(1);

    EXPECT_CALL(desktopFileReaderFactory, createInstance(_, _)).Times(1);

    auto application = applicationManager.startApplication(
                longAppId,
                QStringList());

    EXPECT_EQ(shortAppId, application->appId());
}

TEST_F(ApplicationManagerTests,testAppIdGuessFromDesktopFileName)
{
    using namespace ::testing;
    const pid_t procId = 5921;
    QString appId("sudoku-app");
    QString cmdLine = QString("/usr/bin/my-app --desktop_file_hint=/usr/share/click/preinstalled/com.ubuntu.sudoku/1.0.180/%1.desktop").arg(appId);

    EXPECT_CALL(procInfo,command_line(procId))
        .Times(1)
        .WillOnce(Return(qPrintable(cmdLine)));

    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    Application *app = applicationManager.findApplication(appId);

    EXPECT_EQ(true, authed);
    EXPECT_NE(app, nullptr);
    EXPECT_EQ(appId, app->appId());
}

TEST_F(ApplicationManagerTests,testAppIdGuessFromDesktopFileNameWithLongAppId)
{
    using namespace ::testing;
    const pid_t procId = 5921;
    QString shortAppId("com.ubuntu.desktop_desktop");
    QString cmdLine = QString("/usr/bin/my-app --desktop_file_hint=/usr/share/applications/%1_1.0.180.desktop").arg(shortAppId);

    EXPECT_CALL(procInfo,command_line(procId))
        .Times(1)
        .WillOnce(Return(qPrintable(cmdLine)));

    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    Application *app = applicationManager.findApplication(shortAppId);

    EXPECT_EQ(true, authed);
    EXPECT_NE(app, nullptr);
    EXPECT_EQ(shortAppId, app->appId());
}

TEST_F(ApplicationManagerTests,bug_case_1281075_session_ptrs_always_distributed_to_last_started_app)
{
    using namespace ::testing;
    const pid_t first_procId = 5921;
    const pid_t second_procId = 5922;
    const pid_t third_procId = 5923;
    std::shared_ptr<mir::scene::Surface> aSurface(nullptr);
    const char first_app_id[] = "app1";
    QByteArray first_cmdLine( "/usr/bin/app1 --desktop_file_hint=app1");
    const char second_app_id[] = "app2";
    QByteArray second_cmdLine( "/usr/bin/app2--desktop_file_hint=app2");
    const char third_app_id[] = "app3";
    QByteArray third_cmdLine( "/usr/bin/app3 --desktop_file_hint=app3");

    EXPECT_CALL(procInfo,command_line(first_procId))
        .Times(1)
        .WillOnce(Return(first_cmdLine));

    ON_CALL(*taskController,appIdHasProcessId(_,_)).WillByDefault(Return(false));

    EXPECT_CALL(procInfo,command_line(second_procId))
        .Times(1)
        .WillOnce(Return(second_cmdLine));

    EXPECT_CALL(procInfo,command_line(third_procId))
        .Times(1)
        .WillOnce(Return(third_cmdLine));

    bool authed = true;

    std::shared_ptr<mir::scene::Session> first_session = std::make_shared<MockSession>("Oo", first_procId);
    std::shared_ptr<mir::scene::Session> second_session = std::make_shared<MockSession>("oO", second_procId);
    std::shared_ptr<mir::scene::Session> third_session = std::make_shared<MockSession>("OO", third_procId);
    applicationManager.authorizeSession(first_procId, authed);
    applicationManager.authorizeSession(second_procId, authed);
    applicationManager.authorizeSession(third_procId, authed);
    onSessionStarting(first_session);
    onSessionStarting(third_session);
    onSessionStarting(second_session);

    Application * firstApp = applicationManager.findApplication(first_app_id);
    Application * secondApp = applicationManager.findApplication(second_app_id);
    Application * thirdApp = applicationManager.findApplication(third_app_id);

    EXPECT_EQ(first_session, firstApp->session()->session());
    EXPECT_EQ(second_session, secondApp->session()->session());
    EXPECT_EQ(third_session, thirdApp->session()->session());
}

TEST_F(ApplicationManagerTests,two_session_on_one_application)
{
    using namespace ::testing;
    const pid_t a_procId = 5921;
    const char an_app_id[] = "some_app";
    QByteArray a_cmd( "/usr/bin/app1 --desktop_file_hint=some_app");

    ON_CALL(procInfo,command_line(_)).WillByDefault(Return(a_cmd));

    ON_CALL(*taskController,appIdHasProcessId(_,_)).WillByDefault(Return(false));

    bool authed = true;

    std::shared_ptr<mir::scene::Session> first_session = std::make_shared<MockSession>("Oo", a_procId);
    std::shared_ptr<mir::scene::Session> second_session = std::make_shared<MockSession>("oO", a_procId);
    applicationManager.authorizeSession(a_procId, authed);

    onSessionStarting(first_session);
    onSessionStarting(second_session);

    Application * the_app = applicationManager.findApplication(an_app_id);

    EXPECT_EQ(true, authed);
    EXPECT_EQ(second_session, the_app->session()->session());
}

TEST_F(ApplicationManagerTests,DISABLED_upstart_launching_sidestage_app_on_phone_forced_into_mainstage)
{
    using namespace ::testing;
    QString appId("sideStage");

    EXPECT_CALL(*taskController, findDesktopFileForAppId(appId)).Times(1);

    auto mockDesktopFileReader = new NiceMock<MockDesktopFileReader>(appId, QFileInfo());
    ON_CALL(*mockDesktopFileReader, loaded()).WillByDefault(Return(true));
    ON_CALL(*mockDesktopFileReader, stageHint()).WillByDefault(Return("SideStage"));

    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Return(mockDesktopFileReader));

    // mock upstart launching an app which reports itself as sidestage, but we're on phone
    applicationManager.onProcessStarting(appId);

    // ensure the app stage is overridden to be main stage
    Application* theApp = applicationManager.findApplication(appId);
    ASSERT_NE(theApp, nullptr);
    EXPECT_EQ(Application::MainStage, theApp->stage());
}

TEST_F(ApplicationManagerTests,two_session_on_one_application_after_starting)
{
    using namespace ::testing;
    const pid_t a_procId = 5921;
    const char an_app_id[] = "some_app";
    QByteArray a_cmd( "/usr/bin/app1 --desktop_file_hint=some_app");
    FakeMirSurface *aSurface = new FakeMirSurface;

    ON_CALL(procInfo,command_line(_)).WillByDefault(Return(a_cmd));

    ON_CALL(*taskController,appIdHasProcessId(_,_)).WillByDefault(Return(false));

    bool authed = true;

    std::shared_ptr<mir::scene::Session> first_session = std::make_shared<MockSession>("Oo", a_procId);
    std::shared_ptr<mir::scene::Session> second_session = std::make_shared<MockSession>("oO", a_procId);
    applicationManager.authorizeSession(a_procId, authed);

    onSessionStarting(first_session);
    onSessionCreatedSurface(first_session.get(), aSurface);
    aSurface->drawFirstFrame();
    onSessionStarting(second_session);

    Application * the_app = applicationManager.findApplication(an_app_id);

    EXPECT_EQ(true, authed);
    EXPECT_EQ(Application::Running, the_app->state());
    EXPECT_EQ(first_session, the_app->session()->session());
}

TEST_F(ApplicationManagerTests,starting_app_is_suspended_when_it_gets_ready_if_requested)
{
    using namespace ::testing;
    const pid_t procId = 5921;
    FakeMirSurface *aSurface = new FakeMirSurface;
    QByteArray cmdLine( "/usr/bin/app --desktop_file_hint=app");

    EXPECT_CALL(procInfo,command_line(procId))
        .Times(1)
        .WillOnce(Return(cmdLine));

    ON_CALL(*taskController,appIdHasProcessId(_,_)).WillByDefault(Return(false));

    bool authed = true;

    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("Oo", procId);
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    Application * app = applicationManager.findApplication("app");
    app->setRequestedState(Application::RequestedSuspended);

    // First app starting...
    EXPECT_EQ(Application::Starting, app->state());

    // Signal app is ready now
    applicationManager.onProcessStarting("app");
    onSessionCreatedSurface(session.get(), aSurface);
    aSurface->drawFirstFrame();

    // now that its ready, suspend process should have begun
    EXPECT_EQ(Application::InternalState::SuspendingWaitSession, app->internalState());
}

TEST_F(ApplicationManagerTests,requestFocusApplication)
{
    using namespace ::testing;
    const pid_t first_procId = 5921;
    const pid_t second_procId = 5922;
    const pid_t third_procId = 5923;
    std::shared_ptr<mir::scene::Surface> aSurface(nullptr);
    QByteArray first_cmdLine( "/usr/bin/app1 --desktop_file_hint=app1");
    QByteArray second_cmdLine( "/usr/bin/app2--desktop_file_hint=app2");
    QByteArray third_cmdLine( "/usr/bin/app3 --desktop_file_hint=app3");

    EXPECT_CALL(procInfo,command_line(first_procId))
        .Times(1)
        .WillOnce(Return(first_cmdLine));

    ON_CALL(*taskController,appIdHasProcessId(_,_)).WillByDefault(Return(false));

    EXPECT_CALL(procInfo,command_line(second_procId))
        .Times(1)
        .WillOnce(Return(second_cmdLine));

    EXPECT_CALL(procInfo,command_line(third_procId))
        .Times(1)
        .WillOnce(Return(third_cmdLine));

    bool authed = true;

    std::shared_ptr<mir::scene::Session> first_session = std::make_shared<MockSession>("Oo", first_procId);
    std::shared_ptr<mir::scene::Session> second_session = std::make_shared<MockSession>("oO", second_procId);
    std::shared_ptr<mir::scene::Session> third_session = std::make_shared<MockSession>("OO", third_procId);
    applicationManager.authorizeSession(first_procId, authed);
    applicationManager.authorizeSession(second_procId, authed);
    applicationManager.authorizeSession(third_procId, authed);
    onSessionStarting(first_session);
    onSessionStarting(third_session);
    onSessionStarting(second_session);

    QSignalSpy spy(&applicationManager, SIGNAL(focusRequested(const QString &)));

    applicationManager.requestFocusApplication("app3");

    EXPECT_EQ(spy.count(), 1);

    QList<QVariant> arguments = spy.takeFirst(); // take the first signal
    EXPECT_EQ(arguments.at(0).toString(), "app3");
}

/*
 * Test that an application launched by shell itself creates the correct Application instance and
 * emits signals indicating the model updated
 */
TEST_F(ApplicationManagerTests,appStartedByShell)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const QString name("Test App");

    // Set up Mocks & signal watcher
    auto mockDesktopFileReader = new NiceMock<MockDesktopFileReader>(appId, QFileInfo());
    ON_CALL(*mockDesktopFileReader, loaded()).WillByDefault(Return(true));
    ON_CALL(*mockDesktopFileReader, appId()).WillByDefault(Return(appId));
    ON_CALL(*mockDesktopFileReader, name()).WillByDefault(Return(name));

    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Return(mockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));

    // start the application
    Application *theApp = applicationManager.startApplication(appId);

    // check application data
    EXPECT_EQ(Application::Starting, theApp->state());
    EXPECT_EQ(appId, theApp->appId());
    EXPECT_EQ(name, theApp->name());
    EXPECT_FALSE(theApp->canBeResumed());

    // check signals were emitted
    EXPECT_EQ(2, countSpy.count()); //FIXME(greyback)
    EXPECT_EQ(1, applicationManager.count());

    // check application in list of apps
    Application *theAppAgain = applicationManager.findApplication(appId);
    EXPECT_EQ(theApp, theAppAgain);
}

/*
 * Test that an application launched upstart (i.e. not by shell itself) creates the correct Application
 * instance and emits signals indicating the model updated
 */
TEST_F(ApplicationManagerTests,appStartedByUpstart)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const QString name("Test App");

    // Set up Mocks & signal watcher
    auto mockDesktopFileReader = new NiceMock<MockDesktopFileReader>(appId, QFileInfo());
    ON_CALL(*mockDesktopFileReader, loaded()).WillByDefault(Return(true));
    ON_CALL(*mockDesktopFileReader, appId()).WillByDefault(Return(appId));
    ON_CALL(*mockDesktopFileReader, name()).WillByDefault(Return(name));

    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Return(mockDesktopFileReader));

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));
    QSignalSpy focusSpy(&applicationManager, SIGNAL(focusRequested(const QString &)));

    // upstart sends notification that the application was started
    applicationManager.onProcessStarting(appId);

    Application *theApp = applicationManager.findApplication(appId);

    // check application data
    EXPECT_EQ(Application::Starting, theApp->state());
    EXPECT_EQ(appId, theApp->appId());
    EXPECT_EQ(name, theApp->name());
    EXPECT_EQ(true, theApp->canBeResumed());

    // check signals were emitted
    EXPECT_EQ(2, countSpy.count()); //FIXME(greyback)
    EXPECT_EQ(1, applicationManager.count());
    EXPECT_EQ(1, focusSpy.count());
    EXPECT_EQ(appId, focusSpy.takeFirst().at(0).toString());
}

/*
 * Test that an application launched via the command line with a correct --desktop_file_hint is accepted,
 * creates the correct Application instance and emits signals indicating the model updated
 */
TEST_F(ApplicationManagerTests,appStartedUsingCorrectDesktopFileHintSwitch)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const QString name("Test App");
    const pid_t procId = 5551;
    QByteArray cmdLine("/usr/bin/testApp --desktop_file_hint=");
    cmdLine = cmdLine.append(appId);

    // Set up Mocks & signal watcher
    EXPECT_CALL(procInfo,command_line(procId))
        .Times(1)
        .WillOnce(Return(cmdLine));

    auto mockDesktopFileReader = new NiceMock<MockDesktopFileReader>(appId, QFileInfo());
    ON_CALL(*mockDesktopFileReader, loaded()).WillByDefault(Return(true));
    ON_CALL(*mockDesktopFileReader, appId()).WillByDefault(Return(appId));
    ON_CALL(*mockDesktopFileReader, name()).WillByDefault(Return(name));

    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Return(mockDesktopFileReader));

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));

    // Mir requests authentication for an application that was started
    bool authed = false;
    applicationManager.authorizeSession(procId, authed);
    EXPECT_EQ(authed, true);

    Application *theApp = applicationManager.findApplication(appId);

    // check application data
    EXPECT_EQ(theApp->state(), Application::Starting);
    EXPECT_EQ(theApp->appId(), appId);
    EXPECT_EQ(theApp->name(), name);
    EXPECT_EQ(theApp->canBeResumed(), false);

    // check signals were emitted
    EXPECT_EQ(countSpy.count(), 2); //FIXME(greyback)
    EXPECT_EQ(applicationManager.count(), 1);
}

/*
 * Test that an application launched via the command line without the correct --desktop_file_hint is rejected
 */
TEST_F(ApplicationManagerTests,appDoesNotStartWhenUsingBadDesktopFileHintSwitch)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const QString name("Test App");
    const pid_t procId = 5551;
    QByteArray cmdLine("/usr/bin/testApp");

    // Set up Mocks & signal watcher
    EXPECT_CALL(procInfo,command_line(procId))
        .Times(1)
        .WillOnce(Return(cmdLine));

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));

    // Mir requests authentication for an application that was started
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    EXPECT_EQ(authed, false);

    Application *theApp = applicationManager.findApplication(appId);

    EXPECT_EQ(theApp, nullptr);

    // check no new signals were emitted
    EXPECT_EQ(countSpy.count(), 0);
    EXPECT_EQ(applicationManager.count(), 0);
}

/*
 * Test that an application launched via the command line with the --desktop_file_hint but an incorrect
 * desktop file specified is rejected
 */
TEST_F(ApplicationManagerTests,appDoesNotStartWhenUsingBadDesktopFileHintFile)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const QString badDesktopFile = QString("%1.desktop").arg(appId);
    const pid_t procId = 5551;
    QByteArray cmdLine("/usr/bin/testApp --desktop_file_hint=");
    cmdLine = cmdLine.append(badDesktopFile);

    // Set up Mocks & signal watcher
    EXPECT_CALL(procInfo,command_line(procId))
        .Times(1)
        .WillOnce(Return(cmdLine));

    auto mockDesktopFileReader = new NiceMock<MockDesktopFileReader>(appId, QFileInfo());
    ON_CALL(*mockDesktopFileReader, loaded()).WillByDefault(Return(false));

    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Return(mockDesktopFileReader));

    // Mir requests authentication for an application that was started, should fail
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    EXPECT_EQ(authed, false);
}

/*
 * Test that if TaskController synchronously calls back processStarted, that ApplicationManager
 * does not add the app to the model twice.
 */
TEST_F(ApplicationManagerTests,synchronousProcessStartedCallDoesNotDuplicateEntryInModel)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const QString name("Test App");

    // Set up Mocks & signal watcher
    auto mockDesktopFileReader = new NiceMock<MockDesktopFileReader>(appId, QFileInfo());
    ON_CALL(*mockDesktopFileReader, loaded()).WillByDefault(Return(true));
    ON_CALL(*mockDesktopFileReader, appId()).WillByDefault(Return(appId));
    ON_CALL(*mockDesktopFileReader, name()).WillByDefault(Return(name));

    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Return(mockDesktopFileReader));

    ON_CALL(*taskController, start(appId, _))
        .WillByDefault(Invoke(
                        [&](const QString &appId, Unused) {
                            applicationManager.onProcessStarting(appId);
                            return true;
                        }
                      ));

    // start the application
    Application *theApp = applicationManager.startApplication(appId);

    // check application data
    EXPECT_EQ(theApp->state(), Application::Starting);
    EXPECT_EQ(theApp->appId(), appId);
    EXPECT_EQ(theApp->name(), name);
    EXPECT_EQ(theApp->canBeResumed(), true);

    // check only once instance in the model
    EXPECT_EQ(applicationManager.count(), 1);

    // check application in list of apps
    Application *theAppAgain = applicationManager.findApplication(appId);
    EXPECT_EQ(theAppAgain, theApp);
}

/*
 * Test that the child sessions of a webapp process are accepted
 */
TEST_F(ApplicationManagerTests,webAppSecondarySessionsAccepted)
{
    using namespace ::testing;
    const pid_t procId = 5551;
    QByteArray cmdLine("/usr/bin/qt5/libexec/QtWebProcess");

    // Set up Mocks & signal watcher
    EXPECT_CALL(procInfo,command_line(procId))
        .Times(1)
        .WillOnce(Return(cmdLine));

    bool authed = false;
    applicationManager.authorizeSession(procId, authed);
    EXPECT_EQ(authed, true);
}

/*
 * Test that maliit sessions are accepted
 */
TEST_F(ApplicationManagerTests,maliitSessionsAccepted)
{
    using namespace ::testing;
    const pid_t procId = 151;
    QByteArray cmdLine("maliit-server --blah");

    // Set up Mocks & signal watcher
    EXPECT_CALL(procInfo,command_line(procId))
        .Times(1)
        .WillOnce(Return(cmdLine));

    bool authed = false;
    applicationManager.authorizeSession(procId, authed);
    EXPECT_EQ(authed, true);
}

/*
 * Test that an application in the Starting state is not impacted by the upstart "Starting" message
 * for that application (i.e. the upstart message is effectively useless)
 */
TEST_F(ApplicationManagerTests,onceAppAddedToApplicationLists_upstartStartingEventIgnored)
{
    using namespace ::testing;
    const QString appId("testAppId");

    // Set up Mocks & signal watcher
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    applicationManager.startApplication(appId);

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));

    // upstart sends notification that the application was started
    applicationManager.onProcessStarting(appId);

    // check no new signals were emitted and application state unchanged
    EXPECT_EQ(countSpy.count(), 0);
    EXPECT_EQ(applicationManager.count(), 1);

    Application *theApp = applicationManager.findApplication(appId);
    EXPECT_EQ(Application::Starting, theApp->state());
}

/*
 * Test that an application in the Starting state reacts correctly to the Mir sessionStarted
 * event for that application (i.e. the Session is associated)
 */
TEST_F(ApplicationManagerTests,onceAppAddedToApplicationLists_mirSessionStartingEventHandled)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const pid_t procId = 5551;

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));

    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);

    // Authorize session and emit Mir sessionStarting event
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    EXPECT_EQ(countSpy.count(), 0);
    EXPECT_EQ(applicationManager.count(), 1);

    // Check application state and session are correctly set
    Application *theApp = applicationManager.findApplication(appId);
    EXPECT_EQ(theApp->session()->session(), session);
    EXPECT_EQ(theApp->focused(), false);
}

/*
 * Test that an application in the Starting state reacts correctly to the Mir surfaceCreated
 * event for that application (i.e. the Surface is associated and state set to Running)
 */
TEST_F(ApplicationManagerTests,onceAppAddedToApplicationLists_mirSurfaceCreatedEventHandled)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const pid_t procId = 5551;

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);

    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);

    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    FakeMirSurface *surface = new FakeMirSurface;

    onSessionCreatedSurface(session.get(), surface);
    surface->drawFirstFrame();

    // Check application state is correctly set
    Application *theApp = applicationManager.findApplication(appId);
    EXPECT_EQ(theApp->state(), Application::Running);
}

/*
 * Test that an application is stopped correctly, if it has not yet created a surface (still in Starting state)
 */
TEST_F(ApplicationManagerTests,shellStopsAppCorrectlyBeforeSurfaceCreated)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const pid_t procId = 5551;

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));

    EXPECT_CALL(*taskController, stop(appId))
        .Times(1)
        .WillOnce(Return(true));

    // Stop app
    applicationManager.stopApplication(appId);

    Mock::VerifyAndClearExpectations(taskController);

    // emulate mir session dying and taskController emitting processStopped(appId) in response
    // to the taskController->stop(appId) call from applicationManager
    onSessionStopping(session);
    applicationManager.onProcessStopped(appId);

    EXPECT_EQ(countSpy.count(), 2); //FIXME(greyback)
    EXPECT_EQ(applicationManager.count(), 0);
}

/*
 * Test that a running application is stopped correctly (is in Running state, has surface)
 */
TEST_F(ApplicationManagerTests,shellStopsForegroundAppCorrectly)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const pid_t procId = 5551;

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    Application *app = applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<NiceMock<MockSession>>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    FakeMirSurface *surface = new FakeMirSurface;
    onSessionCreatedSurface(session.get(), surface);
    surface->drawFirstFrame();

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));

    QSignalSpy closeRequestedSpy(surface, SIGNAL(closeRequested()));

    // Stop app
    applicationManager.stopApplication(appId);

    // Asking ApplicationManager to stop the application just makes it request its surfaces to be
    // closed
    EXPECT_EQ(1, closeRequestedSpy.count());

    // comply
    delete surface;
    surface = nullptr;

    // now it's the turn of the application process itself to go away, since its last surface has gone
    EXPECT_EQ(Application::InternalState::Closing, app->internalState());

    // Simulates that the application complied to the close() request and stopped itself
    onSessionStopping(session);
    applicationManager.onProcessStopped(appId);

    EXPECT_EQ(countSpy.count(), 2); //FIXME(greyback)
    EXPECT_EQ(applicationManager.count(), 0);
}

/*
 * Test that if the foreground Running application is stopped by upstart, AppMan cleans up after it ok
 */
TEST_F(ApplicationManagerTests,upstartNotifiesOfStoppingForegroundApp)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const pid_t procId = 5551;

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    FakeMirSurface *surface = new FakeMirSurface;
    onSessionCreatedSurface(session.get(), surface);
    surface->drawFirstFrame();

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));

    onSessionStopping(session);
    // Upstart notifies of stopping app
    applicationManager.onProcessStopped(appId);

    EXPECT_EQ(2, countSpy.count()); //FIXME(greyback)
    EXPECT_EQ(0, applicationManager.count());
}

/*
 * Test that if a running application is reported to have stopped unexpectedly by upstart, AppMan
 * cleans up after it ok (as was not suspended, had not lifecycle saved its state, so cannot be resumed)
 */
TEST_F(ApplicationManagerTests,upstartNotifiesOfUnexpectedStopOfRunningApp)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const pid_t procId = 5551;

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    FakeMirSurface *surface = new FakeMirSurface;
    onSessionCreatedSurface(session.get(), surface);
    surface->drawFirstFrame();

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));

    onSessionStopping(session);

    // Upstart notifies of crashing / OOM killed app
    applicationManager.onProcessFailed(appId, TaskController::Error::APPLICATION_CRASHED);

    // Upstart finally notifies the app stopped
    applicationManager.onProcessStopped(appId);

    EXPECT_EQ(countSpy.count(), 2); //FIXME(greyback)
    EXPECT_EQ(applicationManager.count(), 0);
}

/*
 * Test that if a background application is reported to unexpectedly stop by upstart, AppMan does not remove
 * it from the app lists but instead considers it Stopped, ready to be resumed. This is due to the fact the
 * app should have saved its state, so can be resumed. This situation can occur due to the OOM killer, or
 * a dodgy app crashing.
 */
TEST_F(ApplicationManagerTests,unexpectedStopOfBackgroundApp)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const pid_t procId = 5551;

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    Application *app = applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    FakeMirSurface *surface = new FakeMirSurface;
    onSessionCreatedSurface(session.get(), surface);
    surface->drawFirstFrame();

    suspend(app);

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));
    QSignalSpy focusSpy(&applicationManager, SIGNAL(focusedApplicationIdChanged()));

    // Mir reports disconnection
    onSessionStopping(session);

    // Upstart notifies of crashing / OOM-killed app
    applicationManager.onProcessFailed(appId, TaskController::Error::APPLICATION_CRASHED);

    EXPECT_EQ(0, focusSpy.count());

    // Upstart finally notifies the app stopped
    applicationManager.onProcessStopped(appId);

    EXPECT_EQ(0, countSpy.count());
    EXPECT_EQ(1, applicationManager.count());
}

/*
 * Test that if a background application is reported to have stopped on startup by upstart, that it
 * is kept in the application model, as the app can be lifecycle resumed.
 *
 * Note that upstart reports this "stopped on startup" even for applications which are running.
 * This may be an upstart bug, or AppMan mis-understanding upstart's intentions. But need to check
 * we're doing the right thing in either case. CHECKME(greyback)!
 */
TEST_F(ApplicationManagerTests,unexpectedStopOfBackgroundAppCheckingUpstartBug)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const pid_t procId = 5551;

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    Application *app = applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    FakeMirSurface *surface = new FakeMirSurface;
    onSessionCreatedSurface(session.get(), surface);
    surface->drawFirstFrame();

    suspend(app);

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));
    QSignalSpy focusSpy(&applicationManager, SIGNAL(focusedApplicationIdChanged()));

    // Mir reports disconnection
    onSessionStopping(session);

    // Upstart notifies of crashing app
    applicationManager.onProcessFailed(appId, TaskController::Error::APPLICATION_FAILED_TO_START);

    EXPECT_EQ(focusSpy.count(), 0);

    // Upstart finally notifies the app stopped
    applicationManager.onProcessStopped(appId);

    EXPECT_EQ(countSpy.count(), 0);
    EXPECT_EQ(applicationManager.count(), 1);
}

/*
 * Test that if a Starting application is then reported to be stopping by Mir, AppMan cleans up after it ok
 */
TEST_F(ApplicationManagerTests,mirNotifiesStartingAppIsNowStopping)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const pid_t procId = 5551;

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));

    // Mir notifies of stopping app
    onSessionStopping(session);

    EXPECT_EQ(countSpy.count(), 2); //FIXME(greyback)
    EXPECT_EQ(applicationManager.count(), 0);
}

/*
 * Test that if a Running foreground application is reported to be stopping by Mir, AppMan cleans up after it ok
 */
TEST_F(ApplicationManagerTests,mirNotifiesOfStoppingForegroundApp)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const pid_t procId = 5551;

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    // Associate a surface so AppMan considers app Running, check focused
    FakeMirSurface *surface = new FakeMirSurface;
    onSessionCreatedSurface(session.get(), surface);
    surface->drawFirstFrame();

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));

    // Mir notifies of stopping app
    onSessionStopping(session);

    EXPECT_EQ(countSpy.count(), 2); //FIXME(greyback)
    EXPECT_EQ(applicationManager.count(), 0);
}

/*
 * Test that if an application (one launched via desktop_file_hint) is reported to be stopping by
 * Mir, AppMan removes it from the model immediately
 */
TEST_F(ApplicationManagerTests,mirNotifiesOfStoppingAppLaunchedWithDesktopFileHint)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const QString name("Test App");
    const pid_t procId = 5551;
    QByteArray cmdLine("/usr/bin/testApp --desktop_file_hint=");
    cmdLine = cmdLine.append(appId);

    // Set up Mocks & signal watcher
    EXPECT_CALL(procInfo,command_line(procId))
        .Times(1)
        .WillOnce(Return(cmdLine));

    auto mockDesktopFileReader = new NiceMock<MockDesktopFileReader>(appId, QFileInfo());
    ON_CALL(*mockDesktopFileReader, loaded()).WillByDefault(Return(true));
    ON_CALL(*mockDesktopFileReader, appId()).WillByDefault(Return(appId));
    ON_CALL(*mockDesktopFileReader, name()).WillByDefault(Return(name));

    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Return(mockDesktopFileReader));

    // Mir requests authentication for an application that was started
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    EXPECT_EQ(authed, true);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    onSessionStarting(session);

    // Associate a surface so AppMan considers app Running, check focused
    FakeMirSurface *surface = new FakeMirSurface;
    onSessionCreatedSurface(session.get(), surface);
    surface->drawFirstFrame();

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));

    // Mir notifies of stopping app
    onSessionStopping(session);

    EXPECT_EQ(countSpy.count(), 2); //FIXME(greyback)
    EXPECT_EQ(applicationManager.count(), 0);

    Application *app = applicationManager.findApplication(appId);
    EXPECT_EQ(nullptr, app);
}

/*
 * Test that if a background application is reported to be stopping by Mir, AppMan sets its state to Stopped
 * but does not remove it from the model
 */
TEST_F(ApplicationManagerTests,mirNotifiesOfStoppingBackgroundApp)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const pid_t procId = 5551;

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    Application *app = applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);
    EXPECT_EQ(Application::Starting, app->state());

    app->setRequestedState(Application::RequestedSuspended);

    // should not suspend an app that`s still starting up
    ASSERT_EQ(Application::InternalState::Starting, app->internalState());

    // Associate a surface so AppMan considers app Running
    FakeMirSurface *surface = new FakeMirSurface;
    onSessionCreatedSurface(session.get(), surface);
    surface->drawFirstFrame();

    ASSERT_EQ(Application::InternalState::SuspendingWaitSession, app->internalState());

    static_cast<qtmir::Session*>(app->session())->doSuspend();
    ASSERT_EQ(Application::InternalState::SuspendingWaitProcess, app->internalState());

    applicationManager.onProcessSuspended(app->appId());
    ASSERT_EQ(Application::InternalState::Suspended, app->internalState());

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));

    // Mir notifies of stopping app
    onSessionStopping(session);

    EXPECT_EQ(0, countSpy.count());
    EXPECT_EQ(1, applicationManager.count());

    EXPECT_EQ(Application::Stopped, app->state());
}

/*
 * Test that when an application is stopped correctly by shell, the upstart stopping event is ignored
 */
TEST_F(ApplicationManagerTests,shellStoppedApp_upstartStoppingEventIgnored)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const pid_t procId = 5551;

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    Mock::VerifyAndClearExpectations(taskController);
    EXPECT_CALL(*taskController, stop(appId))
        .Times(1)
        .WillOnce(Return(true));

    applicationManager.stopApplication(appId);

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));

    // the mir session always ends before upstart notifies the process has stopped
    onSessionStopping(session);

    // Upstart notifies of stopping app
    applicationManager.onProcessStopped(appId);

    EXPECT_EQ(0, countSpy.count());
}

/*
 * Webapps have multiple sessions, but only one is linked to the application (other is considered a hidden session).
 * If webapp in foreground stops unexpectedly, remove it and it alone from app list
 */
TEST_F(ApplicationManagerTests,unexpectedStopOfForegroundWebapp)
{
    using namespace ::testing;
    const QString appId("webapp");
    const pid_t procId1 = 5551;
    const pid_t procId2 = 5564;
    QByteArray cmdLine("/usr/bin/qt5/libexec/QtWebProcess");

    // Set up Mocks & signal watcher
    EXPECT_CALL(procInfo,command_line(procId2))
        .Times(1)
        .WillOnce(Return(cmdLine));

    ON_CALL(*taskController,appIdHasProcessId(appId, procId1)).WillByDefault(Return(true));
    ON_CALL(*taskController,appIdHasProcessId(_, procId2)).WillByDefault(Return(false));

    // Set up Mocks & signal watcher
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session1 = std::make_shared<MockSession>("", procId1);
    std::shared_ptr<mir::scene::Session> session2 = std::make_shared<MockSession>("", procId2);

    bool authed = false;
    applicationManager.authorizeSession(procId1, authed);
    onSessionStarting(session1);
    EXPECT_EQ(authed, true);
    applicationManager.authorizeSession(procId2, authed);
    onSessionStarting(session2);
    EXPECT_EQ(authed, true);
    FakeMirSurface *surface = new FakeMirSurface;
    onSessionCreatedSurface(session2.get(), surface);
    surface->drawFirstFrame();

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));

    // Mir notifies of stopping app/Session
    onSessionStopping(session2);
    onSessionStopping(session1);

    EXPECT_EQ(countSpy.count(), 2); //FIXME(greyback)
    EXPECT_EQ(applicationManager.count(), 0);
}

/*
 * Webapps have multiple sessions, but only one is linked to the application (other is considered a hidden session).
 * If webapp in background stops unexpectedly, do not remove it from app list
 */
TEST_F(ApplicationManagerTests,unexpectedStopOfBackgroundWebapp)
{
    using namespace ::testing;
    const QString appId("webapp");
    const pid_t procId1 = 5551;
    const pid_t procId2 = 5564;
    QByteArray cmdLine("/usr/bin/qt5/libexec/QtWebProcess");

    // Set up Mocks & signal watcher
    EXPECT_CALL(procInfo,command_line(procId2))
        .Times(1)
        .WillOnce(Return(cmdLine));

    ON_CALL(*taskController,appIdHasProcessId(appId, procId1)).WillByDefault(Return(true));
    ON_CALL(*taskController,appIdHasProcessId(_, procId2)).WillByDefault(Return(false));

    // Set up Mocks & signal watcher
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    Application *app = applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session1 = std::make_shared<MockSession>("", procId1);
    std::shared_ptr<mir::scene::Session> session2 = std::make_shared<MockSession>("", procId2);

    bool authed = false;
    applicationManager.authorizeSession(procId1, authed);
    onSessionStarting(session1);
    EXPECT_EQ(true, authed);
    applicationManager.authorizeSession(procId2, authed);
    onSessionStarting(session2);
    EXPECT_EQ(true, authed);

    // both sessions create surfaces, then get them all suspended
    FakeMirSurface *surface1 = new FakeMirSurface;
    onSessionCreatedSurface(session1.get(), surface1);
    surface1->drawFirstFrame();
    FakeMirSurface *surface2 = new FakeMirSurface;
    onSessionCreatedSurface(session2.get(), surface2);
    surface2->drawFirstFrame();
    suspend(app);
    EXPECT_EQ(Application::Suspended, app->state());

    QSignalSpy countSpy(&applicationManager, SIGNAL(countChanged()));

    // Mir notifies of stopping app/Session
    onSessionStopping(session2);
    onSessionStopping(session1);

    EXPECT_EQ(0, countSpy.count());
}

/*
 * Test for when a background application that has been OOM killed is relaunched by upstart.
 * AppMan will have the application in the app lists, in a Stopped state. Upstart will notify of
 * the app launching (like any normal app). Need to set the old Application instance to Starting
 * state and emit focusRequested to shell - authorizeSession will then associate new process with
 * the Application as normal.
 */
TEST_F(ApplicationManagerTests,stoppedBackgroundAppRelaunchedByUpstart)
{
    using namespace ::testing;
    const QString appId("testAppId");
    const pid_t procId = 5551;

    int argc = 0;
    char* argv[0];
    QCoreApplication qtApp(argc, argv); // app for deleteLater event

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    Application *app = applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    // App creates surface, puts it in background, then is OOM killed.
    FakeMirSurface *surface = new FakeMirSurface;
    onSessionCreatedSurface(session.get(), surface);
    surface->drawFirstFrame();
    suspend(app);

    surface->setLive(false);
    onSessionStopping(session);
    applicationManager.onProcessFailed(appId, TaskController::Error::APPLICATION_CRASHED);
    applicationManager.onProcessStopped(appId);

    EXPECT_EQ(Application::Stopped, app->state());

    delete surface; surface = nullptr;

    // Session should have called deleteLater() on itself, as it's zombie and doesn't hold any surface
    // But DeferredDelete is special: likes to be called out specifically or it won't come out
    qtApp.sendPostedEvents(app->session(), QEvent::DeferredDelete);
    qtApp.sendPostedEvents();

    ASSERT_EQ(app->session(), nullptr);

    QSignalSpy focusRequestSpy(&applicationManager, SIGNAL(focusRequested(const QString &)));

    // Upstart re-launches app
    applicationManager.onProcessStarting(appId);

    EXPECT_EQ(Application::Starting, app->state());
    EXPECT_EQ(1, focusRequestSpy.count());
    EXPECT_EQ(1, applicationManager.count());
}

TEST_F(ApplicationManagerTests, lifecycleExemptAppIsNotSuspended)
{
    using namespace ::testing;

    const QString appId("testAppId");
    const quint64 procId = 12345;

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
            .Times(1)
            .WillOnce(Return(true));

    auto the_app = applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    // App creates surface, focuses it so state is running
    FakeMirSurface *surface = new FakeMirSurface;
    onSessionCreatedSurface(session.get(), surface);
    surface->drawFirstFrame();

    // Test normal lifecycle management as a control group
    ASSERT_EQ(Application::InternalState::Running, the_app->internalState());
    ASSERT_EQ(Application::ProcessState::ProcessRunning, the_app->processState());

    EXPECT_CALL(*(mir::scene::MockSession*)session.get(), set_lifecycle_state(mir_lifecycle_state_will_suspend));
    the_app->setRequestedState(Application::RequestedSuspended);
    ASSERT_EQ(Application::InternalState::SuspendingWaitSession, the_app->internalState());

    static_cast<qtmir::Session*>(the_app->session())->doSuspend();
    ASSERT_EQ(Application::InternalState::SuspendingWaitProcess, the_app->internalState());
    applicationManager.onProcessSuspended(the_app->appId());
    ASSERT_EQ(Application::InternalState::Suspended, the_app->internalState());

    EXPECT_CALL(*(mir::scene::MockSession*)session.get(), set_lifecycle_state(mir_lifecycle_state_resumed));
    the_app->setRequestedState(Application::RequestedRunning);

    EXPECT_EQ(Application::Running, the_app->state());

    // Now mark the app as exempt from lifecycle management and retest
    the_app->setExemptFromLifecycle(true);

    EXPECT_EQ(Application::Running, the_app->state());

    EXPECT_CALL(*(mir::scene::MockSession*)session.get(), set_lifecycle_state(_)).Times(0);
    the_app->setRequestedState(Application::RequestedSuspended);

    // And expect it to be running still
    ASSERT_EQ(Application::InternalState::RunningInBackground, the_app->internalState());

    the_app->setRequestedState(Application::RequestedRunning);

    EXPECT_EQ(Application::Running, the_app->state());
    ASSERT_EQ(Application::InternalState::Running, the_app->internalState());
}

/*
 * Test lifecycle exempt applications have their wakelocks released when shell tries to suspend them
 */
TEST_F(ApplicationManagerTests, lifecycleExemptAppHasWakelockReleasedOnAttemptedSuspend)
{
    using namespace ::testing;

    const QString appId("testAppId");
    const quint64 procId = 12345;

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
            .Times(1)
            .WillOnce(Return(true));

    auto application = applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    // App creates surface, focuses it so state is running
    FakeMirSurface *surface = new FakeMirSurface;
    onSessionCreatedSurface(session.get(), surface);
    surface->drawFirstFrame();

    // Mark app as exempt
    application->setExemptFromLifecycle(true);

    application->setRequestedState(Application::RequestedSuspended);

    EXPECT_FALSE(sharedWakelock.enabled());
    ASSERT_EQ(Application::InternalState::RunningInBackground, application->internalState());
    EXPECT_EQ(Application::Running, application->state());
}

/*
 * Test that when user stops an application, application does not delete QML cache
 */
TEST_F(ApplicationManagerTests,QMLcacheRetainedOnAppStop)
{
    using namespace ::testing;
    const QString appId("testAppId1234");
    const pid_t procId = 5551;

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    // Create fake QML cache for this app
    QString path(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
                 + QStringLiteral("/QML/Apps/") + appId);
    QDir dir(path);
    dir.mkpath(path);

    EXPECT_CALL(*taskController, stop(appId))
        .Times(1)
        .WillOnce(Return(true));

    // Stop app
    applicationManager.stopApplication(appId);

    EXPECT_EQ(0, applicationManager.count());
    EXPECT_TRUE(dir.exists());
}

/*
 * Test that if running application stops unexpectedly, AppMan deletes the QML cache
 */
TEST_F(ApplicationManagerTests,DISABLED_QMLcacheDeletedOnAppCrash)
{
    using namespace ::testing;
    const QString appId("testAppId12345");
    const pid_t procId = 5551;

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    Application *the_app = applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    // Have app in fully Running state
    FakeMirSurface *aSurface = new FakeMirSurface;
    onSessionCreatedSurface(session.get(), aSurface);
    aSurface->drawFirstFrame();
    ASSERT_EQ(Application::InternalState::Running, the_app->internalState());

    // Create fake QML cache for this app
    QString path(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
                 + QStringLiteral("/QML/Apps/") + appId);
    QDir dir(path);
    dir.mkpath(path);

    // Report app crash
    onSessionStopping(session);
    // Upstart notifies of **crashing** app
    applicationManager.onProcessFailed(appId, TaskController::Error::APPLICATION_FAILED_TO_START);
    applicationManager.onProcessStopped(appId);

    EXPECT_EQ(0, applicationManager.count());
    EXPECT_FALSE(dir.exists());
}

/*
 * Test that if running application stops itself cleanly, AppMan retains the QML cache
 */
TEST_F(ApplicationManagerTests,QMLcacheRetainedOnAppShutdown)
{
    using namespace ::testing;
    const QString appId("testAppId123456");
    const pid_t procId = 5551;

    // Set up Mocks & signal watcher
    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    Application *the_app = applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    // Have app in fully Running state
    FakeMirSurface *aSurface = new FakeMirSurface;
    onSessionCreatedSurface(session.get(), aSurface);
    aSurface->drawFirstFrame();
    ASSERT_EQ(Application::InternalState::Running, the_app->internalState());

    // Create fake QML cache for this app
    QString path(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
                 + QStringLiteral("/QML/Apps/") + appId);
    QDir dir(path);
    dir.mkpath(path);

    // Report app stop
    onSessionStopping(session);
    // Upstart notifies of stopping app
    applicationManager.onProcessStopped(appId);

    EXPECT_EQ(0, applicationManager.count());
    EXPECT_TRUE(dir.exists());
}

/*
 * Test that there is an attempt at polite exiting of the app by requesting closure of the surface.
 */
TEST_F(ApplicationManagerTests,requestSurfaceCloseOnStop)
{
    using namespace ::testing;

    const QString appId("testAppId");
    quint64 procId = 5551;
    Application* app = startApplication(procId, appId);
    std::shared_ptr<mir::scene::Session> session = app->session()->session();

    FakeMirSurface *surface = new FakeMirSurface;
    onSessionCreatedSurface(session.get(), surface);
    surface->drawFirstFrame();

    QSignalSpy spy(surface, SIGNAL(closeRequested()));

    // Stop app
    applicationManager.stopApplication(appId);

    EXPECT_EQ(1, spy.count());
}


//  * Test that if there is no surface available to the app when it is stopped, that it is forced to close.
TEST_F(ApplicationManagerTests,forceAppDeleteWhenRemovedWithMissingSurface)
{
    using namespace ::testing;

    int argc = 0;
    char* argv[0];
    QCoreApplication qtApp(argc, argv); // app for deleteLater event

    const QString appId("testAppId");
    quint64 procId = 5551;

    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    auto app = applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    Mock::VerifyAndClearExpectations(taskController);

    // Stop app

    QSignalSpy appDestroyedSpy(app, SIGNAL(destroyed(QObject*)));

    EXPECT_CALL(*taskController, stop(appId))
        .Times(1)
        .WillOnce(Return(true));

    applicationManager.stopApplication(appId);

    // the mir session always ends before upstart notifies the process has stopped
    onSessionStopping(session);

    // Upstart notifies of stopping app
    applicationManager.onProcessStopped(appId);

    // DeferredDelete is special: likes to be called out specifically or it won't come out
    qtApp.sendPostedEvents(app, QEvent::DeferredDelete);
    qtApp.sendPostedEvents();

    EXPECT_EQ(1, appDestroyedSpy.count());
}

/*
 * Test that if an application is started while it is still attempting to close, it is queued to start again.
 */
TEST_F(ApplicationManagerTests,applicationStartQueuedOnStartStopStart)
{
    using namespace ::testing;

    int argc = 0;
    char* argv[0];
    QCoreApplication qtApp(argc, argv); // app for deleteLater event

    const QString appId("testAppId");
    quint64 procId = 5551;

    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    auto app = applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    FakeMirSurface *surface = new FakeMirSurface;
    onSessionCreatedSurface(session.get(), surface);
    surface->drawFirstFrame();

    EXPECT_EQ(Application::InternalState::Running, app->internalState());

    // Stop app

    Mock::VerifyAndClearExpectations(taskController);

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    QSignalSpy closeRequestedSpy(surface, SIGNAL(closeRequested()));

    applicationManager.stopApplication(appId);

    // Asking ApplicationManager to stop the application just makes it request its surfaces to be
    // closed
    EXPECT_EQ(1, closeRequestedSpy.count());

    // comply
    delete surface;
    surface = nullptr;

    // now it's the turn of the application process itself to go away, since its last surface has gone
    EXPECT_EQ(Application::InternalState::Closing, app->internalState());

    // Trying to start a new instance of this app while we are still waiting for its current
    // instance to end yields no immediate result. This command gets queued instead.
    EXPECT_EQ(nullptr, applicationManager.startApplication(appId));

    QSignalSpy appAddedSpy(&applicationManager, &QAbstractItemModel::rowsInserted);

    // Simulates that the application complied to the close() request and stopped itself
    onSessionStopping(session);
    applicationManager.onProcessStopped(appId);

    // DeferredDelete is special: likes to be called out specifically or it won't come out
    qtApp.sendPostedEvents(app, QEvent::DeferredDelete);
    qtApp.sendPostedEvents();

    EXPECT_EQ(1, appAddedSpy.count());
}

/*
  Change focus between surfaces of different applications and check that
  ApplicationManager::focusedApplicationId changes accordingly
 */
TEST_F(ApplicationManagerTests,focusedApplicationId)
{
    using namespace ::testing;

    int argc = 0;
    char* argv[0];
    QCoreApplication qtApp(argc, argv);

    const QString appId1("testAppId1");
    quint64 procId1 = 5551;
    const QString appId2("testAppId2");
    quint64 procId2 = 5552;

    ON_CALL(*taskController, primaryPidForAppId(appId1)).WillByDefault(Return(procId1));
    ON_CALL(desktopFileReaderFactory, createInstance(appId1, _)).WillByDefault(Invoke(createMockDesktopFileReader));
    ON_CALL(*taskController, primaryPidForAppId(appId2)).WillByDefault(Return(procId2));
    ON_CALL(desktopFileReaderFactory, createInstance(appId2, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId1, _))
        .Times(1)
        .WillOnce(Return(true));

    auto app1 = applicationManager.startApplication(appId1);
    applicationManager.onProcessStarting(appId1);
    std::shared_ptr<mir::scene::Session> session1 = std::make_shared<MockSession>("", procId1);
    bool authed = true;
    applicationManager.authorizeSession(procId1, authed);
    onSessionStarting(session1);

    FakeMirSurface *surface1 = new FakeMirSurface;
    surface1->setSession(app1->session());
    onSessionCreatedSurface(session1.get(), surface1);
    surface1->drawFirstFrame();

    EXPECT_EQ(Application::InternalState::Running, app1->internalState());

    QSignalSpy focusedApplicationIdChangedSpy(&applicationManager,
            &unityapi::ApplicationManagerInterface::focusedApplicationIdChanged);

    MirFocusController::instance()->setFocusedSurface(surface1);
    qtApp.processEvents(); // process queued signal-slot connections

    EXPECT_EQ(1, focusedApplicationIdChangedSpy.count());
    EXPECT_EQ(appId1, applicationManager.focusedApplicationId());

    EXPECT_CALL(*taskController, start(appId2, _))
        .Times(1)
        .WillOnce(Return(true));

    auto app2 = applicationManager.startApplication(appId2);
    applicationManager.onProcessStarting(appId2);
    std::shared_ptr<mir::scene::Session> session2 = std::make_shared<MockSession>("", procId2);
    authed = true;
    applicationManager.authorizeSession(procId2, authed);
    onSessionStarting(session2);

    FakeMirSurface *surface2 = new FakeMirSurface;
    surface2->setSession(app2->session());
    onSessionCreatedSurface(session2.get(), surface2);
    surface2->drawFirstFrame();

    EXPECT_EQ(Application::InternalState::Running, app2->internalState());

    MirFocusController::instance()->setFocusedSurface(surface2);
    qtApp.processEvents(); // process queued signal-slot connections

    EXPECT_EQ(2, focusedApplicationIdChangedSpy.count());
    EXPECT_EQ(appId2, applicationManager.focusedApplicationId());

    MirFocusController::instance()->setFocusedSurface(surface1);
    qtApp.processEvents(); // process queued signal-slot connections

    EXPECT_EQ(3, focusedApplicationIdChangedSpy.count());
    EXPECT_EQ(appId1, applicationManager.focusedApplicationId());

    // clean up
    delete surface1;
    delete surface2;
}

/*
 This is the compatibility mode between application-centric window management
 and the new (proper) surface-based window management

 Whenever a surface request focus, its corresponding application should do likewise,
 causing ApplicationManager::focusRequested(appId) to be emitted as well.
 */
TEST_F(ApplicationManagerTests,surfaceFocusRequestGeneratesApplicationFocusRequest)
{
    using namespace ::testing;

    const QString appId("testAppId");
    quint64 procId = 5551;

    ON_CALL(*taskController, primaryPidForAppId(appId)).WillByDefault(Return(procId));
    ON_CALL(desktopFileReaderFactory, createInstance(appId, _)).WillByDefault(Invoke(createMockDesktopFileReader));

    EXPECT_CALL(*taskController, start(appId, _))
        .Times(1)
        .WillOnce(Return(true));

    auto app = applicationManager.startApplication(appId);
    applicationManager.onProcessStarting(appId);
    std::shared_ptr<mir::scene::Session> session = std::make_shared<MockSession>("", procId);
    bool authed = true;
    applicationManager.authorizeSession(procId, authed);
    onSessionStarting(session);

    FakeMirSurface *surface = new FakeMirSurface;
    onSessionCreatedSurface(session.get(), surface);
    surface->drawFirstFrame();

    EXPECT_EQ(Application::InternalState::Running, app->internalState());

    QSignalSpy focusRequestedSpy(&applicationManager,
            &unityapi::ApplicationManagerInterface::focusRequested);

    surface->requestFocus();

    EXPECT_EQ(1, focusRequestedSpy.count());
}
