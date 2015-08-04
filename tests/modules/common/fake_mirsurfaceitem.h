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
 */

#ifndef FAKE_MIRSURFACEITEMINTERFACE_H
#define FAKE_MIRSURFACEITEMINTERFACE_H

#include <Unity/Application/mirsurfaceiteminterface.h>

namespace qtmir {

class FakeMirSurfaceItem : public MirSurfaceItemInterface
{
    Q_OBJECT

public:
    FakeMirSurfaceItem(QQuickItem *parent = nullptr)
        : MirSurfaceItemInterface(parent)
        , m_state(Restored)
        , m_live(true)
        , m_session(nullptr)
        , m_isFirstFrameDrawn(false)
        , m_isFrameDropperRunning(true)
    {}

    Type type() const override { return Normal; }
    State state() const override { return m_state; }
    QString name() const override { return QString("fake app surface"); }
    bool live() const override { return m_live; }
    SessionInterface *session() const override { return m_session; }
    OrientationAngle orientationAngle() const override { return Angle0; }

    Q_INVOKABLE void release() override {}

    void stopFrameDropper() override {
        m_isFrameDropperRunning = false;
    }
    void startFrameDropper() override {
        m_isFrameDropperRunning = true;
    }

    bool isFirstFrameDrawn() const override {
        return m_isFirstFrameDrawn;
    }

    void setOrientationAngle(OrientationAngle) override {
    }

    void setSession(SessionInterface *session) override {
        m_session = session;
    }

    void setState(State state) {
        if (m_state != state) {
            m_state = state;
            Q_EMIT stateChanged();
        }
    }

    void drawFirstFrame() {
        if (!m_isFirstFrameDrawn) {
            m_isFirstFrameDrawn = true;
            Q_EMIT firstFrameDrawn();
        }
    }

    bool isFrameDropperRunning() const {
        return m_isFrameDropperRunning;
    }

private:
    void setLive(bool value) override { m_live = value; }

    State m_state;
    bool m_live;
    SessionInterface *m_session;
    bool m_isFirstFrameDrawn;
    bool m_isFrameDropperRunning;
};

} // namespace qtmir

#endif // FAKE_MIRSURFACEITEMINTERFACE_H
