/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef MIRAL_MIRSERVERHOOKS_H
#define MIRAL_MIRSERVERHOOKS_H

#include <memory>
#include <QSharedPointer>

namespace mir { class Server; }
namespace mir { namespace scene { class PromptSessionManager; }}
namespace mir { namespace graphics { class Display; }}

class PromptSessionListener;
class ScreensController;
class ScreensModel;

namespace qtmir
{
class MirServerHooks
{
public:
    MirServerHooks();

    void operator()(mir::Server& server);

    PromptSessionListener *promptSessionListener() const;
    std::shared_ptr<mir::scene::PromptSessionManager> thePromptSessionManager() const;
    std::shared_ptr<mir::graphics::Display> theMirDisplay() const;

    QSharedPointer<ScreensController> createScreensController(QSharedPointer<ScreensModel> const &screensModel) const;

private:
    struct Self;
    std::shared_ptr<Self> self;
};
}

#endif //MIRAL_MIRSERVERHOOKS_H
