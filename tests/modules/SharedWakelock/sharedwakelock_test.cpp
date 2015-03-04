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

using namespace qtmir;
using namespace testing;
using namespace QtDBusTest;
using namespace QtDBusMock;

const char POWERD_NAME[] = "com.canonical.powerd";
const char POWERD_PATH[] = "/com/canonical/powerd";
const char POWERD_INTERFACE[] = "com.canonical.powerd";

class SharedWakelockTest: public Test
{
protected:
    SharedWakelockTest()
        : mock(dbus)
    {
        mock.registerCustomMock(POWERD_NAME,
                                POWERD_PATH,
                                POWERD_INTERFACE,
                                QDBusConnection::SystemBus);
        
        dbus.startServices();
    }
    
    virtual ~SharedWakelockTest()
    {}

    virtual OrgFreedesktopDBusMockInterface& powerdMockInterface()
    {
        return mock.mockInterface(POWERD_NAME,
                                  POWERD_PATH,
                                  POWERD_INTERFACE,
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
    wakelock.release(object.data());
    wakelockEnabledSpy.wait();
    EXPECT_FALSE(wakelock.enabled());
}

TEST_F(SharedWakelockTest, acquireByDifferentOwnerOnlyCreatesASingleWakelock)
{
    powerdMockInterface().AddMethod("com.canonical.powerd",
            "requestSysState", "si", "s", "ret = 'cookie'").waitForFinished();

    SharedWakelock wakelock(dbus.systemConnection());

    // Verify the DBus method is called & wakelock
    QSignalSpy wakelockDBusMethodSpy(&powerdMockInterface(), SIGNAL(MethodCalled(const QString &, const QVariantList &)));

    QScopedPointer<QObject> object1(new QObject);
    QScopedPointer<QObject> object2(new QObject);
    wakelock.acquire(object1.data());
    wakelock.acquire(object2.data());

    wakelockDBusMethodSpy.wait();
    EXPECT_EQ(wakelockDBusMethodSpy.count(), 1);
}

TEST_F(SharedWakelockTest, twoOwnersWhenBothReleaseWakelockReleased)
{
    // Define mock impementation of the DBus method
    powerdMockInterface().AddMethod("com.canonical.powerd",
            "requestSysState", "si", "s", "ret = 'cookie'").waitForFinished();

    SharedWakelock wakelock(dbus.systemConnection());

    QScopedPointer<QObject> object1(new QObject);
    QScopedPointer<QObject> object2(new QObject);
    wakelock.acquire(object1.data());
    wakelock.acquire(object2.data());

    QSignalSpy wakelockEnabledSpy(&wakelock, SIGNAL( enabledChanged(bool) ));

    wakelock.release(object1.data());
    wakelock.release(object2.data());

    wakelockEnabledSpy.wait();
    EXPECT_FALSE(wakelock.enabled());
}

TEST_F(SharedWakelockTest, doubleReleaseOfSingleOwnerIgnored)
{
    // Define mock impementation of the DBus method
    powerdMockInterface().AddMethod("com.canonical.powerd",
            "requestSysState", "si", "s", "ret = 'cookie'").waitForFinished();

    SharedWakelock wakelock(dbus.systemConnection());

    QSignalSpy wakelockEnabledSpy(&wakelock, SIGNAL( enabledChanged(bool) ));

    QScopedPointer<QObject> object1(new QObject);
    QScopedPointer<QObject> object2(new QObject);
    wakelock.acquire(object1.data());
    wakelock.acquire(object2.data());
    wakelock.release(object1.data());

    wakelockEnabledSpy.wait();

    wakelock.release(object1.data());

    EXPECT_TRUE(wakelock.enabled());
}

TEST_F(SharedWakelockTest, wakelockAcquireReleaseFlood)
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
    wakelock.release(object.data());
    wakelock.acquire(object.data());
    wakelock.release(object.data());
    wakelock.acquire(object.data());
    wakelock.release(object.data());
    while (wakelockEnabledSpy.wait(100)) {
        if (wakelockEnabledSpy.count() == 6)
            break;
    }
    EXPECT_FALSE(wakelock.enabled());
}

TEST_F(SharedWakelockTest, nullOwnerAcquireIgnored)
{
    // Define mock impementation of the DBus method
    powerdMockInterface().AddMethod("com.canonical.powerd",
            "requestSysState", "si", "s", "ret = 'cookie'").waitForFinished();

    SharedWakelock wakelock(dbus.systemConnection());

    QSignalSpy wakelockEnabledSpy(&wakelock, SIGNAL( enabledChanged(bool) ));

    wakelock.acquire(nullptr);

    wakelockEnabledSpy.wait(200); // have to wait to see if anything happens

    EXPECT_FALSE(wakelock.enabled());
}

TEST_F(SharedWakelockTest, nullReleaseAcquireIgnored)
{
    // Define mock impementation of the DBus method
    powerdMockInterface().AddMethod("com.canonical.powerd",
            "requestSysState", "si", "s", "ret = 'cookie'").waitForFinished();

    SharedWakelock wakelock(dbus.systemConnection());

    wakelock.release(nullptr);

    EXPECT_FALSE(wakelock.enabled());
}

TEST_F(SharedWakelockTest, ifOwnerDestroyedWakelockReleased)
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

    object.reset();
    wakelockDBusMethodSpy.wait();

    EXPECT_FALSE(wakelockDBusMethodSpy.empty());
    EXPECT_CALL(wakelockDBusMethodSpy, 0, "clearSysState",
                QVariantList() << QString("cookie"));

    EXPECT_FALSE(wakelock.enabled());
}

TEST_F(SharedWakelockTest, reloadCachedWakelockCookie)
{
    const char cookieFile[] = "/tmp/qtmir_powerd_cookie";

    // Custom Deleter for QScopedPointer to delete the qtmir cookie file no matter what
    struct ScopedQFileCustomDeleter
    {
        static inline void cleanup(QFile *file)
        {
            file->remove();
            delete file;
        }
    };
    QScopedPointer<QFile, ScopedQFileCustomDeleter> cookieCache(new QFile(cookieFile));
    cookieCache->open(QFile::WriteOnly | QFile::Text);
    cookieCache->write("myCookie");

    SharedWakelock wakelock(dbus.systemConnection());
    EXPECT_TRUE(wakelock.enabled());
}

TEST_F(SharedWakelockTest, wakelockReleasedOnSharedWakelockDestroyed)
{
    // Define mock impementation of the DBus method
    powerdMockInterface().AddMethod("com.canonical.powerd",
            "requestSysState", "si", "s", "ret = 'cookie'").waitForFinished();

    powerdMockInterface().AddMethod("com.canonical.powerd",
            "clearSysState", "s", "", "").waitForFinished();

    auto wakelock = new SharedWakelock(dbus.systemConnection());

    QSignalSpy wakelockEnabledSpy(wakelock, SIGNAL( enabledChanged(bool) ));

    QScopedPointer<QObject> object(new QObject);
    wakelock->acquire(object.data());
    wakelockEnabledSpy.wait(); // wait for wakelock to be enabled

    QSignalSpy wakelockDBusMethodSpy(&powerdMockInterface(), SIGNAL(MethodCalled(const QString &, const QVariantList &)));
    delete wakelock;
    wakelockDBusMethodSpy.wait();

    EXPECT_FALSE(wakelockDBusMethodSpy.empty());
    EXPECT_CALL(wakelockDBusMethodSpy, 0, "clearSysState",
                QVariantList() << QString("cookie"));
}


// Define custom main() as these tests rely on a QEventLoop
int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
