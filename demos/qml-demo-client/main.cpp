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
#include <QDebug>
#include <csignal>
#include <libintl.h>
#include <getopt.h>
#include "../paths.h"

// REMOVEME - Should be able to use qmlscene, but in order to use the mir benchmarking we need
// to parse command line switches. Wait until MIR_SOCKET supported by the benchmark framework.

int main(int argc, char **argv)
{
    int arg;
    opterr = 0;
    while ((arg = getopt (argc, argv, "hm:")) != -1)
    {
        switch (arg)
        {
        case 'm':
            setenv("MIR_SOCKET", optarg, 1);
            break;

        case '?':
        case 'h':
        default:
            puts(argv[0]);
            puts("Usage:");
            puts("    -m <Mir server socket>");
            puts("    -h: this help text");
            return -1;
        }
    }

    QGuiApplication::setApplicationName("qml-demo-client");
    QGuiApplication *application;

    application = new QGuiApplication(argc, (char**)argv);
    QQuickView* view = new QQuickView();
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->setColor("black");
    view->setTitle("Demo Client");
    
    QUrl source(::qmlDirectory() + "qtmir-demo-client/qml-demo-client.qml");

    view->setSource(source);
    QObject::connect(view->engine(), SIGNAL(quit()), application, SLOT(quit()));

    view->showFullScreen();
    int result = application->exec();

    delete view;
    delete application;

    return result;
}
