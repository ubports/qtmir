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

#ifndef QTCOMPOSITOR_H
#define QTCOMPOSITOR_H

// std lib
#include <memory>

// Qt
#include <QObject>

namespace mir {
    namespace graphics {
        class Cursor;
    }
}

class QtCompositor : public QObject
{
    Q_OBJECT
public:
    QtCompositor() = default;
    virtual ~QtCompositor() noexcept = default;

Q_SIGNALS:
    void starting();
    void stopping();
};

#endif // QTCOMPOSITOR_H
