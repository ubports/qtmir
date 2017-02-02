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

#ifndef QTMIR_PLATFORMSCREEN_H
#define QTMIR_PLATFORMSCREEN_H

// Qt
#include <QObject>
#include <QScopedPointer>
#include <QTimer>
#include <QtDBus/QDBusInterface>
#include <qpa/qplatformscreen.h>
#include <QQmlListProperty>

// Mir
#include <mir_toolkit/common.h>

// local
#include "cursor.h"
#include "screenwindow.h"
#include "screentypes.h"

class QOrientationSensor;
class ScreensController;

namespace mir {
    namespace graphics { class DisplayBuffer; class DisplaySyncGroup; class DisplayConfigurationOutput; }
    namespace renderer { namespace gl { class RenderTarget; }}
}

class PlatformScreen : public QObject,
                       public QPlatformScreen
{
    Q_OBJECT
public:
    PlatformScreen(const mir::graphics::DisplayConfigurationOutput &);
    ~PlatformScreen();

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
    QWindow *topLevelAt(const QPoint &point) const;

    bool used() const { return m_used; }
    float scale() const { return m_scale; }
    MirFormFactor formFactor() const { return m_formFactor; }
    MirPowerMode powerMode() const { return m_powerMode; }
    qtmir::OutputId outputId() const { return m_outputId; }
    qtmir::OutputTypes outputType() const { return m_type; }
    uint32_t currentModeIndex() const { return m_currentModeIndex; }
    bool isActive() const { return m_isActive; }

    typedef QPair<qreal, QSize> Mode;
    QList<Mode> availableModes() const;

    const QVector<ScreenWindow*>& windows() const { return m_screenWindows; }
    ScreenWindow* primaryWindow() const;

    // QObject methods.
    void customEvent(QEvent* event) override;

    // To make it testable
    static bool skipDBusRegistration;
    bool orientationSensorEnabled();

    void setUsed(bool used);
    void setScale(float scale);
    void setFormFactor(MirFormFactor formFactor);
    void setCurrentModeIndex(uint32_t currentModeIndex);
    void setActive(bool active);

Q_SIGNALS:
    void primaryWindowChanged(ScreenWindow* window);

    void usedChanged();
    void nameChanged();
    void outputTypeChanged();
    void scaleChanged();
    void formFactorChanged();
    void currentModeIndexChanged();
    void positionChanged();
    void modeChanged();
    void physicalSizeChanged();
    void availableModesChanged();
    void activeChanged(bool active);

public Q_SLOTS:
   void onDisplayPowerStateChanged(int, int);
   void onOrientationReadingChanged();

    void activate();

protected:
    void addWindow(ScreenWindow *window);
    void removeWindow(ScreenWindow *window);

    void setMirDisplayConfiguration(const mir::graphics::DisplayConfigurationOutput &, bool notify = true);
    void setMirDisplayBuffer(mir::graphics::DisplayBuffer *, mir::graphics::DisplaySyncGroup *);
    void swapBuffers();
    void makeCurrent();
    void doneCurrent();

private:
    void toggleSensors(const bool enable) const;
    bool internalDisplay() const;

    bool m_used;
    QRect m_geometry;
    int m_depth;
    QImage::Format m_format;
    qreal m_devicePixelRatio;
    QSizeF m_physicalSize;
    qreal m_refreshRate;
    float m_scale;
    MirFormFactor m_formFactor;
    uint32_t m_currentModeIndex;
    QList<PlatformScreen::Mode> m_availableModes;
    bool m_isActive;

    mir::renderer::gl::RenderTarget *m_renderTarget;
    mir::graphics::DisplaySyncGroup *m_displayGroup;
    qtmir::OutputId m_outputId;
    qtmir::OutputTypes m_type;
    MirPowerMode m_powerMode;

    Qt::ScreenOrientation m_nativeOrientation;
    Qt::ScreenOrientation m_currentOrientation;
    QOrientationSensor *m_orientationSensor;

    QVector<ScreenWindow*> m_screenWindows;
    QDBusInterface *m_unityScreen;
    ScreensController *m_screensController;

    QScopedPointer<qtmir::Cursor> m_cursor;

    friend class ScreensModel;
    friend class ScreenWindow;
};

#endif // QTMIR_PLATFORMSCREEN_H
