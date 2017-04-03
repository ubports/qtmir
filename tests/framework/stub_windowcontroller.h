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

#ifndef STUB_WINDOWCONTROLLER_H
#define STUB_WINDOWCONTROLLER_H

#include "windowcontrollerinterface.h"

namespace qtmir {

struct StubWindowModelController : public WindowControllerInterface
{
    void activate (const miral::Window &/*window*/) override { return; }
    void raise(const miral::Window &/*window*/) override {};

    void resize(const miral::Window &/*window*/, const QSize &/*size*/) override { return; }
    void move  (const miral::Window &/*window*/, const QPoint &/*topLeft*/) override { return; }

    void requestClose(const miral::Window &/*window*/) override { return; }
    void forceClose(const miral::Window &/*window*/) override { return; }

    void requestState(const miral::Window &/*window*/, const Mir::State /*state*/) override { return; }

    void deliverKeyboardEvent(const miral::Window &/*window*/, const MirKeyboardEvent */*event*/) override { return; }
    void deliverTouchEvent   (const miral::Window &/*window*/, const MirTouchEvent */*event*/)    override { return; }
    void deliverPointerEvent (const miral::Window &/*window*/, const MirPointerEvent */*event*/)  override { return; }

    void setWindowConfinementRegions(const QVector<QRect> &/*regions*/) override { return; }
    void setWindowMargins(Mir::Type /*windowType*/, const QMargins &/*margins*/) override { return; }
};

} //namespace qtmir

#endif // STUB_WINDOWCONTROLLER_H
