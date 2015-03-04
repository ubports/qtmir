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
 *
 */

#include <Unity/Application/sharedwakelock.h>

#include <libqtdbusmock/DBusMock.h>

#include <QCoreApplication>
#include <QSignalSpy>
#include <gtest/gtest.h>

#include "PowerdInterface.h"

using namespace qtmir;
using namespace testing;
using namespace QtDBusTest;
using namespace QtDBusMock;

const char POWERD_INTERFACE[] = "com.canonical.powerd";
const char POWERD_PATH[] = "/com/canonical/powerd";

class SharedWakelockTest: public Test
{
protected:
    SharedWakelockTest()
        : mock(dbus)
    {
        mock.registerCustomMock(POWERD_INTERFACE,
                                POWERD_PATH,
                                ComCanonicalPowerdInterface::staticInterfaceName(),
                                QDBusConnection::SystemBus);
        
        dbus.startServices();
        
        powerd.reset(
                    new ComCanonicalPowerdInterface(
                        POWERD_INTERFACE,
                        POWERD_PATH,
                        dbus.systemConnection()));
    }
    
    virtual ~SharedWakelockTest()
    {}

    virtual OrgFreedesktopDBusMockInterface& powerdMockInterface()
    {
        return mock.mockInterface(POWERD_INTERFACE,
                                  POWERD_PATH,
                                  ComCanonicalPowerdInterface::staticInterfaceName(),
                                  QDBusConnection::SystemBus);
    }

    void EXPECT_CALL(const QList<QVariantList> &spy, int index,
                     const QString &name, const QVariantList &args)
    {
        QVariant args2(QVariant::fromValue(args));
        ASSERT_LT(index, spy.size());
        const QVariantList &call(spy.at(index));
        EXPECT_EQ(name, call.at(0).toString());
        EXPECT_EQ(args2.toString().toStdString(), call.at(1).toString().toStdString());
    }
    
    DBusTestRunner dbus;
    DBusMock mock;
    QScopedPointer<ComCanonicalPowerdInterface> powerd;
};

TEST_F(SharedWakelockTest, acquireCreatesAWakelock)
{
    // Define mock impementation of the DBus method
    powerdMockInterface().AddMethod("com.canonical.powerd",
            "requestSysState", "si", "s", "ret = 'cookie'").waitForFinished();

    SharedWakelock wakelock(dbus.systemConnection());

    // Verify the DBus method is called & wakelock
    QSignalSpy wakelockDBusMethodSpy(&powerdMockInterface(), SIGNAL(MethodCalled(const QString &, const QVariantList &)));
    QSignalSpy wakelockEnabledSpy(&wakelock, SIGNAL( enabledChanged(bool) ));

    QScopedPointer<QObject> object(new QObject);
    wakelock.acquire(object.data());
    wakelockDBusMethodSpy.wait();

    EXPECT_FALSE(wakelockDBusMethodSpy.empty());
    EXPECT_CALL(wakelockDBusMethodSpy, 0, "requestSysState",
                QVariantList() << QString("active") << 1);

    // Ensure a wakelock created
    wakelockEnabledSpy.wait();
    EXPECT_FALSE(wakelockEnabledSpy.empty());
    EXPECT_TRUE(wakelock.enabled());
}

TEST_F(SharedWakelockTest, acquireThenReleaseDestroysTheWakelock)
{
    // Define mock impementation of the DBus method
    powerdMockInterface().AddMethod("com.canonical.powerd",
            "requestSysState", "si", "s", "ret = 'cookie'").waitForFinished();

    powerdMockInterface().AddMethod("com.canonical.powerd",
            "clearSysState", "s", "", "").waitForFinished();

    SharedWakelock wakelock(dbus.systemConnection());

    QSignalSpy wakelockEnabledSpy(&wakelock, SIGNAL( enabledChanged(bool) ));

    QScopedPointer<QObject> object(new QObject);
    wakelock.acquire(object.data());
    wakelockEnabledSpy.wait();

    // Verify the DBus method is called
    QSignalSpy wakelockDBusMethodSpy(&powerdMockInterface(), SIGNAL(MethodCalled(const QString &, const QVariantList &)));
    wakelock.release(object.data());
    wakelockDBusMethodSpy.wait();

    EXPECT_FALSE(wakelockDBusMethodSpy.empty());
    EXPECT_CALL(wakelockDBusMethodSpy, 0, "clearSysState",
                QVariantList() << QString("cookie"));

    // Ensure a wakelock released
//    wakelockEnabledSpy.wait();
//    EXPECT_FALSE(wakelockEnabledSpy.empty());
    EXPECT_FALSE(wakelock.enabled());
}

TEST_F(SharedWakelockTest, doubleAcquireBySameOwnerOnlyCreatesASingleWakelock)
{
    powerdMockInterface().AddMethod("com.canonical.powerd",
            "requestSysState", "si", "s", "ret = 'cookie'").waitForFinished();

    SharedWakelock wakelock(dbus.systemConnection());

    // Verify the DBus method is called & wakelock
    QSignalSpy wakelockDBusMethodSpy(&powerdMockInterface(), SIGNAL(MethodCalled(const QString &, const QVariantList &)));

    QScopedPointer<QObject> object(new QObject);
    wakelock.acquire(object.data());
    wakelock.acquire(object.data());
    wakelockDBusMethodSpy.wait();

    EXPECT_EQ(wakelockDBusMethodSpy.count(), 1);
}

