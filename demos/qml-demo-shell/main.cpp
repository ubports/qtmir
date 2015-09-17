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
#include <QCommandLineParser>
#include <QtQuick/QQuickView>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QLibrary>
#include <QDebug>
#include <csignal>
#include <libintl.h>
#include "../paths.h"

#include <private/qobject_p.h>

// REMOVEME - Should be able to use qmlscene, but in order to use the mir benchmarking we need
// to parse command line switches. Wait until MIR_SOCKET supported by the benchmark framework.

int main(int argc, const char *argv[])
{
    QGuiApplication::setApplicationName("qml-demo-shell");
    QGuiApplication *application;

    application = new QGuiApplication(argc, (char**)argv);
    QQuickView* view = new QQuickView();
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->setColor("black");
    view->setTitle("Demo Shell");
    
    QUrl source(::qmlDirectory() + "qtmir-demo-shell/qml-demo-shell.qml");

    view->setSource(source);
    QObject::connect(view->engine(), SIGNAL(quit()), application, SLOT(quit()));

    view->showFullScreen();
    int result = application->exec();

    delete view;
    delete application;

    return result;
}
