/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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

// local
#include "application.h"
#include "applicationinfo.h"
#include "application_manager.h"
#include "session.h"
#include "sharedwakelock.h"
#include "taskcontroller.h"

// common
#include <debughelpers.h>

// QPA mirserver
#include "logging.h"

// mir
#include <mir/scene/session.h>
#include <mir/scene/snapshot.h>

namespace ms = mir::scene;

namespace qtmir
{

Application::Application(const QSharedPointer<SharedWakelock>& sharedWakelock,
                         const std::shared_ptr<ApplicationInfo>& appInfo,
                         const QStringList &arguments,
                         ApplicationManager *parent)
    : ApplicationInfoInterface(appInfo->appId(), parent)
    , m_sharedWakelock(sharedWakelock)
    , m_appInfo(appInfo)
    , m_pid(0)
    , m_stage(Application::MainStage)
    , m_state(InternalState::Starting)
    , m_focused(false)
    , m_arguments(arguments)
    , m_session(nullptr)
    , m_requestedState(RequestedRunning)
    , m_processState(ProcessUnknown)
    , m_closeTimer(0)
    , m_exemptFromLifecycle(false)
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::Application - appId=" << appInfo->appId();

    // Because m_state is InternalState::Starting
    acquireWakelock();

    m_supportedOrientations = m_appInfo->supportedOrientations();

    m_rotatesWindowContents = m_appInfo->rotatesWindowContents();
}

Application::~Application()
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::~Application";

    // (ricmm) -- To be on the safe side, better wipe the application QML compile cache if it crashes on startup
    if (m_processState == Application::ProcessUnknown) {
        wipeQMLCache();
    }

    switch (m_state) {
    case InternalState::Starting:
    case InternalState::Running:
    case InternalState::RunningInBackground:
    case InternalState::SuspendingWaitSession:
    case InternalState::SuspendingWaitProcess:
        wipeQMLCache();
        break;
    case InternalState::Closing:
    case InternalState::Suspended:
    case InternalState::StoppedResumable:
        break;
    case InternalState::Stopped:
        if (m_processState == Application::ProcessFailed) { // process crashed
            wipeQMLCache();
        }
        break;
    }

    if (m_session) {
        m_session->setApplication(nullptr);
        delete m_session;
    }
}


void Application::wipeQMLCache()
{
    QString path(QDir::homePath() + QStringLiteral("/.cache/QML/Apps/"));
    QDir dir(path);
    QStringList apps = dir.entryList();
    for (int i = 0; i < apps.size(); i++) {
        if (apps.at(i).contains(appId())) {
            qCDebug(QTMIR_APPLICATIONS) << "Application appId=" << apps.at(i) << " Wiping QML Cache";
            dir.cd(apps.at(i));
            dir.removeRecursively();
            break;
        }
    }
}

bool Application::isValid() const
{
    return !appId().isEmpty();
}

QString Application::appId() const
{
    return m_appInfo->appId();
}

QString Application::name() const
{
    return m_appInfo->name();
}

QString Application::comment() const
{
    return m_appInfo->comment();
}

QUrl Application::icon() const
{
    return m_appInfo->icon();
}

QString Application::splashTitle() const
{
    return m_appInfo->splashTitle();
}

QUrl Application::splashImage() const
{
    return m_appInfo->splashImage();
}

QColor Application::colorFromString(const QString &colorString, const char *colorName) const
{
    // NB: If a colour which is not fully opaque is specified in the desktop file, it will
    //     be ignored and the default colour will be used instead.
    QColor color;

    if (colorString.isEmpty()) {
        color.setRgba(qRgba(0, 0, 0, 0));
    } else {
        color.setNamedColor(colorString);

        if (color.isValid()) {
            // Force a fully opaque color.
            color.setAlpha(255);
        } else {
            color.setRgba(qRgba(0, 0, 0, 0));
            qCWarning(QTMIR_APPLICATIONS) << QString("Invalid %1: \"%2\"")
                .arg(colorName).arg(colorString);
        }
    }

    return color;
}

const char* Application::internalStateToStr(InternalState state)
{
    switch (state) {
    case InternalState::Starting:
        return "Starting";
    case InternalState::Running:
        return "Running";
    case InternalState::RunningInBackground:
        return "RunningInBackground";
    case InternalState::SuspendingWaitSession:
        return "SuspendingWaitSession";
    case InternalState::SuspendingWaitProcess:
        return "SuspendingWaitProcess";
    case InternalState::Suspended:
        return "Suspended";
    case InternalState::Closing:
        return "Closing";
    case InternalState::StoppedResumable:
        return "StoppedResumable";
    case InternalState::Stopped:
        return "Stopped";
    default:
        return "???";
    }
}

