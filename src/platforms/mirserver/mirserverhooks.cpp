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

#include "mirserverhooks.h"

#include "mircursorimages.h"
#include "promptsessionlistener.h"
#include "screenscontroller.h"
#include "logging.h"
#include "inputdeviceobserver.h"

// mir
#include <mir/server.h>
#include <mir/graphics/cursor.h>
#include <mir/scene/prompt_session_listener.h>
#include <mir/input/input_device_hub.h>

namespace mg = mir::graphics;
namespace ms = mir::scene;

namespace
{
struct PromptSessionListenerImpl : PromptSessionListener, mir::scene::PromptSessionListener
{
    ~PromptSessionListenerImpl();

    void starting(std::shared_ptr<mir::scene::PromptSession> const& prompt_session) override;
    void stopping(std::shared_ptr<mir::scene::PromptSession> const& prompt_session) override;
    void suspending(std::shared_ptr<mir::scene::PromptSession> const& prompt_session) override;
    void resuming(std::shared_ptr<mir::scene::PromptSession> const& prompt_session) override;

    void prompt_provider_added(mir::scene::PromptSession const& prompt_session,
                               std::shared_ptr<mir::scene::Session> const& prompt_provider) override;
    void prompt_provider_removed(mir::scene::PromptSession const& prompt_session,
                                 std::shared_ptr<mir::scene::Session> const& prompt_provider) override;

private:
    QHash<const mir::scene::PromptSession *, qtmir::PromptSession> m_mirPromptToSessionHash;
};

struct HiddenCursorWrapper : mg::Cursor
{
    HiddenCursorWrapper(std::shared_ptr<mg::Cursor> const& wrapped) :
        wrapped{wrapped} { wrapped->hide(); }
    void show() override { }
    void show(mg::CursorImage const&) override { }
    void hide() override { wrapped->hide(); }

    void move_to(mir::geometry::Point position) override { wrapped->move_to(position); }

private:
    std::shared_ptr<mg::Cursor> const wrapped;
};
}

struct qtmir::MirServerHooks::Self
{
    std::weak_ptr<PromptSessionListener> m_promptSessionListener;
    std::weak_ptr<mir::graphics::Display> m_mirDisplay;
    std::weak_ptr<mir::shell::DisplayConfigurationController> m_mirDisplayConfigurationController;
    std::weak_ptr<mir::scene::PromptSessionManager> m_mirPromptSessionManager;
    std::weak_ptr<mir::input::InputDeviceHub> m_inputDeviceHub;
};

qtmir::MirServerHooks::MirServerHooks() :
    self{std::make_shared<Self>()}
{
}

void qtmir::MirServerHooks::operator()(mir::Server& server)
{
    server.override_the_cursor_images([]
        { return std::make_shared<qtmir::MirCursorImages>(); });

    server.wrap_cursor([&](std::shared_ptr<mg::Cursor> const& wrapped)
        { return std::make_shared<HiddenCursorWrapper>(wrapped); });

    server.override_the_prompt_session_listener([this]
        {
            auto const result = std::make_shared<PromptSessionListenerImpl>();
            self->m_promptSessionListener = result;
            return result;
        });

    server.add_init_callback([this, &server]
        {
            self->m_mirDisplay = server.the_display();
            self->m_mirDisplayConfigurationController = server.the_display_configuration_controller();
            self->m_mirPromptSessionManager = server.the_prompt_session_manager();
            self->m_inputDeviceHub = server.the_input_device_hub();
        });
}

PromptSessionListener *qtmir::MirServerHooks::promptSessionListener() const
{
    if (auto result = self->m_promptSessionListener.lock())
        return result.get();

    throw std::logic_error("No prompt session listener available. Server not running?");
}

std::shared_ptr<mir::scene::PromptSessionManager> qtmir::MirServerHooks::thePromptSessionManager() const
{
    if (auto result = self->m_mirPromptSessionManager.lock())
        return result;

    throw std::logic_error("No prompt session manager available. Server not running?");
}

std::shared_ptr<mir::graphics::Display> qtmir::MirServerHooks::theMirDisplay() const
{
    if (auto result = self->m_mirDisplay.lock())
        return result;

    throw std::logic_error("No display available. Server not running?");
}

std::shared_ptr<mir::input::InputDeviceHub> qtmir::MirServerHooks::theInputDeviceHub() const
{
    if (auto result = self->m_inputDeviceHub.lock())
        return result;

    throw std::logic_error("No input device hub available. Server not running?");
}

QSharedPointer<ScreensController> qtmir::MirServerHooks::createScreensController(std::shared_ptr<ScreensModel> const &screensModel) const
{
    return QSharedPointer<ScreensController>(
        new ScreensController(screensModel, theMirDisplay(), self->m_mirDisplayConfigurationController.lock()));
}

void qtmir::MirServerHooks::createInputDeviceObserver()
{
    theInputDeviceHub()->add_observer(std::make_shared<qtmir::MirInputDeviceObserver>());
}

PromptSessionListenerImpl::~PromptSessionListenerImpl() = default;

void PromptSessionListenerImpl::starting(std::shared_ptr<ms::PromptSession> const& prompt_session)
{
    qCDebug(QTMIR_MIR_MESSAGES) << "PromptSessionListener::starting - this=" << this << "prompt_session=" << prompt_session.get();
    m_mirPromptToSessionHash.insert(prompt_session.get(), prompt_session);
    Q_EMIT promptSessionStarting(prompt_session);
}

void PromptSessionListenerImpl::stopping(std::shared_ptr<ms::PromptSession> const& prompt_session)
{
    qCDebug(QTMIR_MIR_MESSAGES) << "PromptSessionListener::stopping - this=" << this << "prompt_session=" << prompt_session.get();
    Q_EMIT promptSessionStopping(prompt_session);
    m_mirPromptToSessionHash.remove(prompt_session.get());
}

void PromptSessionListenerImpl::suspending(std::shared_ptr<ms::PromptSession> const& prompt_session)
{
    qCDebug(QTMIR_MIR_MESSAGES) << "PromptSessionListener::suspending - this=" << this << "prompt_session=" << prompt_session.get();
    Q_EMIT promptSessionSuspending(prompt_session);
}

void PromptSessionListenerImpl::resuming(std::shared_ptr<ms::PromptSession> const& prompt_session)
{
    qCDebug(QTMIR_MIR_MESSAGES) << "PromptSessionListener::resuming - this=" << this << "prompt_session=" << prompt_session.get();
    Q_EMIT promptSessionResuming(prompt_session);
}

void PromptSessionListenerImpl::prompt_provider_added(ms::PromptSession const& prompt_session,
                                                      std::shared_ptr<ms::Session> const& prompt_provider)
{
    qCDebug(QTMIR_MIR_MESSAGES) << "PromptSessionListener::prompt_provider_added - this=" << this
                                << "prompt_session=" << &prompt_session
                                << "prompt_provider=" << prompt_provider.get();
    Q_EMIT promptProviderAdded(m_mirPromptToSessionHash[&prompt_session], prompt_provider);
}

void PromptSessionListenerImpl::prompt_provider_removed(ms::PromptSession const& prompt_session,
                                                        std::shared_ptr<ms::Session> const& prompt_provider)
{
    qCDebug(QTMIR_MIR_MESSAGES) << "PromptSessionListener::prompt_provider_removed - this=" << this
                                << "prompt_session=" << &prompt_session
                                << "prompt_provider=" << prompt_provider.get();
    Q_EMIT promptProviderRemoved(m_mirPromptToSessionHash[&prompt_session], prompt_provider);
}
