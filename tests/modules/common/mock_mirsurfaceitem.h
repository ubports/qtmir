/*
 * Copyright (C) 2015 Canonical, Ltd.
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
 *
 */

#ifndef MOCK_QTMIR_MIRSURFACEITEM_H
#define MOCK_QTMIR_MIRSURFACEITEM_H

#include <Unity/Application/mirsurfaceiteminterface.h>
#include <gmock/gmock.h>

namespace qtmir {

class MockMirSurfaceItem : public MirSurfaceItemInterface {
public:
    MockMirSurfaceItem(QQuickItem *parent = nullptr) : MirSurfaceItemInterface(parent) {}

    MOCK_CONST_METHOD0(type, Type());
    MOCK_CONST_METHOD0(state, State());
    MOCK_CONST_METHOD0(name, QString());
    MOCK_CONST_METHOD0(live, bool());
    MOCK_CONST_METHOD0(orientationAngle, OrientationAngle());
    MOCK_CONST_METHOD0(session, SessionInterface*());

    MOCK_METHOD0(release, void());
    MOCK_METHOD0(close, bool());

    MOCK_METHOD0(stopFrameDropper, void());
    MOCK_METHOD0(startFrameDropper, void());

    MOCK_CONST_METHOD0(isFirstFrameDrawn, bool());

    MOCK_METHOD1(setOrientationAngle, void(OrientationAngle angle));
    MOCK_METHOD1(setSession, void(SessionInterface *app));
    MOCK_METHOD1(setLive, void(bool live));
};

} // namespace qtmir

#endif // MOCK_QTMIR_MIRSURFACEITEM_H
