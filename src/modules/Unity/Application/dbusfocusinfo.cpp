/*
 * Copyright (C) 2016 Canonical, Ltd.
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

#include "dbusfocusinfo.h"

#include <QDBusConnection>

using namespace qtmir;

DBusFocusInfo::DBusFocusInfo(QList<Application*> *applications)
    : m_applications(applications)
{
    QDBusConnection::sessionBus().registerService("com.canonical.Unity.FocusInfo");
    QDBusConnection::sessionBus().registerObject("/", this, QDBusConnection::ExportScriptableSlots);
}

bool DBusFocusInfo::isPidFocused(unsigned int pid)
{
    if (QCoreApplication::applicationPid() == (qint64)pid) {
        // Shell itself.
        // Don't bother checking if it has a QML with activeFocus() which is not a MirSurfaceItem.
        return true;
    } else {
        Q_FOREACH (Application* application, *m_applications) {
            if ((pid_t)pid == application->pid()) {
                return application->activeFocus();
            }
        }
    }

    return false;
}
