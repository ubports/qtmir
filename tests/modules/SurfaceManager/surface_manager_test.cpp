/*
 * Copyright (C) 2017 Canonical, Ltd.
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

#include <QLoggingCategory>
#include <QSignalSpy>

// the test subject
#include <Unity/Application/surfacemanager.h>
#include <Unity/Application/mirsurface.h>

// miral
#include <miral/window.h>
#include <miral/window_info.h>

// mirtest
#include <mir/test/doubles/stub_session.h>
#include <mir/test/doubles/stub_surface.h>

// local
#include "qtmir_test.h"
#include "fake_session.h"
#include "mock_session_manager.h"
#include "mock_window_controller.h"

using namespace qtmir;
using StubSurface = mir::test::doubles::StubSurface;
using StubSession = mir::test::doubles::StubSession;

struct TestableSurfaceManager : public SurfaceManager
{
    // just exports the test-only protected constructor
    TestableSurfaceManager(WindowControllerInterface *windowController,
                           WindowModelNotifier *windowModel,
                           SessionManager *sessionManager)
        : SurfaceManager(windowController, windowModel, sessionManager) {}

    MirSurface* find(const miral::WindowInfo &needle) const
    {
        return SurfaceManager::find(needle);
    }
};

class SurfaceManagerTests : public ::testing::Test
{
public:
    SurfaceManagerTests()
    {
        // We don't want the logging spam cluttering the test results
        QLoggingCategory::setFilterRules(QStringLiteral("qtmir.*=false"));

        qRegisterMetaType<unity::shell::application::MirSurfaceInterface*>();
        qRegisterMetaType<QVector<unity::shell::application::MirSurfaceInterface*>>();
    }

    testing::NiceMock<MockSessionManager> sessionManager;
    testing::NiceMock<MockWindowController> wmController;
    WindowModelNotifier wmNotifier;
    QScopedPointer<TestableSurfaceManager> surfaceManager;
    QScopedPointer<QCoreApplication> qtApp; // need to spin event loop for queued connections

    // Needed to create miral::WindowInfo
    const std::shared_ptr<StubSession> stubSession{std::make_shared<StubSession>()};
    const std::shared_ptr<StubSurface> stubSurface{std::make_shared<StubSurface>()};
    const miral::Window window{stubSession, stubSurface};
    const ms::SurfaceCreationParameters spec;
    const miral::WindowInfo windowInfo{window, spec};
    FakeSession fakeSession;

protected:
    void SetUp() override
    {
        int argc = 0;
        char* argv[0];
        qtApp.reset(new QCoreApplication(argc, argv));

        using namespace ::testing;
        ON_CALL(sessionManager,findSession(_))
                .WillByDefault(Return(&fakeSession));

        surfaceManager.reset(new TestableSurfaceManager(&wmController,
                                                        &wmNotifier, &sessionManager));
    }
};

/*
 * Test if MirAL notifies that a window was created, SurfaceManager emits the surfaceCreated
 * signal
 */
TEST_F(SurfaceManagerTests, miralWindowCreationCausesSignalEmission)
{
    QSignalSpy newMirSurfaceSpy(surfaceManager.data(), &SurfaceManager::surfaceCreated);

    // Test
    Q_EMIT wmNotifier.windowAdded(windowInfo);
    qtApp->sendPostedEvents();

    // Check result
    EXPECT_EQ(1, newMirSurfaceSpy.count());
}


/*
 * Test if MirAL notifies that a window was created, SurfaceManager emits the surfaceCreated
 * signal with a corresponding MirSurface
 */
TEST_F(SurfaceManagerTests, miralWindowCreationCausesMirSurfaceCreation)
{
    QSignalSpy newMirSurfaceSpy(surfaceManager.data(), &SurfaceManager::surfaceCreated);

    // Test
    Q_EMIT wmNotifier.windowAdded(windowInfo);
    qtApp->sendPostedEvents();

    // Check result
    auto mirSurface = qvariant_cast<MirSurface*>(newMirSurfaceSpy.takeFirst().at(0));
    ASSERT_TRUE(mirSurface);
    EXPECT_EQ(window, mirSurface->window());
}

/*
 * Test if MirAL notifies that a window was created, SurfaceManager adds the corresponding
 * MirSurface to its internal list
 */
TEST_F(SurfaceManagerTests, miralWindowCreationAddsMirSurfaceToItsInternalList)
{
    // Test
    Q_EMIT wmNotifier.windowAdded(windowInfo);
    qtApp->sendPostedEvents();

    // Check result
    auto mirSurface = surfaceManager->find(windowInfo);
    ASSERT_TRUE(mirSurface);
    EXPECT_EQ(window, mirSurface->window());
}

