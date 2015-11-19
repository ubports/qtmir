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

#include <qtmir_test.h>
#include <fake_mirsurface.h>

#include <Unity/Application/application.h>
#include <Unity/Application/mirsurfaceitem.h>

#include "stub_scene_surface.h"

using namespace qtmir;
using mir::scene::MockSession;

namespace ms = mir::scene;
namespace mtd = mir::test::doubles;

class SessionTests : public ::testing::QtMirTest
{
public:
    SessionTests()
    {}

    QList<SessionInterface*> listChildSessions(Session* session) {
        QList<SessionInterface*> sessions;
        session->foreachChildSession([&sessions](SessionInterface* session) {
            sessions << session;
        });
        return sessions;
    }
};

TEST_F(SessionTests, FromStartingToRunningOnceSurfaceDrawsFirstFrame)
{
    using namespace testing;

    const QString appId("test-app");
    const pid_t procId = 5551;

    auto mirSession = std::make_shared<MockSession>(appId.toStdString(), procId);

    auto session = std::make_shared<qtmir::Session>(mirSession, mirServer->the_prompt_session_manager());

    // On Starting as it has no surface.
    EXPECT_EQ(Session::Starting, session->state());

    FakeMirSurface *surface = new FakeMirSurface;
    session->setSurface(surface);

    // Still on Starting as the surface hasn't drawn its first frame yet
    EXPECT_EQ(Session::Starting, session->state());

    surface->drawFirstFrame();
    EXPECT_EQ(Session::Running, session->state());
}

TEST_F(SessionTests, AddChildSession)
{
    using namespace testing;

    const QString appId("test-app");
    const pid_t procId = 5551;

    std::shared_ptr<ms::Session> mirSession = std::make_shared<MockSession>(appId.toStdString(), procId);

    Session session(mirSession, mirServer->the_prompt_session_manager());
    Session session1(mirSession, mirServer->the_prompt_session_manager());
    Session session2(mirSession, mirServer->the_prompt_session_manager());
    Session session3(mirSession, mirServer->the_prompt_session_manager());

    // add surfaces
    session.addChildSession(&session1);
    EXPECT_EQ(session1.parentSession(), &session);
    EXPECT_THAT(listChildSessions(&session), ElementsAre(&session1));

    session.addChildSession(&session2);
    EXPECT_EQ(session2.parentSession(), &session);
    EXPECT_THAT(listChildSessions(&session), ElementsAre(&session1, &session2));

    session.addChildSession(&session3);
    EXPECT_EQ(session3.parentSession(), &session);
    EXPECT_THAT(listChildSessions(&session), ElementsAre(&session1, &session2, &session3));
}

TEST_F(SessionTests, InsertChildSession)
{
    using namespace testing;

    const QString appId("test-app");
    const pid_t procId = 5551;

    std::shared_ptr<ms::Session> mirSession = std::make_shared<MockSession>(appId.toStdString(), procId);

    Session session(mirSession, mirServer->the_prompt_session_manager());
    Session session1(mirSession, mirServer->the_prompt_session_manager());
    Session session2(mirSession, mirServer->the_prompt_session_manager());
    Session session3(mirSession, mirServer->the_prompt_session_manager());

    // add surfaces
    session.insertChildSession(100, &session1); // test overflow
    EXPECT_EQ(session1.parentSession(), &session);
    EXPECT_THAT(listChildSessions(&session), ElementsAre(&session1));

    session.insertChildSession(0, &session2); // test insert before
    EXPECT_EQ(session2.parentSession(), &session);
    EXPECT_THAT(listChildSessions(&session), ElementsAre(&session2, &session1));

    session.insertChildSession(1, &session3); // test before end
    EXPECT_EQ(session3.parentSession(), &session);
    EXPECT_THAT(listChildSessions(&session), ElementsAre(&session2, &session3, &session1));
}

TEST_F(SessionTests, RemoveChildSession)
{
    using namespace testing;

    const QString appId("test-app");
    const pid_t procId = 5551;

    std::shared_ptr<ms::Session> mirSession = std::make_shared<MockSession>(appId.toStdString(), procId);

    Session session(mirSession, mirServer->the_prompt_session_manager());
    Session session1(mirSession, mirServer->the_prompt_session_manager());
    Session session2(mirSession, mirServer->the_prompt_session_manager());
    Session session3(mirSession, mirServer->the_prompt_session_manager());

    // add surfaces
    session.addChildSession(&session1);
    session.addChildSession(&session2);
    session.addChildSession(&session3);

    // remove surfaces
    session.removeChildSession(&session2);
    EXPECT_EQ(session2.parentSession(), nullptr);
    EXPECT_THAT(listChildSessions(&session), ElementsAre(&session1, &session3));

    session.removeChildSession(&session3);
    EXPECT_EQ(session3.parentSession(), nullptr);
    EXPECT_THAT(listChildSessions(&session), ElementsAre(&session1));

    session.removeChildSession(&session1);
    EXPECT_EQ(session1.parentSession(), nullptr);
    EXPECT_THAT(listChildSessions(&session), IsEmpty());
}