bool Application::splashShowHeader() const
{
    return m_appInfo->splashShowHeader();
}

QColor Application::splashColor() const
{
    QString colorStr = m_appInfo->splashColor();
    return colorFromString(colorStr, "splashColor");
}

QColor Application::splashColorHeader() const
{
    QString colorStr = m_appInfo->splashColorHeader();
    return colorFromString(colorStr, "splashColorHeader");
}

QColor Application::splashColorFooter() const
{
    QString colorStr = m_appInfo->splashColorFooter();
    return colorFromString(colorStr, "splashColorFooter");
}

Application::Stage Application::stage() const
{
    return m_stage;
}

Application::Stages Application::supportedStages() const
{
    return m_supportedStages;
}

Application::State Application::state() const
{
    // The public state is a simplified version of the internal one as our consumers
    // don't have to know or care about all the nasty details.
    switch (m_state) {
    case InternalState::Starting:
        return Starting;
    case InternalState::Running:
    case InternalState::RunningInBackground:
    case InternalState::SuspendingWaitSession:
    case InternalState::SuspendingWaitProcess:
    case InternalState::Closing:
        return Running;
    case InternalState::Suspended:
        return Suspended;
    case InternalState::Stopped:
    default:
        return Stopped;
    }
}

Application::RequestedState Application::requestedState() const
{
    return m_requestedState;
}

void Application::setRequestedState(RequestedState value)
{
    if (m_requestedState == value) {
        // nothing to do
        return;
    }

    qCDebug(QTMIR_APPLICATIONS) << "Application::setRequestedState - appId=" << appId()
                                << "requestedState=" << applicationStateToStr(value);
    m_requestedState = value;
    Q_EMIT requestedStateChanged(m_requestedState);

    applyRequestedState();
}

void Application::applyRequestedState()
{
    if (m_requestedState == RequestedRunning) {
        applyRequestedRunning();
    } else {
        applyRequestedSuspended();
    }
}

void Application::applyRequestedRunning()
{
    switch (m_state) {
    case InternalState::Starting:
        // should leave the app alone until it reaches Running state
        break;
    case InternalState::Running:
        // already where it's wanted to be
        break;
    case InternalState::RunningInBackground:
    case InternalState::SuspendingWaitSession:
    case InternalState::Suspended:
        resume();
        break;
    case InternalState::SuspendingWaitProcess:
        // should leave the app alone until it reaches Suspended state
        break;
    case InternalState::Closing:
        break;
    case InternalState::StoppedResumable:
        respawn();
        break;
    case InternalState::Stopped:
        // dead end.
        break;
    }
}

void Application::applyRequestedSuspended()
{
    switch (m_state) {
    case InternalState::Starting:
        // should leave the app alone until it reaches Running state
        break;
    case InternalState::Running:
        if (m_processState == ProcessRunning) {
            suspend();
        } else {
            // we can't suspend it since we have no information on the app process
            Q_ASSERT(m_processState == ProcessUnknown);
        }
        break;
    case InternalState::RunningInBackground:
    case InternalState::SuspendingWaitSession:
    case InternalState::SuspendingWaitProcess:
    case InternalState::Suspended:
        // it's already going where we it's wanted
        break;
    case InternalState::Closing:
        // don't suspend while it is closing
        break;
    case InternalState::StoppedResumable:
    case InternalState::Stopped:
        // the app doesn't have a process in the first place, so there's nothing to suspend
        break;
    }
}

bool Application::focused() const
{
    return m_focused;
}

bool Application::fullscreen() const
{
    return m_session ? m_session->fullscreen() : false;
}

bool Application::canBeResumed() const
{
    return m_processState != ProcessUnknown;
}

pid_t Application::pid() const
{
    return m_pid;
}

void Application::close()
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::close - appId=" << appId();

    switch (m_state) {
    case InternalState::Starting:
        stop();
        break;
    case InternalState::Running:
        doClose();
        break;
    case InternalState::RunningInBackground:
    case InternalState::SuspendingWaitSession:
    case InternalState::SuspendingWaitProcess:
    case InternalState::Suspended:
        setRequestedState(RequestedRunning);
        doClose();
        break;
    case InternalState::Closing:
        // already on the way
        break;
    case InternalState::StoppedResumable:
    case InternalState::Stopped:
        // too late
        break;
    }

}

