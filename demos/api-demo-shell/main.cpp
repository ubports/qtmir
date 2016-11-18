/*
 * Copyright (C) 2012-2015 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Qt
#include <QtQuick/QQuickView>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QDebug>
#include <libintl.h>
#include "../paths.h"
#include "../qml-demo-shell/pointerposition.h"

#include <qtmir/guiserverapplication.h>
#include <qtmir/displayconfigurationpolicy.h>
#include <qtmir/sessionauthorizer.h>

// REMOVEME - Should be able to use qmlscene, but in order to use the mir benchmarking we need
// to parse command line switches. Wait until MIR_SOCKET supported by the benchmark framework.

class MyDisplayConfigurationPolicy : public qtmir::DisplayConfigurationPolicy
{
public:
    MyDisplayConfigurationPolicy(std::shared_ptr<mg::DisplayConfigurationPolicy> const&)
    {
    }

    void apply_to(mir::graphics::DisplayConfiguration& conf)
    {
        qDebug() << "OVERRIDE APPLY TO";
        qtmir::DisplayConfigurationPolicy::apply_to(conf);
    }
};

class MySessionAuthorizer : public qtmir::SessionAuthorizer
{
    bool connection_is_allowed(miral::ApplicationCredentials const& creds) override
    {
        qDebug() << "OVERRIDE connection_is_allowed";
        return qtmir::SessionAuthorizer::connection_is_allowed(creds);
    }
    bool configure_display_is_allowed(miral::ApplicationCredentials const& creds) override
    {
        qDebug() << "OVERRIDE configure_display_is_allowed";
        return qtmir::SessionAuthorizer::configure_display_is_allowed(creds);
    }
    bool set_base_display_configuration_is_allowed(miral::ApplicationCredentials const& creds) override
    {
        qDebug() << "OVERRIDE set_base_display_configuration_is_allowed";
        return qtmir::SessionAuthorizer::set_base_display_configuration_is_allowed(creds);
    }
    bool screencast_is_allowed(miral::ApplicationCredentials const& creds) override
    {
        qDebug() << "OVERRIDE screencast_is_allowed";
        return qtmir::SessionAuthorizer::screencast_is_allowed(creds);
    }
    bool prompt_session_is_allowed(miral::ApplicationCredentials const& creds) override
    {
        qDebug() << "OVERRIDE prompt_session_is_allowed";
        return qtmir::SessionAuthorizer::prompt_session_is_allowed(creds);
    }
};

int main(int argc, const char *argv[])
{
    qtmir::SetSessionAuthorizer<MySessionAuthorizer> sessionAuth;
    qtmir::SetDisplayConfigurationPolicy<MyDisplayConfigurationPolicy> displayConfig;

    setenv("QT_QPA_PLATFORM_PLUGIN_PATH", qPrintable(::qpaPluginDirectory()), 1 /* overwrite */);

    qtmir::GuiServerApplication::setApplicationName("api-demo-shell");
    qtmir::GuiServerApplication *application;

    application = new qtmir::GuiServerApplication(argc, (char**)argv, { displayConfig, sessionAuth });
    QQuickView* view = new QQuickView();
    view->engine()->addImportPath(::qmlPluginDirectory());
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->setColor("lightgray");
    view->setTitle("Demo Shell");

    qmlRegisterSingletonType<PointerPosition>("Mir.Pointer", 0, 1, "PointerPosition",
        [](QQmlEngine*, QJSEngine*) -> QObject* { return PointerPosition::instance(); });

    QUrl source(::qmlDirectory() + "qml-demo-shell/windowModel.qml");

    view->setSource(source);
    QObject::connect(view->engine(), SIGNAL(quit()), application, SLOT(quit()));

    view->showFullScreen();
    int result = application->exec();

    delete view;
    delete application;

    return result;
}
