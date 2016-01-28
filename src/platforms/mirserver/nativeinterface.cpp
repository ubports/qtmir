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

#include "nativeinterface.h"

#include "mirserver.h"
#include "screen.h"

NativeInterface::NativeInterface(QMirServer *server)
    : m_qMirServer(server)
{
}

void *NativeInterface::nativeResourceForIntegration(const QByteArray &resource)
{
    void *result = nullptr;

    auto const server = m_qMirServer->mirServer().lock();

    if (server) {
        if (resource == "SessionAuthorizer")
            result = server->sessionAuthorizer();
        else if (resource == "Shell")
            result = server->shell();
        else if (resource == "SessionListener")
            result = server->sessionListener();
        else if (resource == "PromptSessionListener")
            result = server->promptSessionListener();
        else if (resource == "ScreensController")
            result = m_qMirServer->screensController().data();
    }
    return result;
}

// Changes to these properties are emitted via the UbuntuNativeInterface::windowPropertyChanged
// signal fired via UbuntuScreen. Connect to this signal for these properties updates.
QVariantMap NativeInterface::windowProperties(QPlatformWindow *window) const
{
    QVariantMap propertyMap;
    auto w = static_cast<ScreenWindow*>(window);
    auto s = static_cast<Screen*>(w->screen());
    if (s) {
        propertyMap.insert("scale", s->scale());
        propertyMap.insert("formFactor", s->formFactor());
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
        return s->formFactor();
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