TEST_F(SharedWakelockTest, doubleAcquireThenReleaseBySameOwnerDestroysWakelock)
{
    // Define mock impementation of the DBus method
    powerdMockInterface().AddMethod("com.canonical.powerd",
            "requestSysState", "si", "s", "ret = 'cookie'").waitForFinished();

    powerdMockInterface().AddMethod("com.canonical.powerd",
            "clearSysState", "s", "", "").waitForFinished();

    SharedWakelock wakelock(dbus.systemConnection());

    QSignalSpy wakelockEnabledSpy(&wakelock, SIGNAL( enabledChanged(bool) ));
    QScopedPointer<QObject> object(new QObject);

    wakelock.acquire(object.data());
    wakelock.acquire(object.data());
//    wakelockEnabledSpy.wait();
    wakelock.release(object.data());
    wakelockEnabledSpy.wait();
    EXPECT_FALSE(wakelock.enabled());
}

/*
 * TEST(SharedWakelock, acquireByDifferentOwnerOnlyCreatesASingleWakelock)
{
    using namespace ::testing;

    testing::NiceMock<MockSharedWakelock> sharedWakelock;
    QScopedPointer<QObject> app1(new QObject);
    QScopedPointer<QObject> app2(new QObject);

    EXPECT_CALL(sharedWakelock, createWakelock()).Times(1).WillOnce(Return(new QObject));
    sharedWakelock.acquire(app1.data());
    sharedWakelock.acquire(app2.data());
}

TEST(SharedWakelock, twoOwnersWhenOneReleasesStillHoldWakelock)
{
    using namespace ::testing;

    testing::NiceMock<MockSharedWakelock> sharedWakelock;
    QScopedPointer<QObject> app1(new QObject);
    QScopedPointer<QObject> app2(new QObject);

    EXPECT_CALL(sharedWakelock, createWakelock()).Times(1).WillOnce(Return(new QObject));
    sharedWakelock.acquire(app1.data());
    sharedWakelock.acquire(app2.data());
    sharedWakelock.release(app1.data());
    EXPECT_TRUE(sharedWakelock.wakelockHeld());
}

TEST(SharedWakelock, twoOwnersWhenBothReleaseWakelockReleased)
{
    using namespace ::testing;

    testing::NiceMock<MockSharedWakelock> sharedWakelock;
    QScopedPointer<QObject> app1(new QObject);
    QScopedPointer<QObject> app2(new QObject);

    EXPECT_CALL(sharedWakelock, createWakelock()).Times(1).WillOnce(Return(new QObject));
    sharedWakelock.acquire(app1.data());
    sharedWakelock.acquire(app2.data());
    sharedWakelock.release(app2.data());
    sharedWakelock.release(app1.data());
    EXPECT_FALSE(sharedWakelock.wakelockHeld());
}

TEST(SharedWakelock, doubleReleaseOfSingleOwnerIgnored)
{
    using namespace ::testing;

    testing::NiceMock<MockSharedWakelock> sharedWakelock;
    QScopedPointer<QObject> app1(new QObject);
    QScopedPointer<QObject> app2(new QObject);

    EXPECT_CALL(sharedWakelock, createWakelock()).Times(1).WillOnce(Return(new QObject));
    sharedWakelock.acquire(app1.data());
    sharedWakelock.acquire(app2.data());
    sharedWakelock.release(app1.data());
    EXPECT_TRUE(sharedWakelock.wakelockHeld());

    sharedWakelock.release(app1.data());
    EXPECT_TRUE(sharedWakelock.wakelockHeld());
}

TEST(SharedWakelock, nullOwnerAcquireIgnored)
{
    using namespace ::testing;

    testing::NiceMock<MockSharedWakelock> sharedWakelock;

    EXPECT_CALL(sharedWakelock, createWakelock()).Times(0);
    sharedWakelock.acquire(nullptr);
}

TEST(SharedWakelock, nullOwnerReleaseIgnored)
{
    using namespace ::testing;

    testing::NiceMock<MockSharedWakelock> sharedWakelock;

    EXPECT_CALL(sharedWakelock, createWakelock()).Times(0);
    sharedWakelock.release(nullptr);
}

TEST(SharedWakelock, ifOwnerDestroyedWakelockReleased)
{
    using namespace ::testing;

    testing::NiceMock<MockSharedWakelock> sharedWakelock;
    QScopedPointer<QObject> app1(new QObject);

    EXPECT_CALL(sharedWakelock, createWakelock()).Times(1).WillOnce(Return(new QObject));
    sharedWakelock.acquire(app1.data());
    app1.reset();
    EXPECT_FALSE(sharedWakelock.wakelockHeld());
}
*/


// Define custom main() as these tests rely on a QEventLoop
int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
