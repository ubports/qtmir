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

#include "shelluuid.h"

#include <QMutexLocker>

using namespace qtmir;

QUuid ShellUuId::m_uuid;
QMutex ShellUuId::m_mutex;

QString ShellUuId::toString()
{
    QMutexLocker mutexLocker(&m_mutex);

    if (m_uuid.isNull()) {
        m_uuid = QUuid::createUuid();
    }

    return m_uuid.toString();
}
