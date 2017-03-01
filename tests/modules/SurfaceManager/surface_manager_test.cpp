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
        //QLoggingCategory::setFilterRules(QStringLiteral("qtmir.*=false"));

        qRegisterMetaType<unity::shell::application::MirSurfaceInterface*>("MirSurfaceInterface*");
    }

    testing::NiceMock<MockSessionManager> sessionManager;
    testing::NiceMock<MockWindowController> wmController;
    WindowModelNotifier wmNotifier;
    QScopedPointer<TestableSurfaceManager> surfaceManager;
    QScopedPointer<QCoreApplication> qtApp; // need to spin event loop for queued connections

    // Needed to create miral::WindowInfo
    const std::shared_ptr<StubSession> stubSession{std::make_shared<StubSession>()};
    const std::shared_ptr<StubSurface> stubSurface{std::make_shared<StubSurface>()};

protected:
    void SetUp() override {
        int argc = 0;
        char* argv[0];
        qtApp.reset(new QCoreApplication(argc, argv));

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
    // Setup
    miral::Window window(stubSession, stubSurface);
    ms::SurfaceCreationParameters spec;
    miral::WindowInfo windowInfo(window, spec);

    QSignalSpy newMirSurfaceSpy(surfaceManager.data(), &SurfaceManager::surfaceCreated);

    // Test
    Q_EMIT wmNotifier.windowAdded(windowInfo);
    qtApp->sendPostedEvents();

    // Check result
    ASSERT_EQ(1, newMirSurfaceSpy.count());
}


/*
 * Test if MirAL notifies that a window was created, SurfaceManager emits the surfaceCreated
 * signal with a corresponding MirSurface
 */
TEST_F(SurfaceManagerTests, miralWindowCreationCausesMirSurfaceCreation)
{
    // Setup
    miral::Window window(stubSession, stubSurface);
    ms::SurfaceCreationParameters spec;
    miral::WindowInfo windowInfo(window, spec);

    QSignalSpy newMirSurfaceSpy(surfaceManager.data(), &SurfaceManager::surfaceCreated);

    // Test
    Q_EMIT wmNotifier.windowAdded(windowInfo);
    qtApp->sendPostedEvents();

    // Check result
    auto mirSurface = qvariant_cast<MirSurface*>(newMirSurfaceSpy.takeFirst().at(0));
    ASSERT_TRUE(mirSurface);
    ASSERT_EQ(window, mirSurface->window());
}

/*
 * Test if MirAL notifies that a window was created, SurfaceManager adds the corresponding
 * MirSurface to its internal list
 */
TEST_F(SurfaceManagerTests, miralWindowCreationAddsMirSurfaceToItsInternalList)
{
    // Setup
    miral::Window window(stubSession, stubSurface);
    ms::SurfaceCreationParameters spec;
    miral::WindowInfo windowInfo(window, spec);

    // Test
    Q_EMIT wmNotifier.windowAdded(windowInfo);
    qtApp->sendPostedEvents();

    // Check result
    auto mirSurface = surfaceManager->find(windowInfo);
    ASSERT_TRUE(mirSurface);
    ASSERT_EQ(window, mirSurface->window());
}

/*
 * Test when MirAL creates a surface with a parent, SurfaceManager correctly associates
 * the parent MirSurface to the MirSurface
 */
TEST_F(SurfaceManagerTests, parentedMiralWindowGeneratesMirSurfaceWithCorrectParent)
{
    // Setup
    ms::SurfaceCreationParameters spec;
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

    ASSERT_EQ(parentMirSurface, childMirSurface->parentSurface());
}

/*
 * Test when MirAL creates a surface with a parent, SurfaceManager correctly associates
 * the child MirSurface to the parent MirSurface
 */
TEST_F(SurfaceManagerTests, miralWindowWithChildHasMirSurfaceWithCorrectChild)
{
    // Setup
    ms::SurfaceCreationParameters spec;
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
    ASSERT_EQ(childMirSurface, parentMirSurface->childSurfaceList()->first());
}
