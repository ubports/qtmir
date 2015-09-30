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

#ifndef MOCK_SHARED_WAKELOCK_H
#define MOCK_SHARED_WAKELOCK_H

#include <Unity/Application/sharedwakelock.h>

#include <gmock/gmock.h>

namespace testing
{
class MockSharedWakelock : public qtmir::SharedWakelock
{
public:
    MockSharedWakelock(const QDBusConnection& /*connection*/= QDBusConnection::systemBus());
    virtual ~MockSharedWakelock();

    MOCK_CONST_METHOD0(enabled, bool());
    MOCK_METHOD1(acquire, void(const QObject *));
    MOCK_METHOD1(release, void(const QObject *));

    bool doEnabled();
    void doAcquire(const QObject *object);
    void doRelease(const QObject *object);

private:
    QSet<const QObject *> m_owners;
};

} // namespace testing
#endif // MOCK_SHARED_WAKELOCK_H
