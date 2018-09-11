/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
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
    qreal devicePixelRatio() const override { return m_devicePixelRatio; }
    qreal pixelDensity() const override;
    QSizeF physicalSize() const override { return m_physicalSize; }
    qreal refreshRate() const override { return m_refreshRate; }
    Qt::ScreenOrientation nativeOrientation() const override { return m_nativeOrientation; }
    Qt::ScreenOrientation orientation() const override { return m_currentOrientation; }
    QPlatformCursor *cursor() const override;
    QString name() const override;

    float scale() const { return m_scale; }
    MirFormFactor formFactor() const { return m_formFactor; }
    MirPowerMode powerMode() const { return m_powerMode; }
    mir::graphics::DisplayConfigurationOutputId outputId() const { return m_outputId; }
    mir::graphics::DisplayConfigurationOutputType outputType() const { return m_type; }
    std::vector<MirPixelFormat> pixelFormats() const { return m_pixelFormats; }
    uint32_t currentModeIndex() const { return m_currentModeIndex; }
    uint32_t preferredModeIndex() const { return m_preferredModeIndex; }

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

    void setMirDisplayConfiguration(const mir::graphics::DisplayConfigurationOutput &, bool notify = true);
    void setMirDisplayBuffer(mir::graphics::DisplayBuffer *, mir::graphics::DisplaySyncGroup *);
    void swapBuffers();
    void makeCurrent();
    void doneCurrent();

private:
    void toggleSensors(const bool enable) const;
    bool internalDisplay() const;

    QRect m_geometry;
    int m_depth;
    QImage::Format m_format;
    qreal m_devicePixelRatio;
    QSizeF m_physicalSize;
    qreal m_refreshRate;
    float m_scale;
    MirFormFactor m_formFactor;
    std::vector<MirPixelFormat> m_pixelFormats;
    uint32_t m_currentModeIndex;
    uint32_t m_preferredModeIndex;

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