/*
 * Test SurfaceManager creates a MirSurface with a Session associated
 */
TEST_F(SurfaceManagerTests, createdMirSurfaceHasSessionSet)
{
    // Setup
    using namespace ::testing;
    EXPECT_CALL(sessionManager,findSession(_))
            .WillOnce(Return(&fakeSession));

    // Test
    Q_EMIT wmNotifier.windowAdded(windowInfo);
    qtApp->sendPostedEvents();

    // Check result
    auto mirSurface = surfaceManager->find(windowInfo);
    ASSERT_TRUE(mirSurface);
    EXPECT_EQ(&fakeSession, mirSurface->session());
}

/*
 * Test when MirAL creates a surface with a parent, SurfaceManager correctly associates
 * the parent MirSurface to the MirSurface
 */
TEST_F(SurfaceManagerTests, parentedMiralWindowGeneratesMirSurfaceWithCorrectParent)
{
    // Setup
    miral::Window parentWindow(stubSession, stubSurface);
    miral::Window childWindow(stubSession, stubSurface);
    miral::WindowInfo parentWindowInfo(parentWindow, spec);
    miral::WindowInfo childWindowInfo(childWindow, spec);
    childWindowInfo.parent(parentWindow);

    // Test
    Q_EMIT wmNotifier.windowAdded(parentWindowInfo);
    Q_EMIT wmNotifier.windowAdded(childWindowInfo);
    qtApp->sendPostedEvents();

    // Check result
    auto childMirSurface = surfaceManager->find(childWindowInfo);
    auto parentMirSurface = surfaceManager->find(parentWindowInfo);
    ASSERT_TRUE(childMirSurface);
    ASSERT_TRUE(parentMirSurface);

    EXPECT_EQ(parentMirSurface, childMirSurface->parentSurface());
}

/*
 * Test when MirAL creates a surface with a parent, SurfaceManager correctly associates
 * the child MirSurface to the parent MirSurface
 */
TEST_F(SurfaceManagerTests, miralWindowWithChildHasMirSurfaceWithCorrectChild)
{
    // Setup
    miral::Window parentWindow(stubSession, stubSurface);
    miral::Window childWindow(stubSession, stubSurface);
    miral::WindowInfo parentWindowInfo(parentWindow, spec);
    miral::WindowInfo childWindowInfo(childWindow, spec);
    childWindowInfo.parent(parentWindow);

    // Test
    Q_EMIT wmNotifier.windowAdded(parentWindowInfo);
    Q_EMIT wmNotifier.windowAdded(childWindowInfo);
    qtApp->sendPostedEvents();

    // Check result
    auto childMirSurface = surfaceManager->find(childWindowInfo);
    auto parentMirSurface = surfaceManager->find(parentWindowInfo);
    ASSERT_TRUE(childMirSurface);
    ASSERT_TRUE(parentMirSurface);

    ASSERT_EQ(1, parentMirSurface->childSurfaceList()->count());
    EXPECT_EQ(childMirSurface, parentMirSurface->childSurfaceList()->first());
}

/*
 * Test if MirAL notifies that a window is ready, SurfaceManager updates the corresponding
 * MirSurface causing it to emit a ready() signal
 */
TEST_F(SurfaceManagerTests, miralWindowReadyUpdatesMirSurfaceState)
{
    // Setup: add window and get corresponding MirSurface
    Q_EMIT wmNotifier.windowAdded(windowInfo);
    qtApp->sendPostedEvents();
    auto mirSurface = surfaceManager->find(windowInfo);
    ASSERT_TRUE(mirSurface);

    QSignalSpy mirSurfaceReadySpy(mirSurface, &MirSurface::ready);

    // Test
    Q_EMIT wmNotifier.windowReady(windowInfo);
    qtApp->sendPostedEvents();

    // Check result
    EXPECT_EQ(1, mirSurfaceReadySpy.count());
}

/*
 * Test if MirAL notifies that a window is moved, SurfaceManager updates the corresponding
 * MirSurface position
 */
TEST_F(SurfaceManagerTests, miralWindowMoveUpdatesMirSurfacePosition)
{
    QPoint newPosition(222,333);

    // Setup: add window and get corresponding MirSurface
    Q_EMIT wmNotifier.windowAdded(windowInfo);
    qtApp->sendPostedEvents();
    auto mirSurface = surfaceManager->find(windowInfo);
    ASSERT_TRUE(mirSurface);

    QSignalSpy mirSurfacePositionSpy(mirSurface, &MirSurface::positionChanged);

    // Test
    Q_EMIT wmNotifier.windowMoved(windowInfo, newPosition);
    qtApp->sendPostedEvents();

    // Check result
    EXPECT_EQ(1, mirSurfacePositionSpy.count());
    EXPECT_EQ(mirSurface->position(), newPosition);
}

