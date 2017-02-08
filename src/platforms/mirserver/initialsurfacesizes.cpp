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

#include "initialsurfacesizes.h"

#include <QMutexLocker>

QMap<pid_t, QSize> InitialSurfaceSizes::sizeForSession;
QMutex InitialSurfaceSizes::mutex;

void InitialSurfaceSizes::set(pid_t pid, const QSize &size)
{
    QMutexLocker locker(&mutex);

    sizeForSession[pid] = size;
}

void InitialSurfaceSizes::remove(pid_t pid)
{
    QMutexLocker locker(&mutex);

    sizeForSession.remove(pid);
}

QSize InitialSurfaceSizes::get(pid_t pid)
{
    QMutexLocker locker(&mutex);

    if (sizeForSession.contains(pid)) {
        return sizeForSession[pid];
    } else {
        return QSize();
    }
}