void Application::doClose()
{
    Q_ASSERT(m_closeTimer == 0);
    Q_ASSERT(m_session != nullptr);

    m_session->close();
    m_closeTimer = startTimer(3000);
    setInternalState(InternalState::Closing);
}

void Application::setPid(pid_t pid)
{
    m_pid = pid;
}

void Application::setArguments(const QStringList arguments)
{
    m_arguments = arguments;
}

void Application::setSession(SessionInterface *newSession)
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::setSession - appId=" << appId() << "session=" << newSession;

    if (newSession == m_session)
        return;

    if (m_session) {
        m_session->disconnect(this);
        m_session->setApplication(nullptr);
        m_session->setParent(nullptr);
    }

    bool oldFullscreen = fullscreen();
    m_session = newSession;

    if (m_session) {
        m_session->setParent(this);
        m_session->setApplication(this);

        switch (m_state) {
        case InternalState::Starting:
        case InternalState::Running:
        case InternalState::RunningInBackground:
        case InternalState::Closing:
            m_session->resume();
            break;
        case InternalState::SuspendingWaitSession:
        case InternalState::SuspendingWaitProcess:
        case InternalState::Suspended:
            m_session->suspend();
            break;
        case InternalState::Stopped:
        default:
            m_session->stop();
            break;
        }

        connect(m_session, &SessionInterface::stateChanged, this, &Application::onSessionStateChanged);
        connect(m_session, &SessionInterface::fullscreenChanged, this, &Application::fullscreenChanged);

        if (oldFullscreen != fullscreen())
            Q_EMIT fullscreenChanged(fullscreen());
    } else {
        // this can only happen after the session has stopped and QML code called Session::release()
        Q_ASSERT(m_state == InternalState::Stopped || m_state == InternalState::StoppedResumable);
    }

    Q_EMIT sessionChanged(m_session);
}

void Application::setStage(Application::Stage stage)
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::setStage - appId=" << appId() << "stage=" << stage;

    if (m_stage != stage) {
        if (stage | m_supportedStages) {
            return;
        }

        m_stage = stage;
        Q_EMIT stageChanged(stage);
    }
}

void Application::setInternalState(Application::InternalState state)
{
    if (m_state == state) {
        return;
    }

    qCDebug(QTMIR_APPLICATIONS) << "Application::setInternalState - appId=" << appId()
        << "state=" << internalStateToStr(state);

    auto oldPublicState = this->state();
    m_state = state;

    switch (m_state) {
        case InternalState::Starting:
        case InternalState::Running:
            acquireWakelock();
            break;
        case InternalState::RunningInBackground:
        case InternalState::Suspended:
            releaseWakelock();
            break;
        case InternalState::Closing:
            acquireWakelock();
            break;
        case InternalState::StoppedResumable:
            releaseWakelock();
            break;
        case InternalState::Stopped:
            Q_EMIT stopped();
            releaseWakelock();
            break;
        case InternalState::SuspendingWaitSession:
        case InternalState::SuspendingWaitProcess:
            // transitory states. leave as it is
        default:
            break;
    };

    if (this->state() != oldPublicState) {
        Q_EMIT stateChanged(this->state());
    }

    applyRequestedState();
}

void Application::setFocused(bool focused)
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::setFocused - appId=" << appId() << "focused=" << focused;

    if (m_focused != focused) {
        m_focused = focused;
        Q_EMIT focusedChanged(focused);
    }
}

void Application::setProcessState(ProcessState newProcessState)
{
    if (m_processState == newProcessState) {
        return;
    }

    m_processState = newProcessState;

    switch (m_processState) {
    case ProcessUnknown:
        // it would be a coding error
        Q_ASSERT(false);
        break;
    case ProcessRunning:
        if (m_state == InternalState::StoppedResumable) {
            setInternalState(InternalState::Starting);
        }
        break;
    case ProcessSuspended:
        Q_ASSERT(m_state == InternalState::SuspendingWaitProcess);
        setInternalState(InternalState::Suspended);
        break;
    case ProcessFailed:
        // we assume the session always stop before the process
        Q_ASSERT(!m_session || m_session->state() == Session::Stopped);

        if (m_state == InternalState::Starting) {
            // that was way too soon. let it go away
            setInternalState(InternalState::Stopped);
        } else {
            Q_ASSERT(m_state == InternalState::Stopped
                    || m_state == InternalState::StoppedResumable);
        }
        break;
    case ProcessStopped:
        // we assume the session always stop before the process
        Q_ASSERT(!m_session || m_session->state() == Session::Stopped);

        if (m_state == InternalState::Starting) {
            // that was way too soon. let it go away
            setInternalState(InternalState::Stopped);
        } else if (m_state == InternalState::StoppedResumable ||
                   m_state == InternalState::Closing) {
            // The application stopped nicely, likely closed itself. Thus not meant to be resumed later.
            setInternalState(InternalState::Stopped);
        } else {
            Q_ASSERT(m_state == InternalState::Stopped);
        }
        break;
    }

    applyRequestedState();
}

