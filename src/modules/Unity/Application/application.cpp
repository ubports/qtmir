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
#include "application_manager.h"
#include "desktopfilereader.h"
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

QStringList Application::lifecycleExceptions;

Application::Application(const QSharedPointer<SharedWakelock>& sharedWakelock,
                         DesktopFileReader *desktopFileReader,
                         const QStringList &arguments,
                         ApplicationManager *parent)
    : ApplicationInfoInterface(desktopFileReader->appId(), parent)
    , m_sharedWakelock(sharedWakelock)
    , m_desktopData(desktopFileReader)
    , m_pid(0)
    , m_stage((m_desktopData->stageHint() == "SideStage") ? Application::SideStage : Application::MainStage)
    , m_state(InternalState::Starting)
    , m_focused(false)
    , m_arguments(arguments)
    , m_session(nullptr)
    , m_requestedState(RequestedRunning)
    , m_processState(ProcessUnknown)
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::Application - appId=" << desktopFileReader->appId();

    // Because m_state is InternalState::Starting
    acquireWakelock();

    // FIXME(greyback) need to save long appId internally until ubuntu-app-launch can hide it from us
    m_longAppId = desktopFileReader->file().remove(QRegExp(".desktop$")).split('/').last();

    m_supportedOrientations = m_desktopData->supportedOrientations();

    m_rotatesWindowContents = m_desktopData->rotatesWindowContents();
}

Application::~Application()
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::~Application";

    // (ricmm) -- To be on the safe side, better wipe the application QML compile cache if it crashes on startup
    if (m_processState == Application::ProcessUnknown
            || state() == Application::Starting
            || state() == Application::Running) {
        wipeQMLCache();
    }

    if (m_session) {
        m_session->setApplication(nullptr);
        delete m_session;
    }
    delete m_desktopData;
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
    return m_desktopData->loaded();
}

QString Application::desktopFile() const
{
    return m_desktopData->file();
}

QString Application::appId() const
{
    return m_desktopData->appId();
}

QString Application::name() const
{
    return m_desktopData->name();
}

QString Application::comment() const
{
    return m_desktopData->comment();
}

QUrl Application::icon() const
{
    QString iconString = m_desktopData->icon();
    QString pathString = m_desktopData->path();

    if (QFileInfo(iconString).exists()) {
        return QUrl(iconString);
    } else if (QFileInfo(pathString + '/' + iconString).exists()) {
        return QUrl(pathString + '/' + iconString);
    } else {
        return QUrl("image://theme/" + iconString);
    }
}

QString Application::splashTitle() const
{
    return m_desktopData->splashTitle();
}

QUrl Application::splashImage() const
{
    if (m_desktopData->splashImage().isEmpty()) {
        return QUrl();
    } else {
        QFileInfo imageFileInfo(m_desktopData->path(), m_desktopData->splashImage());
        if (imageFileInfo.exists()) {
            return QUrl::fromLocalFile(imageFileInfo.canonicalFilePath());
        } else {
            qCWarning(QTMIR_APPLICATIONS)
                << QString("Application(%1).splashImage file does not exist: \"%2\". Ignoring it.")
                    .arg(appId()).arg(imageFileInfo.absoluteFilePath());

            return QUrl();
        }
    }
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
    case InternalState::StoppedUnexpectedly:
        return "StoppedUnexpectedly";
    case InternalState::Stopped:
        return "Stopped";
    default:
        return "???";
    }
}

bool Application::splashShowHeader() const
{
    QString showHeader = m_desktopData->splashShowHeader();
    if (showHeader.toLower() == "true") {
        return true;
    } else {
        return false;
    }
}

QColor Application::splashColor() const
{
    QString colorStr = m_desktopData->splashColor();
    return colorFromString(colorStr, "splashColor");
}

QColor Application::splashColorHeader() const
{
    QString colorStr = m_desktopData->splashColorHeader();
    return colorFromString(colorStr, "splashColorHeader");
}

QColor Application::splashColorFooter() const
{
    QString colorStr = m_desktopData->splashColorFooter();
    return colorFromString(colorStr, "splashColorFooter");
}

QString Application::exec() const
{
    return m_desktopData->exec();
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
    case InternalState::StoppedUnexpectedly:
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
    case InternalState::StoppedUnexpectedly:
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
        Q_ASSERT(m_state == InternalState::Stopped || m_state == InternalState::StoppedUnexpectedly);
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
            releaseWakelock();
            break;
        case InternalState::Suspended:
            releaseWakelock();
            break;
        case InternalState::StoppedUnexpectedly:
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
        if (m_state == InternalState::StoppedUnexpectedly) {
            setInternalState(InternalState::Starting);
        }
        break;
    case ProcessSuspended:
        Q_ASSERT(m_state == InternalState::SuspendingWaitProcess);
        setInternalState(InternalState::Suspended);
        break;
    case ProcessKilled:
        // we assume the session always stop before the process
        Q_ASSERT(!m_session || m_session->state() == Session::Stopped);

        if (m_state == InternalState::Starting) {
            // that was way too soon. let it go away
            setInternalState(InternalState::Stopped);
        } else {
            Q_ASSERT(m_state == InternalState::Stopped
                    || m_state == InternalState::StoppedUnexpectedly);
        }
        break;
    case ProcessStopped:
        // we assume the session always stop before the process
        Q_ASSERT(!m_session || m_session->state() == Session::Stopped);

        if (m_state == InternalState::Starting) {
            // that was way too soon. let it go away
            setInternalState(InternalState::Stopped);
        } else if (m_state == InternalState::StoppedUnexpectedly) {
            // the application stopped nicely, likely closed itself. Thus not really unexpected after all.
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
    Q_ASSERT(m_state == InternalState::Running);
    Q_ASSERT(m_session != nullptr);

    if (!lifecycleExceptions.filter(appId().section('_',0,0)).empty()) {
        // Present in exceptions list.
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

QString Application::longAppId() const
{
    return m_longAppId;
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
                || m_state == InternalState::Running) {
            /*  1. application is not managed by upstart
             *  2. application is managed by upstart, but has stopped before it managed
             *     to create a surface, we can assume it crashed on startup, and thus
             *     cannot be resumed
             *  3. application is managed by upstart and is in foreground (i.e. has
             *     Running state), if Mir reports the application disconnects, it
             *     either crashed or stopped itself.
             */
            setInternalState(InternalState::Stopped);
        } else {
            setInternalState(InternalState::StoppedUnexpectedly);
        }
    }
}

} // namespace qtmir
