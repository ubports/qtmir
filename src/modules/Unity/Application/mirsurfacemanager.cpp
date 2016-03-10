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

#include "mirsurfacemanager.h"

// Qt
#include <QGuiApplication>
#include <QMutexLocker>

// local
#include "mirsurface.h"
#include "sessionmanager.h"
#include "application_manager.h"
#include "tracepoints.h" // generated from tracepoints.tp

// common
#include <debughelpers.h>

// QPA mirserver
#include "nativeinterface.h"
#include "mirserver.h"
#include "sessionlistener.h"
#include "logging.h"
#include "sizehints.h"

Q_LOGGING_CATEGORY(QTMIR_SURFACES, "qtmir.surfaces")

namespace ms = mir::scene;

namespace qtmir {

MirSurfaceManager *MirSurfaceManager::instance = nullptr;


void connectToSessionListener(MirSurfaceManager *manager, SessionListener *listener)
{
    QObject::connect(listener, &SessionListener::sessionCreatedSurface,
                     manager, &MirSurfaceManager::onSessionCreatedSurface);
    QObject::connect(listener, &SessionListener::sessionDestroyingSurface,
                     manager, &MirSurfaceManager::onSessionDestroyingSurface);
}

MirSurfaceManager* MirSurfaceManager::singleton()
{
    if (!instance) {

        NativeInterface *nativeInterface = dynamic_cast<NativeInterface*>(QGuiApplication::platformNativeInterface());

        if (!nativeInterface) {
            qCritical("ERROR: Unity.Application QML plugin requires use of the 'mirserver' QPA plugin");
            QGuiApplication::quit();
            return nullptr;
        }

        SessionListener *sessionListener = static_cast<SessionListener*>(nativeInterface->nativeResourceForIntegration("SessionListener"));
        MirShell *shell = static_cast<MirShell*>(nativeInterface->nativeResourceForIntegration("Shell"));

        instance = new MirSurfaceManager(nativeInterface->m_mirServer, shell, SessionManager::singleton());

        connectToSessionListener(instance, sessionListener);
    }
    return instance;
}

MirSurfaceManager::MirSurfaceManager(
        const QSharedPointer<MirServer>& mirServer,
        MirShell *shell,
        SessionManager* sessionManager,
        QObject *parent)
    : QObject(parent)
    , m_mirServer(mirServer)
    , m_shell(shell)
    , m_sessionManager(sessionManager)
{
    qCDebug(QTMIR_SURFACES) << "MirSurfaceManager::MirSurfaceManager - this=" << this;
    setObjectName("qtmir::SurfaceManager");
}

MirSurfaceManager::~MirSurfaceManager()
{
    qCDebug(QTMIR_SURFACES) << "MirSurfaceManager::~MirSurfaceManager - this=" << this;

    m_mirSurfaceToQmlSurfaceHash.clear();
}

void MirSurfaceManager::onSessionCreatedSurface(const mir::scene::Session *mirSession,
                                                const std::shared_ptr<mir::scene::Surface> &surface,
                                                const std::shared_ptr<SurfaceObserver> &observer,
                                                qtmir::SizeHints sizeHints)
{
    qCDebug(QTMIR_SURFACES) << "MirSurfaceManager::onSessionCreatedSurface - mirSession=" << mirSession
                            << "surface=" << surface.get() << "surface.name=" << surface->name().c_str();

    SessionInterface* session = m_sessionManager->findSession(mirSession);
    auto qmlSurface = new MirSurface(surface, session, m_shell, observer, sizeHints);
    {
        QMutexLocker lock(&m_mutex);
        m_mirSurfaceToQmlSurfaceHash.insert(surface.get(), qmlSurface);
    }

    if (session)
        session->registerSurface(qmlSurface);

    if (qmlSurface->type() == Mir::InputMethodType) {
        m_inputMethodSurface = qmlSurface;
        Q_EMIT inputMethodSurfaceChanged();
    }

    // Only notify QML of surface creation once it has drawn its first frame.
    connect(qmlSurface, &MirSurfaceInterface::firstFrameDrawn, this, [=]() {
        tracepoint(qtmir, firstFrameDrawn);
        Q_EMIT surfaceCreated(qmlSurface);
    });

    // clean up after MirSurface is destroyed
    connect(qmlSurface, &QObject::destroyed, this, [&](QObject *obj) {
        auto qmlSurface = static_cast<MirSurfaceInterface*>(obj);
        {
            QMutexLocker lock(&m_mutex);
            m_mirSurfaceToQmlSurfaceHash.remove(m_mirSurfaceToQmlSurfaceHash.key(qmlSurface));
        }

        tracepoint(qtmir, surfaceDestroyed);
    });
    tracepoint(qtmir, surfaceCreated);
}

void MirSurfaceManager::onSessionDestroyingSurface(const mir::scene::Session *session,
                                                   const std::shared_ptr<mir::scene::Surface> &surface)
{
    qCDebug(QTMIR_SURFACES) << "MirSurfaceManager::onSessionDestroyingSurface - session=" << session
                            << "surface=" << surface.get() << "surface.name=" << surface->name().c_str();

    MirSurfaceInterface* qmlSurface = nullptr;
    {
        QMutexLocker lock(&m_mutex);
        auto it = m_mirSurfaceToQmlSurfaceHash.find(surface.get());
        if (it != m_mirSurfaceToQmlSurfaceHash.end()) {

            qmlSurface = it.value();

            m_mirSurfaceToQmlSurfaceHash.erase(it);
        } else {
            qCritical() << "MirSurfaceManager::onSessionDestroyingSurface: unable to find MirSurface corresponding"
                        << "to surface=" << surface.get() << "surface.name=" << surface->name().c_str();
            return;
        }
    }

    if (qmlSurface->type() == Mir::InputMethodType) {
        m_inputMethodSurface = nullptr;
        Q_EMIT inputMethodSurfaceChanged();
    }

    qmlSurface->setLive(false);
    Q_EMIT surfaceDestroyed(qmlSurface);
}

MirSurfaceInterface* MirSurfaceManager::inputMethodSurface() const
{
    return m_inputMethodSurface;
}

} // namespace qtmir
