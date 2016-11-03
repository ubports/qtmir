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

// Qt
#include <QQmlExtensionPlugin>

// local
#include "application.h"
#include "application_manager.h"
#include "mirsurfaceinterface.h"
#include "mirsurfaceitem.h"
#include "mirsurfacelistmodel.h"
#include "toplevelwindowmodel.h"
#include "window.h"
#include "windowmodel.h"

// platforms/mirserver
#include <mirsingleton.h>

// qtmir
#include "logging.h"

// unity-api
#include <unity/shell/application/Mir.h>

using namespace qtmir;

namespace {
QObject* applicationManagerSingleton(QQmlEngine* engine, QJSEngine* scriptEngine) {
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);
    qCDebug(QTMIR_APPLICATIONS) << "applicationManagerSingleton - engine=" << engine << "scriptEngine=" << scriptEngine;

    return qtmir::ApplicationManager::singleton();
}

QObject* mirSingleton(QQmlEngine* /*engine*/, QJSEngine* /*scriptEngine*/) {
    return qtmir::Mir::instance();
}
} // anonymous namespace

class UnityApplicationPlugin : public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface/1.0")

    virtual void registerTypes(const char* uri)
    {
        qCDebug(QTMIR_APPLICATIONS) << "UnityApplicationPlugin::registerTypes - this=" << this << "uri=" << uri;
        Q_ASSERT(QLatin1String(uri) == QLatin1String("Unity.Application"));

        qRegisterMetaType<qtmir::ApplicationManager*>("ApplicationManager*"); //need for queueing signals
        qRegisterMetaType<qtmir::Application*>("Application*");
        qRegisterMetaType<unity::shell::application::MirSurfaceInterface*>("MirSurfaceInterface*");
        qRegisterMetaType<unity::shell::application::MirSurfaceListInterface*>("unity::shell::application::MirSurfaceListInterface*");
        qRegisterMetaType<MirSurfaceAttrib>("MirSurfaceAttrib");
        qRegisterMetaType<unity::shell::application::WindowInterface*>("unity::shell::application::WindowInterface*");

        qmlRegisterUncreatableType<unity::shell::application::ApplicationManagerInterface>(
                    uri, 0, 1, "ApplicationManagerInterface", "Abstract interface. Cannot be created in QML");
        qmlRegisterSingletonType<qtmir::ApplicationManager>(
                    uri, 0, 1, "ApplicationManager", applicationManagerSingleton);
        qmlRegisterUncreatableType<unity::shell::application::ApplicationInfoInterface>(
                    uri, 0, 1, "ApplicationInfoInterface", "Abstract interface. Cannot be created in QML");
        qmlRegisterUncreatableType<qtmir::Application>(
                    uri, 0, 1, "ApplicationInfo", "Application can't be instantiated");
        qmlRegisterUncreatableType<unity::shell::application::MirSurfaceInterface>(
                    uri, 0, 1, "MirSurface", "MirSurface can't be instantiated from QML");
        qmlRegisterType<qtmir::MirSurfaceItem>(uri, 0, 1, "MirSurfaceItem");
        qmlRegisterSingletonType<qtmir::Mir>(uri, 0, 1, "Mir", mirSingleton);

        qmlRegisterType<qtmir::WindowModel>(uri, 0, 1, "WindowModel");
        qmlRegisterType<qtmir::TopLevelWindowModel>(uri, 0, 1, "TopLevelWindowModel");
    }

    virtual void initializeEngine(QQmlEngine *engine, const char *uri)
    {
        QQmlExtensionPlugin::initializeEngine(engine, uri);
    }
};

#include "plugin.moc"
