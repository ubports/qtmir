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

#ifndef SCREEN_H
#define SCREEN_H

// Qt
#include <QObject>
#include <QTimer>
#include <QtDBus/QDBusInterface>
#include <qpa/qplatformscreen.h>

// Mir
#include <mir/graphics/display_configuration.h>

// local
#include "cursor.h"
#include "screenwindow.h"

class QOrientationSensor;
namespace mir {
    namespace graphics { class DisplayBuffer; class DisplaySyncGroup; }
    namespace renderer { namespace gl { class RenderTarget; }}
}

class Screen : public QObject, public QPlatformScreen
{
    Q_OBJECT
public:
    Screen(const mir::graphics::DisplayConfigurationOutput &);
    ~Screen();

    // QPlatformScreen methods.
    QRect geometry() const override { return m_geometry; }
    int depth() const override { return m_depth; }
    QImage::Format format() const override { return m_format; }
    QSizeF physicalSize() const override { return m_physicalSize; }
    qreal refreshRate() const override { return m_refreshRate; }
    Qt::ScreenOrientation nativeOrientation() const override { return m_nativeOrientation; }
    Qt::ScreenOrientation orientation() const override { return m_currentOrientation; }
    QPlatformCursor *cursor() const override;

    void toggleSensors(const bool enable) const;
    mir::graphics::DisplayConfigurationOutputType outputType() const { return m_type; }

    ScreenWindow* window() const;

    // QObject methods.
    void customEvent(QEvent* event) override;

    // To make it testable
    static bool skipDBusRegistration;
    bool orientationSensorEnabled();

public Q_SLOTS:
   void onDisplayPowerStateChanged(int, int);
   void onOrientationReadingChanged();

protected:
    void setWindow(ScreenWindow *window);

    void setMirDisplayConfiguration(const mir::graphics::DisplayConfigurationOutput &);
    void setMirDisplayBuffer(mir::graphics::DisplayBuffer *, mir::graphics::DisplaySyncGroup *);
    void swapBuffers();
    void makeCurrent();
    void doneCurrent();

private:
    QRect m_geometry;
    int m_depth;
    QImage::Format m_format;
    QSizeF m_physicalSize;
    qreal m_refreshRate;

    mir::renderer::gl::RenderTarget *m_renderTarget;
    mir::graphics::DisplaySyncGroup *m_displayGroup;
    mir::graphics::DisplayConfigurationOutputId m_outputId;
    mir::graphics::DisplayConfigurationCardId m_cardId;
    mir::graphics::DisplayConfigurationOutputType m_type;
    MirPowerMode m_powerMode;

    Qt::ScreenOrientation m_nativeOrientation;
    Qt::ScreenOrientation m_currentOrientation;
    QOrientationSensor *m_orientationSensor;

    ScreenWindow *m_screenWindow;
    QDBusInterface *m_unityScreen;

    qtmir::Cursor m_cursor;

    friend class ScreensModel;
    friend class ScreenWindow;
};

#endif // SCREEN_H
