/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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

#include "qtcompositor.h"
#include "logging.h"

#include <mir/graphics/cursor.h>

// Lives in a Mir thread
void QtCompositor::start()
{
    qCDebug(QTMIR_SCREENS) << "QtCompositor::start";

    // FIXME: Hack to work around https://bugs.launchpad.net/mir/+bug/1502200
    //        See the FIXME in mirserver.cpp
    if (m_cursor) {
        m_cursor->hide();
    }

    Q_EMIT starting(); // blocks
}

void QtCompositor::stop()
{
    qCDebug(QTMIR_SCREENS) << "QtCompositor::stop";

    Q_EMIT stopping(); // blocks
}

void QtCompositor::setCursor(std::shared_ptr<mir::graphics::Cursor> cursor)
{
    m_cursor = cursor;
}
