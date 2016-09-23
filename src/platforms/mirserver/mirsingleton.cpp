/*
 * Copyright (C) 2015-2016 Canonical, Ltd.
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

#include "mirsingleton.h"

#include <QDebug>

qtmir::Mir *qtmir::Mir::m_instance = nullptr;

qtmir::Mir::Mir()
{
}

qtmir::Mir::~Mir()
{
    m_instance = nullptr;
}

qtmir::Mir *qtmir::Mir::instance()
{
    if (!m_instance) {
        m_instance = new qtmir::Mir;
    }
    return m_instance;
}

void qtmir::Mir::setCursorName(const QString &cursorName)
{
    if (m_cursorName != cursorName) {
        m_cursorName = cursorName;
        Q_EMIT cursorNameChanged(m_cursorName);
    }
}

QString qtmir::Mir::cursorName() const
{
    return m_cursorName;
}

QString qtmir::Mir::currentKeymap() const
{
    return m_currentKeymap;
}

void qtmir::Mir::setCurrentKeymap(const QString &currentKeymap)
{
    if (m_currentKeymap == currentKeymap)
        return;

    m_currentKeymap = currentKeymap;
    Q_EMIT currentKeymapChanged(m_currentKeymap);

    qDebug() << "!!! Current keymap changed to:" << m_currentKeymap;
}
