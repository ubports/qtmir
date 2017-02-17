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

#include "screenadaptor.h"

// qtmir
#include "platformscreen.h"
#include "screenscontroller.h"
#include "nativeinterface.h"

// Qt
#include <QScreen>
#include <QQmlEngine>
#include <QDebug>
#include <QGuiApplication>

ScreenAdaptor::ScreenAdaptor(QScreen* screen, QObject* parent)
    : qtmir::Screen(parent)
    , m_screen(screen)
    , m_screensController(static_cast<ScreensController*>(qGuiApp->platformNativeInterface()
                                                          ->nativeResourceForIntegration("ScreensController")))
{
    if (!m_screensController) {
        qFatal("Screens Controller not initialized");
    }

    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (platformScreen) {
        connect(platformScreen, &PlatformScreen::usedChanged, this, &ScreenAdaptor::usedChanged);
        connect(platformScreen, &PlatformScreen::nameChanged, this, &ScreenAdaptor::nameChanged);
        connect(platformScreen, &PlatformScreen::outputTypeChanged, this, &ScreenAdaptor::outputTypeChanged);
        connect(platformScreen, &PlatformScreen::scaleChanged, this, &ScreenAdaptor::scaleChanged);
        connect(platformScreen, &PlatformScreen::formFactorChanged, this, &ScreenAdaptor::formFactorChanged);
        connect(platformScreen, &PlatformScreen::physicalSizeChanged, this, &ScreenAdaptor::physicalSizeChanged);
        connect(platformScreen, &PlatformScreen::positionChanged, this, &ScreenAdaptor::positionChanged);
        connect(platformScreen, &PlatformScreen::activeChanged, this, &ScreenAdaptor::activeChanged);
        connect(platformScreen, &PlatformScreen::currentModeIndexChanged, this, &ScreenAdaptor::currentModeIndexChanged);
        connect(platformScreen, &PlatformScreen::availableModesChanged, this, &ScreenAdaptor::updateScreenModes);
    }
    updateScreenModes();
}

ScreenAdaptor::~ScreenAdaptor()
{
    qDeleteAll(m_modes);
    m_modes.clear();
}

qtmir::OutputId ScreenAdaptor::outputId() const
{
    if (!m_screen) return qtmir::OutputId(-1);
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return qtmir::OutputId(-1);

    return platformScreen->outputId();
}

bool ScreenAdaptor::used() const
{
    if (!m_screen) return false;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return false;

    return platformScreen->used();
}

QString ScreenAdaptor::name() const
{
    if (!m_screen) return QString();
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return QString();

    return platformScreen->name();
}

float ScreenAdaptor::scale() const
{
    if (!m_screen) return 1.0;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return 1.0;

    return platformScreen->scale();
}

QSizeF ScreenAdaptor::physicalSize() const
{
    if (!m_screen) return QSizeF();
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return QSizeF();

    return platformScreen->physicalSize();
}

qtmir::FormFactor ScreenAdaptor::formFactor() const
{
    if (!m_screen) return qtmir::FormFactorUnknown;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return qtmir::FormFactorUnknown;

    return static_cast<qtmir::FormFactor>(platformScreen->formFactor()); // needs compile time check
}

qtmir::OutputTypes ScreenAdaptor::outputType() const
{
    if (!m_screen) return qtmir::Unknown;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return qtmir::Unknown;

    return platformScreen->outputType();
}

QPoint ScreenAdaptor::position() const
{
    if (!m_screen) return QPoint();
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return QPoint();

    return platformScreen->geometry().topLeft();
}

QQmlListProperty<qtmir::ScreenMode> ScreenAdaptor::availableModes()
{
    return QQmlListProperty<qtmir::ScreenMode>(this, m_modes);
}

uint ScreenAdaptor::currentModeIndex() const
{
    if (!m_screen) return -1;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return -1;

    return platformScreen->currentModeIndex();
}

bool ScreenAdaptor::isActive() const
{
    if (!m_screen) return false;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return false;

    return platformScreen->isActive();
}

QScreen *ScreenAdaptor::qscreen() const
{
    return m_screen.data();
}

qtmir::ScreenConfiguration *ScreenAdaptor::beginConfiguration() const
{
    auto config = m_screensController->outputConfiguration(this->outputId());
    return new qtmir::ScreenConfiguration(config);
}

bool ScreenAdaptor::applyConfiguration(qtmir::ScreenConfiguration *configuration)
{
    if (!m_screen) return false;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return false;

    return m_screensController->setOutputConfiguration(*configuration);
}

void ScreenAdaptor::setActive(bool active)
{
    if (!m_screen) return;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return;

    platformScreen->setActive(active);
}

void ScreenAdaptor::updateScreenModes()
{
    if (!m_screen) return;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return;

    qDeleteAll(m_modes);
    m_modes.clear();
    Q_FOREACH(auto mode, platformScreen->availableModes()) {
        auto newMode(new qtmir::ScreenMode);
        QQmlEngine::setObjectOwnership(newMode, QQmlEngine::CppOwnership);
        newMode->refreshRate = mode.first;
        newMode->size = mode.second;
        m_modes.append(newMode);
    }

    Q_EMIT availableModesChanged();
}
