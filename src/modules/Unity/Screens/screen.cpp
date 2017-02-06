#include "screen.h"

// qtmir
#include "platformscreen.h"
#include "screenscontroller.h"
#include "nativeinterface.h"

// Qt
#include <QScreen>
#include <QQmlEngine>
#include <QDebug>
#include <QGuiApplication>

ScreenAdapter::ScreenAdapter(QScreen* screen, QObject* parent)
    : QObject(parent)
    , m_screen(screen)
    , m_screensController(static_cast<ScreensController*>(qGuiApp->platformNativeInterface()
                                                          ->nativeResourceForIntegration("ScreensController")))
{
    if (!m_screensController) {
        qFatal("Screens Controller not initialized");
    }

    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (platformScreen) {
        connect(platformScreen, &PlatformScreen::usedChanged, this, &ScreenAdapter::usedChanged);
        connect(platformScreen, &PlatformScreen::nameChanged, this, &ScreenAdapter::nameChanged);
        connect(platformScreen, &PlatformScreen::outputTypeChanged, this, &ScreenAdapter::outputTypeChanged);
        connect(platformScreen, &PlatformScreen::scaleChanged, this, &ScreenAdapter::scaleChanged);
        connect(platformScreen, &PlatformScreen::formFactorChanged, this, &ScreenAdapter::formFactorChanged);
        connect(platformScreen, &PlatformScreen::physicalSizeChanged, this, &ScreenAdapter::physicalSizeChanged);
        connect(platformScreen, &PlatformScreen::positionChanged, this, &ScreenAdapter::positionChanged);
        connect(platformScreen, &PlatformScreen::activeChanged, this, &ScreenAdapter::activeChanged);
        connect(platformScreen, &PlatformScreen::currentModeIndexChanged, this, &ScreenAdapter::currentModeIndexChanged);
        connect(platformScreen, &PlatformScreen::availableModesChanged, this, &ScreenAdapter::updateScreenModes);
    }
    updateScreenModes();
}

ScreenAdapter::~ScreenAdapter()
{
    qDebug() << "delete screens";
    qDeleteAll(m_modes);
    m_modes.clear();
}

qtmir::OutputId ScreenAdapter::outputId() const
{
    if (!m_screen) return qtmir::OutputId(-1);
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return qtmir::OutputId(-1);

    return platformScreen->outputId();
}

bool ScreenAdapter::used() const
{
    if (!m_screen) return false;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return false;

    return platformScreen->used();
}

QString ScreenAdapter::name() const
{
    if (!m_screen) return QString();
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return QString();

    return platformScreen->name();
}

float ScreenAdapter::scale() const
{
    if (!m_screen) return 1.0;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return 1.0;

    return platformScreen->scale();
}

QSizeF ScreenAdapter::physicalSize() const
{
    if (!m_screen) return QSizeF();
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return QSizeF();

    return platformScreen->physicalSize();
}

qtmir::FormFactor ScreenAdapter::formFactor() const
{
    if (!m_screen) return qtmir::FormFactorUnknown;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return qtmir::FormFactorUnknown;

    return static_cast<qtmir::FormFactor>(platformScreen->formFactor()); // needs compile time check
}

qtmir::OutputTypes ScreenAdapter::outputType() const
{
    if (!m_screen) return qtmir::Unknown;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return qtmir::Unknown;

    return platformScreen->outputType();
}

QPoint ScreenAdapter::position() const
{
    if (!m_screen) return QPoint();
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return QPoint();

    return platformScreen->geometry().topLeft();
}

QQmlListProperty<ScreenMode> ScreenAdapter::availableModes()
{
    return QQmlListProperty<ScreenMode>(this, m_modes);
}

uint ScreenAdapter::currentModeIndex() const
{
    if (!m_screen) return -1;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return -1;

    return platformScreen->currentModeIndex();
}

bool ScreenAdapter::isActive() const
{
    if (!m_screen) return false;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return false;

    return platformScreen->isActive();
}

QScreen *ScreenAdapter::screen() const
{
    return m_screen.data();
}

ScreenConfig *ScreenAdapter::beginConfiguration() const
{
    auto newConfig = new ScreenConfig();
    auto config = m_screensController->outputConfiguration(this->outputId());
    *newConfig = config;

    return newConfig;
}

bool ScreenAdapter::applyConfiguration(ScreenConfig *configuration)
{
    if (!m_screen) return false;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return false;

    return m_screensController->setOutputConfiguration(*configuration);
}

void ScreenAdapter::setActive(bool active)
{
    if (!m_screen) return;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return;

    platformScreen->setActive(active);
}

void ScreenAdapter::updateScreenModes()
{
    if (!m_screen) return;
    auto platformScreen = static_cast<PlatformScreen*>(m_screen->handle());
    if (!platformScreen) return;

    qDeleteAll(m_modes);
    m_modes.clear();
    Q_FOREACH(auto mode, platformScreen->availableModes()) {
        auto newMode(new ScreenMode);
        QQmlEngine::setObjectOwnership(newMode, QQmlEngine::CppOwnership);
        newMode->refreshRate = mode.first;
        newMode->size = mode.second;
        m_modes.append(newMode);
    }

    Q_EMIT availableModesChanged();
}

ScreenConfig::ScreenConfig(QObject *parent)
    : QObject(parent)
{
}

ScreenConfig &ScreenConfig::operator=(const CustomScreenConfiguration &other)
{
    if (&other == this) return *this;

    valid = other.valid;
    id = other.id;
    used = other.used;
    topLeft = other.topLeft;
    currentModeIndex = other.currentModeIndex;
    powerMode = other.powerMode;
    orientation = other.orientation;
    scale = other.scale;
    formFactor = other.formFactor;

    return *this;
}
