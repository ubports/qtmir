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
#include "platformscreen.h"

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
    auto s = static_cast<PlatformScreen*>(w->screen());
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
    auto s = static_cast<PlatformScreen*>(w->screen());
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

std::shared_ptr<qtmir::PromptSessionManager> NativeInterface::thePromptSessionManager() const
{
    return m_qMirServer->thePromptSessionManager();
}

