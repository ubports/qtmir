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

#ifndef WINDOWCONTROLLERINTERFACE_H
#define WINDOWCONTROLLERINTERFACE_H

#include <miral/window.h>

#include <mir_toolkit/event.h>

#include <QPoint>
#include <QSize>
#include <QMargins>

// Unity API
#include <unity/shell/application/Mir.h>

namespace qtmir {

class MirSurface;

class WindowControllerInterface {
public:
    WindowControllerInterface() = default;
    virtual ~WindowControllerInterface() = default;

    // activate() asks Mir to bring particular window to the front and recommend to shell that it be focused
    virtual void activate (const miral::Window &window) = 0;
    virtual void raise(const miral::Window &window) = 0;

    virtual void resize(const miral::Window &window, const QSize &size) = 0;
    virtual void move  (const miral::Window &window, const QPoint &topLeft) = 0;

    virtual void requestClose(const miral::Window &window) = 0;
    virtual void forceClose(const miral::Window &window) = 0;

    virtual void requestState(const miral::Window &window, const Mir::State state) = 0;

    virtual void deliverKeyboardEvent(const miral::Window &window, const MirKeyboardEvent *event) = 0;
    virtual void deliverTouchEvent   (const miral::Window &window, const MirTouchEvent *event) = 0;
    virtual void deliverPointerEvent (const miral::Window &window, const MirPointerEvent *event) = 0;

    virtual void setWindowPositionBoundaries(const QRegion &region) = 0;
    virtual void setWindowMargins(MirWindowType windowType, const QMargins &margins) = 0;
};

} // namespace qtmir

#endif // WINDOWCONTROLLERINTERFACE_H
