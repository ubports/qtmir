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
    connect(platformScreen, &PlatformScreen::usedChanged, this, &ScreenAdaptor::usedChanged);
    connect(platformScreen, &PlatformScreen::nameChanged, this, &ScreenAdaptor::nameChanged);
    connect(platformScreen, &PlatformScreen::outputTypeChanged, this, &ScreenAdaptor::outputTypeChanged);
    connect(platformScreen, &PlatformScreen::scaleChanged, this, &ScreenAdaptor::scaleChanged);
    connect(platformScreen, &PlatformScreen::formFactorChanged, this, &ScreenAdaptor::formFactorChanged);
    connect(platformScreen, &PlatformScreen::powerModeChanged, this, &ScreenAdaptor::powerModeChanged);
    connect(platformScreen, &PlatformScreen::physicalSizeChanged, this, &ScreenAdaptor::physicalSizeChanged);
    connect(platformScreen, &PlatformScreen::positionChanged, this, &ScreenAdaptor::positionChanged);
    connect(platformScreen, &PlatformScreen::activeChanged, this, &ScreenAdaptor::activeChanged);
    connect(platformScreen, &PlatformScreen::currentModeIndexChanged, this, &ScreenAdaptor::currentModeIndexChanged);
    connect(platformScreen, &PlatformScreen::availableModesChanged, this, &ScreenAdaptor::updateScreenModes);
    connect(screen, &QScreen::orientationChanged, this, &ScreenAdaptor::orientationChanged);

    updateScreenModes();
}

ScreenAdaptor::~ScreenAdaptor()
{
    qDeleteAll(m_modes);
    m_modes.clear();
}

qtmir::OutputId ScreenAdaptor::outputId() const
{
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    return platformScreen->outputId();
}

bool ScreenAdaptor::used() const
{
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    return platformScreen->used();
}

QString ScreenAdaptor::name() const
{
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    return platformScreen->name();
}

float ScreenAdaptor::scale() const
{
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    return platformScreen->scale();
}

QSizeF ScreenAdaptor::physicalSize() const
{
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    return platformScreen->physicalSize();
}

qtmir::FormFactor ScreenAdaptor::formFactor() const
{
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    return static_cast<qtmir::FormFactor>(platformScreen->formFactor()); // needs compile time check
}

qtmir::OutputTypes ScreenAdaptor::outputType() const
{
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    return platformScreen->outputType();
}

MirPowerMode ScreenAdaptor::powerMode() const
{
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    return platformScreen->powerMode();
}

Qt::ScreenOrientation ScreenAdaptor::orientation() const
{
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    return platformScreen->orientation();
}

QPoint ScreenAdaptor::position() const
{
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    return platformScreen->geometry().topLeft();
}

QQmlListProperty<qtmir::ScreenMode> ScreenAdaptor::availableModes()
{
    return QQmlListProperty<qtmir::ScreenMode>(this, m_modes);
}

uint ScreenAdaptor::currentModeIndex() const
{
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    return platformScreen->currentModeIndex();
}

bool ScreenAdaptor::isActive() const
{
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
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
    return m_screensController->setOutputConfiguration(*configuration);
}

void ScreenAdaptor::setActive(bool active)
{
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    platformScreen->setActive(active);
}

void ScreenAdaptor::updateScreenModes()
{
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());

    qDeleteAll(m_modes);
    m_modes.clear();
    Q_FOREACH(auto mode, platformScreen->availableModes()) {
        auto newMode(new qtmir::ScreenMode(mode.first, mode.second));
        QQmlEngine::setObjectOwnership(newMode, QQmlEngine::CppOwnership);
        m_modes.append(newMode);
    }

    Q_EMIT availableModesChanged();
}