void Application::suspend()
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::suspend - appId=" << appId();

    Q_ASSERT(m_state == InternalState::Running);
    Q_ASSERT(m_session != nullptr);

    if (exemptFromLifecycle()) {
        // There's no need to keep the wakelock as the process is never suspended
        // and thus has no cleanup to perform when (for example) the display is
        // blanked.
        setInternalState(InternalState::RunningInBackground);
    } else {
        setInternalState(InternalState::SuspendingWaitSession);
        m_session->suspend();
    }
}

void Application::resume()
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::resume - appId=" << appId();

    if (m_state == InternalState::Suspended) {
        setInternalState(InternalState::Running);
        Q_EMIT resumeProcessRequested();
        if (m_processState == ProcessSuspended) {
            setProcessState(ProcessRunning); // should we wait for a resumed() signal?
        }
        m_session->resume();
    } else if (m_state == InternalState::SuspendingWaitSession) {
        setInternalState(InternalState::Running);
        m_session->resume();
    } else if (m_state == InternalState::RunningInBackground) {
        setInternalState(InternalState::Running);
    }
}

void Application::respawn()
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::respawn - appId=" << appId();

    setInternalState(InternalState::Starting);

    Q_EMIT startProcessRequested();
}

void Application::stop()
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::stop - appId=" << appId();

    Q_EMIT stopProcessRequested();
}

void Application::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_closeTimer) {
        m_closeTimer = 0;
        stop();
    }
}

bool Application::isTouchApp() const
{
    return m_appInfo->isTouchApp();
}

bool Application::exemptFromLifecycle() const
{
    return m_exemptFromLifecycle;
}

void Application::setExemptFromLifecycle(bool exemptFromLifecycle)
{
    if (m_exemptFromLifecycle != exemptFromLifecycle)
    {
        // We don't adjust current suspension state, we only care about exempt
        // status going into a suspend.
        m_exemptFromLifecycle = exemptFromLifecycle;
        Q_EMIT exemptFromLifecycleChanged(m_exemptFromLifecycle);
    }
}

Qt::ScreenOrientations Application::supportedOrientations() const
{
    return m_supportedOrientations;
}

bool Application::rotatesWindowContents() const
{
    return m_rotatesWindowContents;
}

SessionInterface* Application::session() const
{
    return m_session;
}

void Application::acquireWakelock() const
{
    if (appId() == "unity8-dash")
        return;

    m_sharedWakelock->acquire(this);
}

void Application::releaseWakelock() const
{
    if (appId() == "unity8-dash")
        return;

    m_sharedWakelock->release(this);
}

void Application::onSessionStateChanged(Session::State sessionState)
{
    switch (sessionState) {
    case Session::Starting:
        break;
    case Session::Running:
        if (m_state == InternalState::Starting) {
            setInternalState(InternalState::Running);
        }
        break;
    case Session::Suspending:
        break;
    case Session::Suspended:
        Q_ASSERT(m_state == InternalState::SuspendingWaitSession);
        setInternalState(InternalState::SuspendingWaitProcess);
        Q_EMIT suspendProcessRequested();
        break;
    case Session::Stopped:
        if (!canBeResumed()
                || m_state == InternalState::Starting
                || m_state == InternalState::Running
                || m_state == InternalState::Closing) {
            /*  1. application is not managed by upstart
             *  2. application is managed by upstart, but has stopped before it managed
             *     to create a surface, we can assume it crashed on startup, and thus
             *     cannot be resumed
             *  3. application is managed by upstart and is in foreground (i.e. has
             *     Running state), if Mir reports the application disconnects, it
             *     either crashed or stopped itself.
             *  4. We're expecting the application to stop after a close request
             */
            setInternalState(InternalState::Stopped);
        } else {
            setInternalState(InternalState::StoppedResumable);
        }
    }
}

} // namespace qtmir
