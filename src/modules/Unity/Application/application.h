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

#ifndef APPLICATION_H
#define APPLICATION_H

// std
#include <memory>

//Qt
#include <QtCore/QtCore>
#include <QImage>
#include <QSharedPointer>

// Unity API
#include <unity/shell/application/ApplicationInfoInterface.h>

#include "session_interface.h"

namespace mir {
    namespace scene {
        class Session;
    }
}

namespace qtmir
{

class ApplicationManager;
class DesktopFileReader;
class Session;
class SharedWakelock;

class Application : public unity::shell::application::ApplicationInfoInterface
{
    Q_OBJECT

    Q_PROPERTY(QString desktopFile READ desktopFile CONSTANT)
    Q_PROPERTY(QString exec READ exec CONSTANT)
    Q_PROPERTY(bool fullscreen READ fullscreen NOTIFY fullscreenChanged)
    Q_PROPERTY(Stage stage READ stage WRITE setStage NOTIFY stageChanged)
    Q_PROPERTY(SessionInterface* session READ session NOTIFY sessionChanged DESIGNABLE false)

public:
    Q_DECLARE_FLAGS(Stages, Stage)

    enum ProcessState {
        ProcessUnknown,
        ProcessRunning,
        ProcessSuspended,
        ProcessKilled, // it stopped, but because it was killed.
        ProcessStopped
    };

    enum class InternalState {
        Starting,
        Running,
        RunningInBackground,
        SuspendingWaitSession,
        SuspendingWaitProcess,
        Suspended,
        StoppedResumable,
        Stopped // It closed itself, crashed or it stopped and we can't respawn it
                // In any case, this is a dead end.
    };

    Application(const QSharedPointer<SharedWakelock>& sharedWakelock,
                DesktopFileReader *desktopFileReader,
                const QStringList &arguments,
                ApplicationManager *parent);
    virtual ~Application();

    // ApplicationInfoInterface
    QString appId() const override;
    QString name() const override;
    QString comment() const override;
    QUrl icon() const override;
    Stage stage() const override;
    State state() const override;
    RequestedState requestedState() const override;
    void setRequestedState(RequestedState) override;
    bool focused() const override;
    QString splashTitle() const override;
    QUrl splashImage() const override;
    bool splashShowHeader() const override;
    QColor splashColor() const override;
    QColor splashColorHeader() const override;
    QColor splashColorFooter() const override;
    Qt::ScreenOrientations supportedOrientations() const override;
    bool rotatesWindowContents() const override;

    void setStage(Stage stage);

    ProcessState processState() const { return m_processState; }
    void setProcessState(ProcessState value);

    QStringList arguments() const { return m_arguments; }

    SessionInterface* session() const;
    void setSession(SessionInterface *session);

    bool canBeResumed() const;

    bool isValid() const;
    QString desktopFile() const;
    QString exec() const;
    bool fullscreen() const;

    Stages supportedStages() const;

    pid_t pid() const;

    // for tests
    InternalState internalState() const { return m_state; }

    static QStringList lifecycleExceptions;

Q_SIGNALS:
    void fullscreenChanged(bool fullscreen);
    void stageChanged(Stage stage);
    void sessionChanged(SessionInterface *session);

    void startProcessRequested();
    void suspendProcessRequested();
    void resumeProcessRequested();
    void stopped();

private Q_SLOTS:
    void onSessionStateChanged(SessionInterface::State sessionState);

    void respawn();

private:

    QString longAppId() const;
    void acquireWakelock() const;
    void releaseWakelock() const;
    void setPid(pid_t pid);
    void setArguments(const QStringList arguments);
    void setFocused(bool focus);
    void setInternalState(InternalState state);
    void wipeQMLCache();
    void suspend();
    void resume();
    QColor colorFromString(const QString &colorString, const char *colorName) const;
    static const char* internalStateToStr(InternalState state);
    void applyRequestedState();
    void applyRequestedRunning();
    void applyRequestedSuspended();

    QSharedPointer<SharedWakelock> m_sharedWakelock;
    DesktopFileReader* m_desktopData;
    QString m_longAppId;
    qint64 m_pid;
    Stage m_stage;
    Stages m_supportedStages;
    InternalState m_state;
    bool m_focused;
    QStringList m_arguments;
    Qt::ScreenOrientations m_supportedOrientations;
    bool m_rotatesWindowContents;
    SessionInterface *m_session;
    RequestedState m_requestedState;
    ProcessState m_processState;

    friend class ApplicationManager;
    friend class SessionManager;
    friend class Session;
};

} // namespace qtmir

Q_DECLARE_METATYPE(qtmir::Application*)

#endif  // APPLICATION_H
