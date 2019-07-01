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

#include "setqtcompositor.h"

// local
#include "qtcompositor.h"
#include "screensmodel.h"
#include "logging.h"

// mir
#include <mir/compositor/compositor.h>
#include <mir/server.h>
#include <mir/shell/shell.h>
#include <mir/observer_registrar.h>

namespace
{
struct QtCompositorImpl : QtCompositor, mir::compositor::Compositor
{
    void start();
    void stop();
};

void QtCompositorImpl::start()
{
    qCDebug(QTMIR_SCREENS) << "QtCompositor::start";

    Q_EMIT starting(); // blocks
}

void QtCompositorImpl::stop()
{
    qCDebug(QTMIR_SCREENS) << "QtCompositor::stop";

    Q_EMIT stopping(); // blocks
}
}


qtmir::SetQtCompositor::SetQtCompositor(std::shared_ptr<ScreensModel> const& screensModel) :
    m_screensModel{screensModel}
{
}

void qtmir::SetQtCompositor::operator()(mir::Server& server)
{
    server.override_the_compositor([this]
    {
        auto result = std::make_shared<QtCompositorImpl>();
        m_compositor = result;
        return result;
    });

    server.add_init_callback([&, this]
        {
            if (auto const compositor = m_compositor.lock())
            {
                server.the_display_configuration_observer_registrar()->register_interest(m_screensModel);
                m_screensModel->init(server.the_display(), compositor, server.the_shell());
            }
            else
            {
                throw std::logic_error("No m_compositor available. Server not running?");
            }
        });
}

