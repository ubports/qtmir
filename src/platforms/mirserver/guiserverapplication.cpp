/*
 * Copyright (C) 2016 Canonical, Ltd.
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

#include "qtmir/guiserverapplication.h"
#include "qmirserver.h"
#include "screenadaptormodel.h"

#include <QDebug>

namespace qtmir
{
namespace
{

QSharedPointer<QMirServer> mirServer;

void init(int &argc, char **argv, std::initializer_list<std::function<void(QMirServer&)>> const& options)
{
    setenv("QT_QPA_PLATFORM", "mirserver", 1 /* overwrite */);

    mirServer = QMirServer::create(argc, argv);
    for (auto& option : options) {
        option(*mirServer.data());
    }
}

}

struct GuiServerApplication::Private
{
    QScopedPointer<ScreenAdaptorModel> screenModel{new ScreenAdaptorModel()};
};

GuiServerApplication::GuiServerApplication(int &argc,
                                           char **argv,
                                           std::initializer_list<std::function<void(QMirServer&)>> options)
    : QGuiApplication((init(argc, argv, options), argc), argv) // comma operator to ensure init called before QGuiApplication
    , d(new Private)
{
    Q_UNUSED(options);
}

GuiServerApplication::~GuiServerApplication()
{
    mirServer.clear();
}

AppNotifier *GuiServerApplication::appNotifier() const
{
    return mirServer->appNotifier();
}

WindowModelNotifier *GuiServerApplication::windowModelNotifier() const
{
    return mirServer->windowModelNotifier();
}

ScreenModel *GuiServerApplication::screenModel() const
{
    return d->screenModel.data();
}

}
