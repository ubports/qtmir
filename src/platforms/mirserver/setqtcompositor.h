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

#ifndef QTMIR_SET_QT_COMPOSITOR_H
#define QTMIR_SET_QT_COMPOSITOR_H

#include <QSharedPointer>

#include <memory>

namespace mir { class Server; }

class QtCompositor;
class ScreensModel;

namespace qtmir
{
// Configure the server for using the Qt compositor
class SetQtCompositor
{
public:
    explicit SetQtCompositor(std::shared_ptr<ScreensModel> const& screensModel);

    void operator()(mir::Server& server);

private:
    std::shared_ptr<ScreensModel> const m_screensModel;
    std::weak_ptr<QtCompositor> m_compositor;
};
}

#endif //QTMIR_SET_QT_COMPOSITOR_H
