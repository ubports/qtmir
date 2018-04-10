/*
 * Copyright (C) 2014-2016 Canonical, Ltd.
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

#ifndef SESSIONOBSERVER_H
#define SESSIONOBSERVER_H

#include <QObject>
#include <QRect>
#include <QSize>

#include <mir_toolkit/common.h>
#include <mir/geometry/size.h>

namespace mir {
    namespace scene {
        class Surface;
    }
}

namespace miral { class WindowSpecification; }

class SurfaceObserver : public QObject
{
    Q_OBJECT

public:
    virtual ~SurfaceObserver();

    virtual void frame_posted(mir::scene::Surface const*, int frames_available, mir::geometry::Size const& size ) = 0;

    void notifySurfaceModifications(const miral::WindowSpecification&);

    static SurfaceObserver *observerForSurface(const mir::scene::Surface *surface);
    static void registerObserverForSurface(SurfaceObserver *observer, const mir::scene::Surface *surface);

Q_SIGNALS:
    void attributeChanged(const MirWindowAttrib attribute, const int value);
    void framesPosted();
    void resized(const QSize &size);
    void nameChanged(const QString &name);
    void cursorChanged(const QCursor &cursor);

    void minimumWidthChanged(int);
    void minimumHeightChanged(int);
    void maximumWidthChanged(int);
    void maximumHeightChanged(int);
    void widthIncrementChanged(int);
    void heightIncrementChanged(int);
    void shellChromeChanged(MirShellChrome);
    void inputBoundsChanged(const QRect &rect);
    void confinesMousePointerChanged(bool);
};

#endif