/*
 * Test if MirAL notifies that a window's focus state changes, SurfaceManager updates the corresponding
 * MirSurface focus state
 */
TEST_F(SurfaceManagerTests, miralWindowFocusChangeUpdatesMirSurfaceFocus)
{
    // Setup: add window and get corresponding MirSurface
    Q_EMIT wmNotifier.windowAdded(windowInfo);
    qtApp->sendPostedEvents();
    auto mirSurface = surfaceManager->find(windowInfo);
    ASSERT_TRUE(mirSurface);
    ASSERT_FALSE(mirSurface->focused()); // false must be the initial state

    QSignalSpy mirSurfaceFocusSpy(mirSurface, &MirSurface::focusedChanged);

    // Test
    Q_EMIT wmNotifier.windowFocusChanged(windowInfo, true);
    qtApp->sendPostedEvents();

    // Check result
    EXPECT_EQ(1, mirSurfaceFocusSpy.count());
    EXPECT_EQ(mirSurface->focused(), true);
}

/*
 * Test if MirAL notifies that a window's state changes, SurfaceManager updates the corresponding
 * MirSurface state (just testing a single state, see no value in testing all possible states here)
 */
TEST_F(SurfaceManagerTests, miralWindowStateChangeUpdatesMirSurfaceState)
{
    auto newState = Mir::FullscreenState;

    // Setup: add window and get corresponding MirSurface
    Q_EMIT wmNotifier.windowAdded(windowInfo);
    qtApp->sendPostedEvents();
    auto mirSurface = surfaceManager->find(windowInfo);
    ASSERT_TRUE(mirSurface);

    QSignalSpy mirSurfaceStateSpy(mirSurface, &MirSurface::stateChanged);

    // Test
    Q_EMIT wmNotifier.windowStateChanged(windowInfo, newState);
    qtApp->sendPostedEvents();

    // Check result
    EXPECT_EQ(1, mirSurfaceStateSpy.count());
    EXPECT_EQ(mirSurface->state(), newState);
}

/*
 * Test when miral raises a list of surfaces, the raise signal is fired by SurfaceManager with
 * a list of MirSurfaces in the matching order
 */
TEST_F(SurfaceManagerTests, miralsRaiseWindowListTransformedToVectorOfMirSurfaces)
{
    // Setup
    miral::Window window1(stubSession, stubSurface);
    miral::Window window2(stubSession, stubSurface);
    miral::WindowInfo windowInfo1(window1, spec);
    miral::WindowInfo windowInfo2(window2, spec);

    // Setup: add 2 windows and get their MirSurfaces
    Q_EMIT wmNotifier.windowAdded(windowInfo1);
    Q_EMIT wmNotifier.windowAdded(windowInfo2);
    qtApp->sendPostedEvents();
    auto mirSurface1 = surfaceManager->find(windowInfo1);
    auto mirSurface2 = surfaceManager->find(windowInfo2);
    ASSERT_TRUE(mirSurface1);
    ASSERT_TRUE(mirSurface2);

    QSignalSpy mirSurfacesRaisedSpy(surfaceManager.data(), &SurfaceManager::surfacesRaised);

    // Test
    std::vector<miral::Window> raiseWindowList{window2, window1};
    Q_EMIT wmNotifier.windowsRaised(raiseWindowList);
    qtApp->sendPostedEvents();

    // Check results
    ASSERT_EQ(1, mirSurfacesRaisedSpy.count());
    auto raiseMirSurfaceList = qvariant_cast<QVector<unity::shell::application::MirSurfaceInterface*>>(
                                                 mirSurfacesRaisedSpy.takeFirst().at(0)); // first argument of signal
    ASSERT_EQ(2, raiseMirSurfaceList.count());
    EXPECT_EQ(mirSurface1, raiseMirSurfaceList.at(1));
    EXPECT_EQ(mirSurface2, raiseMirSurfaceList.at(0));
}

/*
 * Test focus requests fire focusRequested signal of the MirSurface
 */
TEST_F(SurfaceManagerTests, focusRequestCausesMirSurfaceToFireFocusRequestedSignal)
{
    // Setup: add window and get corresponding MirSurface
    Q_EMIT wmNotifier.windowAdded(windowInfo);
    qtApp->sendPostedEvents();
    auto mirSurface = surfaceManager->find(windowInfo);
    ASSERT_TRUE(mirSurface);

    QSignalSpy mirSurfaceFocusRequestedSpy(mirSurface, &MirSurface::focusRequested);

    // Test
    Q_EMIT wmNotifier.windowRequestedRaise(windowInfo);
    qtApp->sendPostedEvents();

    // Check result
    EXPECT_EQ(1, mirSurfaceFocusRequestedSpy.count());
}

