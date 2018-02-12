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

// local
#include "mirsurfacelistmodel.h"
#include "mirsurfaceinterface.h"
#include "session_interface.h"

// QPA mirserver
#include <logging.h>
#include <shelluuid.h>

#include <QDBusConnection>

using namespace qtmir;

DBusFocusInfo::DBusFocusInfo(const QList<Application*> &applications)
    : m_applications(applications)
{
    QDBusConnection::sessionBus().registerService("com.canonical.Unity.FocusInfo");
    QDBusConnection::sessionBus().registerObject("/com/canonical/Unity/FocusInfo", this, QDBusConnection::ExportScriptableSlots);
}

bool DBusFocusInfo::isPidFocused(unsigned int pid)
{
    if (QCoreApplication::applicationPid() == (qint64)pid) {
        // Shell itself.
        // Don't bother checking if it has a QML with activeFocus() which is not a MirSurfaceItem.
        return true;
    } else {
        auto pidSet = fetchAssociatedPids((pid_t)pid);
        SessionInterface *session = findSessionWithPid(pidSet);
        return session ? session->activeFocus() : false;
    }
}

QSet<pid_t> DBusFocusInfo::fetchAssociatedPids(pid_t pid)
{
    // TODO port me to systemd ubuntu-app-launch
    qCDebug(QTMIR_DBUS) << "DBusFocusInfo: pid" << pid << "unable to determine cgroup, assuming is not app-specific.";
    return QSet<pid_t>({pid});
}

SessionInterface* DBusFocusInfo::findSessionWithPid(const QSet<pid_t> &pidSet)
{
    Q_FOREACH (Application* application, m_applications) {
        QVector<SessionInterface*> sessions = application->sessions();
        for (auto session : sessions) {
            if (pidSet.contains(session->pid())) {
                return session;
            }
            SessionInterface *chosenChildSession = nullptr;
            session->foreachChildSession([&](SessionInterface* childSession) {
                if (pidSet.contains(childSession->pid())) {
                    chosenChildSession = childSession;
                }
            });
            if (chosenChildSession) {
                return chosenChildSession;
            }
        }
    }
    return nullptr;
}

bool DBusFocusInfo::isSurfaceFocused(const QString &serializedId)
{
    // TODO: Implement a penalty for negative queries, such as stalling for some time before answering
    //       further queries. That's in order to avoid brute-force approaches to find a valid surface id.
    //       That's particularly important for shell's own surface id as it's always valid.
    bool result = false;
    if (serializedId == ShellUuId::toString()) {
        result = true;
    } else {
        MirSurfaceInterface *qmlSurface = findQmlSurface(serializedId);
        result = qmlSurface ? qmlSurface->activeFocus() : false;
    }
    qCDebug(QTMIR_DBUS).nospace() << "DBusFocusInfo: isSurfaceFocused("<<serializedId<<") -> " << result;
    return result;
}

MirSurfaceInterface *DBusFocusInfo::findQmlSurface(const QString &serializedId)
{
    for (Application* application : m_applications) {
        for (SessionInterface *session : application->sessions()) {
            if (session) {
                auto surfaceList = static_cast<MirSurfaceListModel*>(session->surfaceList());
                for (int i = 0; i < surfaceList->count(); ++i) {
                    auto qmlSurface = static_cast<MirSurfaceInterface*>(surfaceList->get(i));
                    if (qmlSurface->persistentId() == serializedId) {
                        return qmlSurface;
                    }
                }

                surfaceList = static_cast<MirSurfaceListModel*>(session->promptSurfaceList());
                for (int i = 0; i < surfaceList->count(); ++i) {
                    auto qmlSurface = static_cast<MirSurfaceInterface*>(surfaceList->get(i));
                    if (qmlSurface->persistentId() == serializedId) {
                        return qmlSurface;
                    }
                }
            }
        }
    }
    return nullptr;
}
