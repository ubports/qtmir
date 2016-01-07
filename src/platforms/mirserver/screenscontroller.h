/*
 * Copyright (C) 2015 Canonical, Ltd.
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
#include <QVector>

// Mir
#include <mir/shell/display_configuration_controller.h>

// local
#include "customscreenconfiguration.h"

namespace mir {
    namespace graphics { class Display; }
}

namespace qtmir {

class Screen;

class ScreensController : public QObject
{
    Q_OBJECT
public:
    explicit ScreensController(const std::shared_ptr<mir::graphics::Display> &display,
                               const std::shared_ptr<mir::shell::DisplayConfigurationController> &controller,
                               QObject *parent = 0);

    CustomScreenConfiguration getConfigurationFor(Screen *screen);
    void queueConfigurationChange(CustomScreenConfiguration config);
    void applyConfigurationChanges();

private:
    const std::shared_ptr<mir::graphics::Display> m_display;
    const std::weak_ptr<mir::shell::DisplayConfigurationController> m_displayConfigurationController;
    QVector<CustomScreenConfiguration> m_configurationQueue;
};

} //namespace qtmir

#endif // SCREENSCONTROLLER_H
