#include "screen.h"

// qtmir
#include "platformscreen.h"

#include <QScreen>
#include <QQmlEngine>
#include <QDebug>

Screen::Screen(QScreen* screen, QObject* parent)
    : QObject(parent)
    , m_screen(screen)
{
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (platformScreen) {
        connect(platformScreen, &PlatformScreen::usedChanged, this, &Screen::usedChanged);
        connect(platformScreen, &PlatformScreen::nameChanged, this, &Screen::nameChanged);
        connect(platformScreen, &PlatformScreen::outputTypeChanged, this, &Screen::outputTypeChanged);
        connect(platformScreen, &PlatformScreen::scaleChanged, this, &Screen::scaleChanged);
        connect(platformScreen, &PlatformScreen::formFactorChanged, this, &Screen::formFactorChanged);
        connect(platformScreen, &PlatformScreen::physicalSizeChanged, this, &Screen::physicalSizeChanged);
        connect(platformScreen, &PlatformScreen::positionChanged, this, &Screen::positionChanged);
        connect(platformScreen, &PlatformScreen::activeChanged, this, &Screen::activeChanged);
        connect(platformScreen, &PlatformScreen::currentModeIndexChanged, this, &Screen::modeChanged);
        connect(platformScreen, &PlatformScreen::availableModesChanged, this, &Screen::updateScreenModes);
    }
    updateScreenModes();
}

Screen::~Screen()
{
    qDebug() << "delete screens";
    qDeleteAll(m_modes);
    m_modes.clear();
}

bool Screen::used() const
{
    if (!m_screen) return false;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return false;

    return platformScreen->used();
}

QString Screen::name() const
{
    if (!m_screen) return QString();
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return QString();

    return platformScreen->name();
}

float Screen::scale() const
{
    if (!m_screen) return 1.0;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return 1.0;

    return platformScreen->scale();
}

QSizeF Screen::physicalSize() const
{
    if (!m_screen) return QSizeF();
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return QSizeF();

    return platformScreen->physicalSize();
}

qtmir::FormFactor Screen::formFactor() const
{
    if (!m_screen) return qtmir::FormFactorUnknown;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return qtmir::FormFactorUnknown;

    return static_cast<qtmir::FormFactor>(platformScreen->formFactor()); // needs compile time check
}

qtmir::OutputTypes Screen::outputType() const
{
    if (!m_screen) return qtmir::Unknown;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return qtmir::Unknown;

    return platformScreen->outputType();
}

QPoint Screen::position() const
{
    if (!m_screen) return QPoint();
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return QPoint();

    return platformScreen->geometry().topLeft();
}

QQmlListProperty<qtmir::ScreenMode> Screen::availableModes()
{
    return QQmlListProperty<qtmir::ScreenMode>(this, m_modes);
}

qtmir::ScreenMode *Screen::mode() const
{
    if (!m_screen) return nullptr;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return nullptr;

    uint32_t index = platformScreen->currentModeIndex();
    if (index >= (uint32_t)m_modes.count()) return nullptr;

    return m_modes[index];
}

bool Screen::isActive() const
{
    if (!m_screen) return false;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return false;

    return platformScreen->isActive();
}

QScreen *Screen::screen() const
{
    return m_screen.data();
}

void Screen::setUsed(bool used)
{
    if (!m_screen) return;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return;

    platformScreen->setUsed(used);
}

void Screen::setScale(float scale)
{
    if (!m_screen) return;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return;

    platformScreen->setScale(scale);

}

void Screen::setFormFactor(qtmir::FormFactor formFactor)
{
    if (!m_screen) return;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return;

    platformScreen->setFormFactor(static_cast<MirFormFactor>(formFactor));

}

void Screen::setCurrentModeIndex(uint32_t currentModeIndex)
{
    if (!m_screen) return;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return;

    platformScreen->setCurrentModeIndex(currentModeIndex);
}

void Screen::setActive(bool active)
{
    if (!m_screen) return;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return;

    platformScreen->setActive(active);
}

void Screen::updateScreenModes()
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
    Q_EMIT modeChanged();
}
