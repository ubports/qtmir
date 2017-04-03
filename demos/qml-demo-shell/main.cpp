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
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <libintl.h>
#include "../paths.h"
#include "pointerposition.h"
#include "screenwindow.h"
#include "screens.h"

#include <qtmir/mirserverapplication.h>
#include <qtmir/displayconfigurationpolicy.h>
#include <qtmir/sessionauthorizer.h>
#include <qtmir/windowmanagementpolicy.h>
#include <qtmir/displayconfigurationstorage.h>
#include <qtmir/miral/edid.h>


inline QString stringFromEdid(const miral::Edid& edid)
{
    QString str;
    str += QString::fromStdString(edid.vendor);
    str += QString("%1%2").arg(edid.product_code).arg(edid.serial_number);

    for (int i = 0; i < 4; i++) {
        str += QString::fromStdString(edid.descriptors[i].string_value());
    }
    return str;
}

struct DemoDisplayConfigurationPolicy : qtmir::DisplayConfigurationPolicy
{
    void apply_to(mir::graphics::DisplayConfiguration& conf)
    {
        qDebug() << "OVERRIDE qtmir::DisplayConfigurationPolicy::apply_to";
        qtmir::DisplayConfigurationPolicy::apply_to(conf);
    }
};

class DemoWindowManagementPolicy : public qtmir::WindowManagementPolicy
{
public:
    DemoWindowManagementPolicy(const miral::WindowManagerTools &tools, std::shared_ptr<qtmir::WindowManagementPolicyPrivate> dd)
        : qtmir::WindowManagementPolicy(tools, dd)
    {}

    bool handle_pointer_event(const MirPointerEvent *event) override
    {
        return qtmir::WindowManagementPolicy::handle_pointer_event(event);
    }

    bool handle_keyboard_event(const MirKeyboardEvent *event) override
    {
        return qtmir::WindowManagementPolicy::handle_keyboard_event(event);
    }
};

struct DemoDisplayConfigurationStorage : miral::DisplayConfigurationStorage
{
    void save(const miral::DisplayId& displayId, const miral::DisplayConfigurationOptions& options) override
    {
        QFile f(stringFromEdid(displayId.edid) + ".edid");
        qDebug() << "OVERRIDE miral::DisplayConfigurationStorage::save" << f.fileName();

        QJsonObject json;
        if (options.used.is_set()) json.insert("used", options.used.value());
        if (options.clone_output_index.is_set()) json.insert("clone_output_index", static_cast<int>(options.clone_output_index.value()));
        if (options.mode.is_set()) {
            auto const& mode = options.mode.value();

            QString sz(QString("%1x%2").arg(mode.size.width.as_int()).arg(mode.size.height.as_int()));
            QJsonObject jsonMode({
                {"size", sz},
                {"refresh_rate", mode.refresh_rate }
            });
            json.insert("mode", jsonMode);
        }
        if (options.orientation.is_set()) json.insert("orientation", static_cast<int>(options.orientation.value()));
        if (options.form_factor.is_set()) json.insert("form_factor", static_cast<int>(options.form_factor.value()));
        if (options.scale.is_set()) json.insert("scale", options.scale.value());

        if (f.open(QIODevice::WriteOnly)) {
            QJsonDocument saveDoc(json);
            f.write(saveDoc.toJson());
        }
    }

    bool load(const miral::DisplayId& displayId, miral::DisplayConfigurationOptions& options) const override
    {
        QFile f(stringFromEdid(displayId.edid) + ".edid");
        qDebug() << "OVERRIDE miral::DisplayConfigurationStorage::load" << f.fileName();

        if (f.open(QIODevice::ReadOnly)) {
            QByteArray saveData = f.readAll();
            QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

            QJsonObject json(loadDoc.object());
            if (json.contains("used")) options.used = json["used"].toBool();
            if (json.contains("clone_output_index")) options.clone_output_index = json["clone_output_index"].toInt();
            if (json.contains("mode")) {
                QJsonObject jsonMode = json["mode"].toObject();

                if (jsonMode.contains("size") && jsonMode.contains("refresh_rate")) {
                    QString sz(jsonMode["size"].toString());
                    QStringList geo = sz.split("x", QString::SkipEmptyParts);
                    if (geo.count() == 2) {
                        miral::DisplayConfigurationOptions::DisplayMode mode;
                        mode.size = mir::geometry::Size(geo[0].toInt(), geo[1].toInt());
                        mode.refresh_rate = jsonMode["refresh_rate"].toDouble();
                        options.mode = mode;
                    }
                }
            }
            if (json.contains("orientation")) options.orientation = static_cast<MirOrientation>(json["orientation"].toInt());
            if (json.contains("form_factor")) options.form_factor = static_cast<MirFormFactor>(json["form_factor"].toInt());
            if (json.contains("scale")) options.scale = json["form_factor"].toDouble();

            return true;
        }

        return false;
    }
};

