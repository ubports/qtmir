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
#include <QScopedPointer>
#include <QTimer>
#include <qpa/qplatformscreen.h>
#include <QtSensors/QOrientationReading>

// Mir
#include <mir_toolkit/common.h>

// Miral
#include <miral/output.h>

// local
#include "cursor.h"
#include "screenwindow.h"
#include "screentypes.h"

// std
#include <memory>

class OrientationSensor;
namespace mir {
    namespace graphics { class DisplayBuffer; class DisplaySyncGroup; class DisplayConfigurationOutput; }
    namespace renderer { namespace gl { class RenderTarget; }}
}

class Screen : public QObject, public QPlatformScreen
{
    Q_OBJECT
public:
    Screen(const miral::Output &, const std::shared_ptr<OrientationSensor>);
    ~Screen();

    // QPlatformScreen methods.
    QRect geometry() const override { return m_geometry; }
    int depth() const override { return m_depth; }
    QImage::Format format() const override { return m_format; }
    qreal devicePixelRatio() const override { return m_devicePixelRatio; }
    QSizeF physicalSize() const override { return m_physicalSize; }
    qreal refreshRate() const override { return m_refreshRate; }
    Qt::ScreenOrientation nativeOrientation() const override { return m_nativeOrientation; }
    Qt::ScreenOrientation orientation() const override { return m_currentOrientation; }
    QPlatformCursor *cursor() const override;
    QString name() const override;

    float scale() const { return m_scale; }
    MirFormFactor formFactor() const { return m_formFactor; }
    MirPowerMode powerMode() const { return m_powerMode; }
    qtmir::OutputId outputId() const { return m_outputId; }
    qtmir::OutputTypes outputType() const { return m_type; }
    uint32_t currentModeIndex() const { return m_currentModeIndex; }

    ScreenWindow* window() const;

    // QObject methods.
    void customEvent(QEvent* event) override;

    // To make it testable
    bool orientationSensorEnabled();

    bool isSameOutput(const miral::Output &output);
public Q_SLOTS:
   void onOrientationReadingChanged(QOrientationReading::Orientation);

protected:
    void setWindow(ScreenWindow *window);

    void setMirDisplayConfiguration(const miral::Output &, bool notify = true);
    void setMirDisplayBuffer(mir::graphics::DisplayBuffer *, mir::graphics::DisplaySyncGroup *);
    void swapBuffers();
    void makeCurrent();
    void doneCurrent();

private:
    bool internalDisplay() const;

    QRect m_geometry;
    int m_depth;
    QImage::Format m_format;
    qreal m_devicePixelRatio;
    QSizeF m_physicalSize;
    qreal m_refreshRate;
    float m_scale;
    MirFormFactor m_formFactor;
    uint32_t m_currentModeIndex;
    bool m_sensorEnabled;

    mir::renderer::gl::RenderTarget *m_renderTarget;
    mir::graphics::DisplaySyncGroup *m_displayGroup;
    qtmir::OutputId m_outputId;
    qtmir::OutputTypes m_type;
    MirPowerMode m_powerMode;

    Qt::ScreenOrientation m_nativeOrientation;
    Qt::ScreenOrientation m_currentOrientation;

    ScreenWindow *m_screenWindow;
    miral::Output m_output;

    QScopedPointer<qtmir::Cursor> m_cursor;

    friend class ScreensModel;
    friend class ScreenWindow;
};

#endif // SCREEN_H
