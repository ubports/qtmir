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

#include "nativeinterface.h"

#include "qmirserver.h"
#include "screen.h"
#include "windowcontrollerinterface.h"

#include <QDebug>

NativeInterface::NativeInterface(QMirServer *server)
    : m_qMirServer(server)
{
}

void *NativeInterface::nativeResourceForIntegration(const QByteArray &resource)
{
    return m_qMirServer->nativeResourceForIntegration(resource);
}

// Changes to these properties are emitted via the UbuntuNativeInterface::windowPropertyChanged
// signal fired via UbuntuScreen. Connect to this signal for these properties updates.
QVariantMap NativeInterface::windowProperties(QPlatformWindow *window) const
{
    QVariantMap propertyMap;
    auto w = static_cast<ScreenWindow*>(window);
    auto s = static_cast<Screen*>(w->screen());
    if (s) {
        propertyMap.insert(QStringLiteral("scale"), s->scale());
        propertyMap.insert(QStringLiteral("formFactor"), s->formFactor());
    }
    return propertyMap;
}

QVariant NativeInterface::windowProperty(QPlatformWindow *window, const QString &name) const
{
    if (!window || name.isNull()) {
        return QVariant();
    }
    auto w = static_cast<ScreenWindow*>(window);
    auto s = static_cast<Screen*>(w->screen());
    if (!s) {
        return QVariant();
    }

    if (name == QStringLiteral("scale")) {
        return s->scale();
    } else if (name == QStringLiteral("formFactor")) {
        return static_cast<int>(s->formFactor()); // naughty, should add enum to Qt's Type system
    } else {
        return QVariant();
    }
}

QVariant NativeInterface::windowProperty(QPlatformWindow *window, const QString &name, const QVariant &defaultValue) const
{
    QVariant returnVal = windowProperty(window, name);
    if (!returnVal.isValid()) {
        return defaultValue;
    } else {
        return returnVal;
    }
}

void NativeInterface::setWindowProperty(QPlatformWindow */*window*/, const QString &name, const QVariant &value)
{
    if (name.isNull()) {
        return;
    }

    // TODO remove this hack and expose this properly when QtMir can share WindowController in a C++ header

    // Get WindowController
    auto windowController = static_cast<qtmir::WindowControllerInterface *>(m_qMirServer->nativeResourceForIntegration("WindowController"));

    if (name == QStringLiteral("availableDesktopArea")) {
        QRect rect = value.toRect();
        if (rect.isValid()) {
            windowController->setWindowConfinementRegions({rect});
        } else {
            qWarning().nospace() << "NativeInterface::setWindowProperty("
                << name << "," << value << ") - value is not a QRect";
       }
    } else if (name.endsWith(QStringLiteral("WindowMargins"))) {
        if (value.canConvert(QMetaType::QRect)) {
            QRect rect = value.toRect();
            QMargins margin(rect.x(), rect.y(), rect.width(), rect.height());
            if (name == QStringLiteral("normalWindowMargins")) {
                windowController->setWindowMargins(Mir::NormalType, margin);
            } else if (name == QStringLiteral("dialogWindowMargins")) {
                windowController->setWindowMargins(Mir::DialogType, margin);
            } else {
                qWarning() << "NativeInterface::setWindowProperty missing support for:" << name;
            }
        } else {
            qWarning().nospace() << "NativeInterface::setWindowProperty("
                << name << "," << value << ") - value is not a QRect";
        }
    }
}

std::shared_ptr<qtmir::PromptSessionManager> NativeInterface::thePromptSessionManager() const
{
    return m_qMirServer->thePromptSessionManager();
}