struct DemoSessionAuthorizer : qtmir::SessionAuthorizer
{
    bool connectionIsAllowed(miral::ApplicationCredentials const& creds) override
    {
        qDebug() << "OVERRIDE qtmir::SessionAuthorizer::connectionIsAllowed";
        return qtmir::SessionAuthorizer::connectionIsAllowed(creds);
    }
    bool configureDisplayIsAllowed(miral::ApplicationCredentials const& creds) override
    {
        qDebug() << "OVERRIDE qtmir::SessionAuthorizer::configureDisplayIsAllowed";
        return qtmir::SessionAuthorizer::configureDisplayIsAllowed(creds);
    }
    bool setBaseDisplayConfigurationIsAllowed(miral::ApplicationCredentials const& creds) override
    {
        qDebug() << "OVERRIDE qtmir::SessionAuthorizer::setBaseDisplayConfigurationIsAllowed";
        return qtmir::SessionAuthorizer::setBaseDisplayConfigurationIsAllowed(creds);
    }
    bool screencastIsAllowed(miral::ApplicationCredentials const& creds) override
    {
        qDebug() << "OVERRIDE qtmir::SessionAuthorizer::screencastIsAllowed";
        return qtmir::SessionAuthorizer::screencastIsAllowed(creds);
    }
    bool promptSessionIsAllowed(miral::ApplicationCredentials const& creds) override
    {
        qDebug() << "OVERRIDE qtmir::SessionAuthorizer::promptSessionIsAllowed";
        return qtmir::SessionAuthorizer::promptSessionIsAllowed(creds);
    }
};

int main(int argc, const char *argv[])
{
    qtmir::SetSessionAuthorizer<DemoSessionAuthorizer> sessionAuth;
    qtmir::SetDisplayConfigurationPolicy<DemoDisplayConfigurationPolicy> displayConfig;
    qtmir::SetWindowManagementPolicy<DemoWindowManagementPolicy> wmPolicy;

    qtmir::SetDisplayConfigurationStorage<DemoDisplayConfigurationStorage> displayStorage;

    setenv("QT_QPA_PLATFORM_PLUGIN_PATH", qPrintable(::qpaPluginDirectory()), 1 /* overwrite */);

    qtmir::MirServerApplication::setApplicationName("api-demo-shell");
    qtmir::MirServerApplication *application;

    application = new qtmir::MirServerApplication(argc, (char**)argv, { displayConfig, sessionAuth, wmPolicy, displayStorage });
    auto qmlEngine = new QQmlApplicationEngine(application);
    qmlEngine->setBaseUrl(QUrl::fromLocalFile(::qmlDirectory() + "api-demo-shell"));
    qmlEngine->addImportPath(::qmlPluginDirectory());
    QObject::connect(qmlEngine, &QQmlEngine::quit, application, &QGuiApplication::quit);

    qmlRegisterSingletonType<PointerPosition>("Mir.Pointer", 0, 1, "PointerPosition",
        [](QQmlEngine*, QJSEngine*) -> QObject* { return PointerPosition::instance(); });

    qmlRegisterType<ScreenWindow>("QtMir", 0, 1, "ScreenWindow");
    qmlRegisterSingletonType<Screens>("QtMir", 0, 1, "Screens",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
            static Screens* screens = new Screens();
            return screens;
    });
    qmlRegisterUncreatableType<qtmir::ScreenMode>("QtMir", 0, 1, "ScreenMode", "ScreenMode is not creatable.");

    qmlEngine->load(::qmlDirectory() + "qml-demo-shell/qml-demo-shell.qml");

    int result = application->exec();
    delete application;

    return result;
}
