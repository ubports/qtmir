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

#ifndef MOCK_WINDOW_CONTROLLER_H
#define MOCK_WINDOW_CONTROLLER_H

#include "windowcontrollerinterface.h"

#include <gmock/gmock.h>

class MockWindowController : public qtmir::WindowControllerInterface
{
public:
    MOCK_METHOD1(activate, void(const miral::Window &));
    MOCK_METHOD1(raise,    void(const miral::Window &));

    MOCK_METHOD2(resize, void(const miral::Window &, const QSize &));
    MOCK_METHOD2(move,   void(const miral::Window &, const QPoint &));

    MOCK_METHOD1(requestClose, void(const miral::Window &));
    MOCK_METHOD1(forceClose,   void(const miral::Window &));

    MOCK_METHOD2(requestState, void(const miral::Window &, const Mir::State));

    MOCK_METHOD2(deliverKeyboardEvent, void(const miral::Window &, const MirKeyboardEvent *));
    MOCK_METHOD2(deliverTouchEvent,    void(const miral::Window &, const MirTouchEvent *));
    MOCK_METHOD2(deliverPointerEvent,  void(const miral::Window &, const MirPointerEvent *));

    MOCK_METHOD1(setWindowConfinementRegions, void(const QVector<QRect> &regions));
    MOCK_METHOD2(setWindowMargins, void(Mir::Type windowType, const QMargins &margins));
};

#endif // MOCK_WINDOW_CONTROLLER_H