/*
 * If MirAL notifies that a window was removed, and its corresponding MirSurface is not
 * being displayed, test that SurfaceManager removes the corresponding MirSurface from
 * its internal list and deletes the MirSurface
 */
TEST_F(SurfaceManagerTests, miralWindowRemovedDeletesSurfaceManagerInternalEntryAndMirSurface)
{
    // Setup: add window and get corresponding MirSurface
    Q_EMIT wmNotifier.windowAdded(windowInfo);
    qtApp->sendPostedEvents();
    auto mirSurface = surfaceManager->find(windowInfo);
    ASSERT_TRUE(mirSurface);

    QSignalSpy mirSurfaceDestroyedSpy(mirSurface, &QObject::destroyed);

    // Test
    Q_EMIT wmNotifier.windowRemoved(windowInfo);
    qtApp->sendPostedEvents();

    // Check result
    ASSERT_EQ(2, mirSurfaceDestroyedSpy.count()); //FIXME - should be 1
    EXPECT_FALSE(surfaceManager->find(windowInfo));
}

/*
 * If MirAL notifies that a window was removed, and its corresponding MirSurface *is*
 * being displayed, test that SurfaceManager removes the corresponding MirSurface from
 * its internal list but does *not* delete the MirSurface, but sets it as not "live"
 */
TEST_F(SurfaceManagerTests, miralWindowRemovedDeletesSurfaceManagerInternalEntryButNotMirSurface)
{
    // Setup: add window and get corresponding MirSurface
    Q_EMIT wmNotifier.windowAdded(windowInfo);
    qtApp->sendPostedEvents();
    auto mirSurface = surfaceManager->find(windowInfo);
    ASSERT_TRUE(mirSurface);

    QSignalSpy mirSurfaceDestroyedSpy(mirSurface, &QObject::destroyed);

    mirSurface->registerView(1);

    // Test
    Q_EMIT wmNotifier.windowRemoved(windowInfo);
    qtApp->sendPostedEvents();

    // Check result
    ASSERT_EQ(0, mirSurfaceDestroyedSpy.count());
    EXPECT_FALSE(surfaceManager->find(windowInfo));
    EXPECT_FALSE(mirSurface->live());
}

/*
 * If MirAL notifies that a window was removed, and its corresponding MirSurface *is*
 * being displayed, SurfaceManager does *not* delete the MirSurface. Later when that
 * MirSurface is not being displayed any more, SurfaceManager should delete it.
 */
TEST_F(SurfaceManagerTests, miralWindowRemovedSurfaceManagerDeletesMirSurfaceWhenItDoneWith)
{
    int viewId = 99;

    // Setup: add window and get corresponding MirSurface
    Q_EMIT wmNotifier.windowAdded(windowInfo);
    qtApp->sendPostedEvents();
    auto mirSurface = surfaceManager->find(windowInfo);
    ASSERT_TRUE(mirSurface);

    QSignalSpy mirSurfaceDestroyedSpy(mirSurface, &QObject::destroyed);

    // Setup: indicate MirSurface is being displayed
    mirSurface->registerView(viewId);

    // Setup: notify that window removed
    Q_EMIT wmNotifier.windowRemoved(windowInfo);
    qtApp->sendPostedEvents();

    // Test
    mirSurface->unregisterView(viewId);

    // Check result
    ASSERT_EQ(2, mirSurfaceDestroyedSpy.count()); //FIXME - should be 1
}

/*
 * Test that if a MirSurface is live, and stops being displayed, SurfaceManager does *not*
 * delete the MirSurface.
 */
TEST_F(SurfaceManagerTests, surfaceManagerDoesNotDeleteLiveMirSurfaceWhenStopsBeingDisplayed)
{
    int viewId = 99;

    // Setup: add window and get corresponding MirSurface
    Q_EMIT wmNotifier.windowAdded(windowInfo);
    qtApp->sendPostedEvents();
    auto mirSurface = surfaceManager->find(windowInfo);
    ASSERT_TRUE(mirSurface);
    ASSERT_TRUE(mirSurface->live());

    QSignalSpy mirSurfaceDestroyedSpy(mirSurface, &QObject::destroyed);

    // Setup: indicate MirSurface is being displayed
    mirSurface->registerView(viewId);

    // Test
    mirSurface->unregisterView(viewId);

    // Check result
    ASSERT_EQ(0, mirSurfaceDestroyedSpy.count());
}
