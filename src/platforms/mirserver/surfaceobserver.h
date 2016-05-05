/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
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

#include <QByteArray>
#include <QCursor>
#include <QObject>
#include <QMap>
#include <QMutex>
#include <QSize>
#include <mir/scene/surface_observer.h>

namespace mir {
    namespace scene {
        class Surface;
    }
    namespace shell {
        class SurfaceSpecification;
    }
}

class SurfaceObserver : public QObject, public mir::scene::SurfaceObserver
{
    Q_OBJECT

public:
    SurfaceObserver();
    virtual ~SurfaceObserver();

    void setListener(QObject *listener);

    void attrib_changed(MirSurfaceAttrib, int) override;
    void resized_to(mir::geometry::Size const&) override;
    void moved_to(mir::geometry::Point const&) override {}
    void hidden_set_to(bool) override {}

    // Get new frame notifications from Mir, called from a Mir thread.
    void frame_posted(int frames_available, mir::geometry::Size const& size ) override;

    void alpha_set_to(float) override {}
    void transformation_set_to(glm::mat4 const&) override {}
    void reception_mode_set_to(mir::input::InputReceptionMode) override {}
    void cursor_image_set_to(mir::graphics::CursorImage const&) override;
    void orientation_set_to(MirOrientation) override {}
    void client_surface_close_requested() override {}
    void keymap_changed(MirInputDeviceId, std::string const& model, std::string const& layout,
                        std::string const& variant, std::string const& options) override;
    void renamed(char const * name) override;
    void cursor_image_removed() override;

    void notifySurfaceModifications(const mir::shell::SurfaceSpecification&);

    static SurfaceObserver *observerForSurface(const mir::scene::Surface *surface);
    static void registerObserverForSurface(SurfaceObserver *observer, const mir::scene::Surface *surface);
    static QMutex mutex;

Q_SIGNALS:
    void attributeChanged(const MirSurfaceAttrib attribute, const int value);
    void framesPosted();
    void resized(const QSize &size);
    void keymapChanged(const QString &rules, const QString &variant);
    void nameChanged(const QString &name);
    void cursorChanged(const QCursor &cursor);

    void minimumWidthChanged(int);
    void minimumHeightChanged(int);
    void maximumWidthChanged(int);
    void maximumHeightChanged(int);
    void widthIncrementChanged(int);
    void heightIncrementChanged(int);
    void shellChromeChanged(MirShellChrome);

private:
    QCursor createQCursorFromMirCursorImage(const mir::graphics::CursorImage &cursorImage);
    QObject *m_listener;
    bool m_framesPosted;
    QMap<QByteArray, Qt::CursorShape> m_cursorNameToShape;
    static QMap<const mir::scene::Surface*, SurfaceObserver*> m_surfaceToObserverMap;
};

#endif
