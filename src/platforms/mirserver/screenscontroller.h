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

#ifndef SCREENSCONTROLLER_H
#define SCREENSCONTROLLER_H

// Qt
#include <QObject>
#include <QSharedPointer>
#include <QVector>

// local
#include "customscreenconfiguration.h"

#include <memory>

class ScreensModel;

namespace mir {
    namespace graphics { class Display; }
    namespace shell { class DisplayConfigurationController; }
}


class ScreensController : public QObject
{
    Q_OBJECT

public:
    explicit ScreensController(const QSharedPointer<ScreensModel> &model,
                               const std::shared_ptr<mir::graphics::Display> &display,
                               const std::shared_ptr<mir::shell::DisplayConfigurationController> &controller,
                               QObject *parent = 0);

    CustomScreenConfigurationList configuration();
    bool setConfiguration(const CustomScreenConfigurationList &newConfig);

    CustomScreenConfiguration outputConfiguration(qtmir::OutputId outputId);
    bool setOutputConfiguration(const CustomScreenConfiguration &newConfig);

private:
    const QSharedPointer<ScreensModel> m_screensModel;
    const std::shared_ptr<mir::graphics::Display> m_display;
    const std::shared_ptr<mir::shell::DisplayConfigurationController> m_displayConfigurationController;
};

#endif // SCREENSCONTROLLER_H