TEST_F(SessionTests, DeleteChildSessionRemovesFromApplication)
{
    using namespace testing;

    const QString appId("test-app");
    const pid_t procId = 5551;

    std::shared_ptr<ms::Session> mirSession = std::make_shared<MockSession>(appId.toStdString(), procId);

    Session session(mirSession, mirServer->the_prompt_session_manager());
    Session* session1 = new Session(mirSession, mirServer->the_prompt_session_manager());
    Session* session2 = new Session(mirSession, mirServer->the_prompt_session_manager());
    Session* session3 = new Session(mirSession, mirServer->the_prompt_session_manager());

    // add surfaces
    session.addChildSession(session1);
    session.addChildSession(session2);
    session.addChildSession(session3);

    // delete surfaces
    delete session2;
    EXPECT_THAT(listChildSessions(&session), ElementsAre(session1, session3));

    // delete surfaces
    delete session3;
    EXPECT_THAT(listChildSessions(&session), ElementsAre(session1));

    // delete surfaces
    delete session1;
    EXPECT_THAT(listChildSessions(&session), IsEmpty());
}

TEST_F(SessionTests, DeleteSessionDeletesChildSessions)
{
    using namespace testing;

    const QString appId("test-app");
    const pid_t procId = 5551;

    std::shared_ptr<ms::Session> mirSession = std::make_shared<MockSession>(appId.toStdString(), procId);

    Session* session = new Session(mirSession, mirServer->the_prompt_session_manager());
    Session* session1 = new Session(mirSession, mirServer->the_prompt_session_manager());
    Session* session2 = new Session(mirSession, mirServer->the_prompt_session_manager());
    Session* session3 = new Session(mirSession, mirServer->the_prompt_session_manager());

    // add surfaces
    session->addChildSession(session1);
    session->addChildSession(session2);
    session->addChildSession(session3);

    QList<QObject*> destroyed;
    QObject::connect(session1, &QObject::destroyed, [&](QObject*) { destroyed << session1; });
    QObject::connect(session2, &QObject::destroyed, [&](QObject*) { destroyed << session2; });
    QObject::connect(session3, &QObject::destroyed, [&](QObject*) { destroyed << session3; });

    delete session;
    EXPECT_THAT(destroyed, UnorderedElementsAre(session1, session2, session3));
}

TEST_F(SessionTests, SuspendPromptSessionWhenSessionSuspends)
{
    using namespace testing;

    const QString appId("test-app");
    const pid_t procId = 5551;

    auto mirSession = std::make_shared<MockSession>(appId.toStdString(), procId);

    auto session = std::make_shared<qtmir::Session>(mirSession, mirServer->the_prompt_session_manager());
    {
        FakeMirSurface *surface = new FakeMirSurface;
        session->setSurface(surface);
        surface->drawFirstFrame();
    }
    EXPECT_EQ(Session::Running, session->state());

    auto mirPromptSession = std::make_shared<ms::MockPromptSession>();
    session->appendPromptSession(mirPromptSession);

    EXPECT_CALL(*mirServer->the_mock_prompt_session_manager(), suspend_prompt_session(_)).Times(1);

    EXPECT_CALL(*mirSession, set_lifecycle_state(mir_lifecycle_state_will_suspend));
    session->suspend();
    EXPECT_EQ(Session::Suspending, session->state());
    session->doSuspend();
    EXPECT_EQ(Session::Suspended, session->state());

    Mock::VerifyAndClear(mirServer->the_mock_prompt_session_manager().get());
}

TEST_F(SessionTests, ResumePromptSessionWhenSessionResumes)
{
    using namespace testing;

    const QString appId("test-app");
    const pid_t procId = 5551;

    auto mirSession = std::make_shared<MockSession>(appId.toStdString(), procId);

    auto session = std::make_shared<qtmir::Session>(mirSession, mirServer->the_prompt_session_manager());
    {
        FakeMirSurface *surface = new FakeMirSurface;
        session->setSurface(surface);
        surface->drawFirstFrame();
    }
    EXPECT_EQ(Session::Running, session->state());

    EXPECT_CALL(*mirSession, set_lifecycle_state(mir_lifecycle_state_will_suspend));
    session->suspend();
    EXPECT_EQ(Session::Suspending, session->state());
    session->doSuspend();
    EXPECT_EQ(Session::Suspended, session->state());

    auto mirPromptSession = std::make_shared<ms::MockPromptSession>();
    session->appendPromptSession(mirPromptSession);

    EXPECT_CALL(*mirServer->the_mock_prompt_session_manager(), resume_prompt_session(_)).Times(1);

    EXPECT_CALL(*mirSession, set_lifecycle_state(mir_lifecycle_state_resumed));
    session->resume();
    EXPECT_EQ(Session::Running, session->state());

    Mock::VerifyAndClear(mirServer->the_mock_prompt_session_manager().get());
}
