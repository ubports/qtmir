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

#include <mir/compositor/compositor.h>

// Qt
#include <QObject>

class QtCompositor : public QObject, public mir::compositor::Compositor
{
    Q_OBJECT
public:
    QtCompositor() = default;
    virtual ~QtCompositor() noexcept = default;

    void start();
    void stop();

Q_SIGNALS:
    void starting();
    void stopping();

private:
};

#endif // QTCOMPOSITOR_H
