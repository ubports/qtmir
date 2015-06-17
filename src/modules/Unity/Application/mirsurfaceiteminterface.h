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

#ifndef MIRSURFACEITEMINTERFACE_H
#define MIRSURFACEITEMINTERFACE_H

// Qt
#include <QQuickItem>

// mir
#include <mir_toolkit/common.h>

#include "session_interface.h"

namespace qtmir {

class MirSurfaceItemInterface : public QQuickItem
{
    Q_OBJECT
    Q_ENUMS(Type)
    Q_ENUMS(State)
    Q_ENUMS(OrientationAngle)

    Q_PROPERTY(Type type READ type NOTIFY typeChanged)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(bool live READ live NOTIFY liveChanged)

    // How many degrees, clockwise, the UI in the surface has to rotate to match with the
    // shell UI orientation
    Q_PROPERTY(OrientationAngle orientationAngle READ orientationAngle WRITE setOrientationAngle
               NOTIFY orientationAngleChanged DESIGNABLE false)

public:
    MirSurfaceItemInterface(QQuickItem *parent) : QQuickItem(parent) {}
    virtual ~MirSurfaceItemInterface() {}

    enum Type {
        Normal = mir_surface_type_normal,
        Utility = mir_surface_type_utility,
        Dialog = mir_surface_type_dialog,
        Overlay = mir_surface_type_overlay,
        Freestyle = mir_surface_type_freestyle,
        Popover = mir_surface_type_popover,
        InputMethod = mir_surface_type_inputmethod,
        };

    enum State {
        Unknown = mir_surface_state_unknown,
        Restored = mir_surface_state_restored,
        Minimized = mir_surface_state_minimized,
        Maximized = mir_surface_state_maximized,
        VertMaximized = mir_surface_state_vertmaximized,
        /* SemiMaximized = mir_surface_state_semimaximized, // see mircommon/mir_toolbox/common.h*/
        Fullscreen = mir_surface_state_fullscreen,
    };

    enum OrientationAngle {
        Angle0 = 0,
        Angle90 = 90,
        Angle180 = 180,
        Angle270 = 270
    };

    //getters
    virtual Type type() const = 0;
    virtual State state() const = 0;
    virtual QString name() const = 0;
    virtual bool live() const = 0;
    virtual SessionInterface *session() const = 0;
    virtual OrientationAngle orientationAngle() const = 0;

    virtual Q_INVOKABLE void release() = 0;
    virtual bool close() = 0;

    virtual void stopFrameDropper() = 0;
    virtual void startFrameDropper() = 0;

    virtual bool isFirstFrameDrawn() const = 0;

    virtual void setOrientationAngle(OrientationAngle angle) = 0;
    virtual void setSession(SessionInterface *app) = 0;

Q_SIGNALS:
    void typeChanged();
    void stateChanged();
    void nameChanged();
    void orientationAngleChanged(OrientationAngle angle);
    void liveChanged(bool live);
    void firstFrameDrawn();

private:
    virtual void setLive(bool) = 0;

    friend class MirSurfaceManager;
};

} // namespace qtmir

Q_DECLARE_METATYPE(qtmir::MirSurfaceItemInterface*)
Q_DECLARE_METATYPE(qtmir::MirSurfaceItemInterface::OrientationAngle)

#endif // MIRSURFACEITEMINTERFACE_H

