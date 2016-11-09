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

#include "toplevelwindowmodel.h"

#include "application.h"
#include "application_manager.h"
#include "mirsurface.h"
#include "sessionmanager.h"

// mirserver
#include "nativeinterface.h"

// Qt
#include <QGuiApplication>
#include <QDebug>

Q_LOGGING_CATEGORY(QTMIR_TOPLEVELWINDOWMODEL, "qtmir.toplevelwindowmodel", QtDebugMsg)

#define DEBUG_MSG qCDebug(QTMIR_TOPLEVELWINDOWMODEL).nospace().noquote() << __func__

using namespace qtmir;
namespace unityapi = unity::shell::application;

TopLevelWindowModel::TopLevelWindowModel()
{
    auto nativeInterface = dynamic_cast<NativeInterface*>(QGuiApplication::platformNativeInterface());

    if (!nativeInterface) {
        qFatal("ERROR: Unity.Application QML plugin requires use of the 'mirserver' QPA plugin");
    }

    m_windowController = static_cast<WindowControllerInterface*>(nativeInterface->nativeResourceForIntegration("WindowController"));

    auto windowModel = static_cast<WindowModelNotifier*>(nativeInterface->nativeResourceForIntegration("WindowModelNotifier"));
    connectToWindowModelNotifier(windowModel);

    setApplicationManager(ApplicationManager::singleton());

    m_sessionManager = SessionManager::singleton();
}

TopLevelWindowModel::TopLevelWindowModel(WindowModelNotifier *notifier,
                         WindowControllerInterface *controller)
    : m_windowController(controller)
{
    connectToWindowModelNotifier(notifier);
}

void TopLevelWindowModel::setApplicationManager(ApplicationManagerInterface* value)
{
    if (m_applicationManager == value) {
        return;
    }

    Q_ASSERT(m_modelState == IdleState);
    m_modelState = ResettingState;

    beginResetModel();

    if (m_applicationManager) {
        m_windowModel.clear();
        disconnect(m_applicationManager, 0, this, 0);
    }

    m_applicationManager = value;

    if (m_applicationManager) {
        connect(m_applicationManager, &QAbstractItemModel::rowsInserted,
                this, [this](const QModelIndex &/*parent*/, int first, int last) {
                    for (int i = first; i <= last; ++i) {
                        auto application = m_applicationManager->get(i);
                        addApplication(static_cast<Application*>(application));
                    }
                });

        connect(m_applicationManager, &QAbstractItemModel::rowsAboutToBeRemoved,
                this, [this](const QModelIndex &/*parent*/, int first, int last) {
                    for (int i = first; i <= last; ++i) {
                        auto application = m_applicationManager->get(i);
                        removeApplication(static_cast<Application*>(application));
                    }
                });

        for (int i = 0; i < m_applicationManager->rowCount(); ++i) {
            auto application = m_applicationManager->get(i);
            addApplication(static_cast<Application*>(application));
        }
    }

    endResetModel();
    m_modelState = IdleState;
}

void TopLevelWindowModel::addApplication(Application *application)
{
    DEBUG_MSG << "(" << application->appId() << ")";

    if (application->state() != unityapi::ApplicationInfoInterface::Stopped && application->surfaceList()->count() == 0) {
        prependPlaceholder(application);
    }
}

void TopLevelWindowModel::removeApplication(Application *application)
{
    DEBUG_MSG << "(" << application->appId() << ")";

    Q_ASSERT(m_modelState == IdleState);

    int i = 0;
    while (i < m_windowModel.count()) {
        if (m_windowModel.at(i).application == application) {
            removeAt(i);
        } else {
            ++i;
        }
    }
}

void TopLevelWindowModel::prependPlaceholder(Application *application)
{
    DEBUG_MSG << "(" << application->appId() << ")";

    prependSurfaceHelper(nullptr, application);
}

void TopLevelWindowModel::prependSurface(MirSurface *surface, Application *application)
{
    Q_ASSERT(surface != nullptr);

    bool filledPlaceholder = false;
    for (int i = 0; i < m_windowModel.count() && !filledPlaceholder; ++i) {
        ModelEntry &entry = m_windowModel[i];
        if (entry.application == application && entry.window->surface() == nullptr) {
            entry.window->setSurface(surface);
            connectSurface(surface);
            DEBUG_MSG << " appId=" << application->appId() << " surface=" << surface
                      << ", filling out placeholder. after: " << toString();
            filledPlaceholder = true;
        }
    }

    if (!filledPlaceholder) {
        DEBUG_MSG << " appId=" << application->appId() << " surface=" << surface << ", adding new row";
        prependSurfaceHelper(surface, application);
    }
}

void TopLevelWindowModel::prependSurfaceHelper(MirSurface *surface, Application *application)
{
    if (m_modelState == IdleState) {
        m_modelState = InsertingState;
        beginInsertRows(QModelIndex(), 0 /*first*/, 0 /*last*/);
    } else {
        Q_ASSERT(m_modelState == ResettingState);
        // No point in signaling anything if we're resetting the whole model
    }

    int id = generateId();
    Window *window = new Window(id);
    if (surface) {
        window->setSurface(surface);
    }
    m_windowModel.prepend(ModelEntry(window, application));
    if (surface) {
        connectSurface(surface);
    }

    connectWindow(window);

    if (m_modelState == InsertingState) {
        endInsertRows();
        Q_EMIT countChanged();
        Q_EMIT listChanged();
        m_modelState = IdleState;
    }

    if (!surface) {
        // focus the newly added window. miral can't help with that as it doesn't know about it.
        window->setFocused(true);
        if (m_focusedWindow && m_focusedWindow->surface()) {
            m_windowController->activate(miral::Window());
        }
    }

    DEBUG_MSG << " after " << toString();
}

void TopLevelWindowModel::connectWindow(Window *window)
{
    connect(window, &unityapi::WindowInterface::focusRequested, this, [this, window]() {
        if (!window->surface()) {
            // miral doesn't know about this window, so we have to do it ourselves
            window->setFocused(true);
            raiseId(window->id());
            Window *previousWindow = m_focusedWindow;
            setFocusedWindow(window);
            if (previousWindow && previousWindow->surface() && previousWindow->surface()->focused()) {
                m_windowController->activate(miral::Window());
            }
        }
    });

    connect(window, &unityapi::WindowInterface::focusedChanged, this, [this, window](bool focused) {
        if (window->surface()) {
            // Condense changes to the focused window
            // eg: Do focusedWindow=A to focusedWindow=B instead of
            // focusedWindow=A to focusedWindow=null to focusedWindow=B
            m_focusedWindowChanged = true;
            if (focused) {
                Q_ASSERT(m_newlyFocusedWindow == nullptr);
                m_newlyFocusedWindow = window;
            }
        }
    });

    connect(window, &Window::closeRequested, this, [this, window]() {
        if (!window->surface()) {
            int index = indexForId(window->id());
            Q_ASSERT(index >= 0);
            m_windowModel[index].application->close();
        }
    });
}

void TopLevelWindowModel::connectSurface(MirSurfaceInterface *surface)
{
    connect(surface, &MirSurfaceInterface::liveChanged, this, [this, surface](bool live){
            if (!live) {
                onSurfaceDied(surface);
            }
        });
    connect(surface, &QObject::destroyed, this, [this, surface](){ this->onSurfaceDestroyed(surface); });
}

void TopLevelWindowModel::onSurfaceDied(MirSurfaceInterface *surface)
{
    int i = indexOf(surface);
    if (i == -1) {
        return;
    }

    auto application = m_windowModel[i].application;

    // can't be starting if it already has a surface
    Q_ASSERT(application->state() != unityapi::ApplicationInfoInterface::Starting);

    if (application->state() == unityapi::ApplicationInfoInterface::Running) {
        m_windowModel[i].removeOnceSurfaceDestroyed = true;
    } else {
        // assume it got killed by the out-of-memory daemon.
        //
        // So leave entry in the model and only remove its surface, so shell can display a screenshot
        // in its place.
        m_windowModel[i].removeOnceSurfaceDestroyed = false;
    }
}

void TopLevelWindowModel::onSurfaceDestroyed(MirSurfaceInterface *surface)
{
    int i = indexOf(surface);
    if (i == -1) {
        return;
    }

    if (m_windowModel[i].removeOnceSurfaceDestroyed) {
        removeAt(i);
    } else {
        auto window = m_windowModel[i].window;
        window->setSurface(nullptr);
        window->setFocused(false);
        DEBUG_MSG << " Removed surface from entry. After: " << toString();
    }
}

void TopLevelWindowModel::connectToWindowModelNotifier(WindowModelNotifier *notifier)
{
    connect(notifier, &WindowModelNotifier::windowAdded,        this, &TopLevelWindowModel::onWindowAdded,        Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowRemoved,      this, &TopLevelWindowModel::onWindowRemoved,      Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowReady,        this, &TopLevelWindowModel::onWindowReady,        Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowMoved,        this, &TopLevelWindowModel::onWindowMoved,        Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowStateChanged, this, &TopLevelWindowModel::onWindowStateChanged, Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowFocusChanged, this, &TopLevelWindowModel::onWindowFocusChanged, Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowsRaised,      this, &TopLevelWindowModel::onWindowsRaised,      Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::modificationsStarted, this, &TopLevelWindowModel::onModificationsStarted, Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::modificationsEnded, this, &TopLevelWindowModel::onModificationsEnded, Qt::QueuedConnection);
}

void TopLevelWindowModel::onWindowAdded(const NewWindow &window)
{
    auto mirSession = window.windowInfo.window().application();
    SessionInterface* session = m_sessionManager->findSession(mirSession.get());

    auto surface = new MirSurface(window, m_windowController, session);
    rememberMirSurface(surface);

    if (session)
        session->registerSurface(surface);

    if (window.windowInfo.type() == mir_surface_type_inputmethod) {
        int id = generateId();
        Window *qmlWindow = new Window(id);
        connectWindow(qmlWindow);
        qmlWindow->setSurface(surface);
        setInputMethodWindow(qmlWindow);
    } else {
        Application *application = m_applicationManager->findApplicationWithSession(mirSession);
        if (application) {
            prependSurface(surface, application);
        } else {
            // Must be a prompt session. No need to do anything about it here as a prompt surface is not top-level.
            // It will show up in the qtmir::Application::promptSurfaceList of some application.
        }
    }
    // TODO: handle surfaces that are neither top-level windows nor input method. eg: child dialogs, popups, menus
}

void TopLevelWindowModel::onWindowRemoved(const miral::WindowInfo &windowInfo)
{
    forgetMirSurface(windowInfo.window());

    if (windowInfo.type() == mir_surface_type_inputmethod) {
        removeInputMethodWindow();
        return;
    }

    const int index = findIndexOf(windowInfo.window());
    if (index >= 0) {
        auto surface = static_cast<MirSurface*>(m_windowModel[index].window->surface());
        surface->setLive(false);
    }
}

void TopLevelWindowModel::rememberMirSurface(MirSurface *surface)
{
    m_allSurfaces.append(surface);
}

void TopLevelWindowModel::forgetMirSurface(const miral::Window &window)
{
    for (int i = 0; i < m_allSurfaces.count(); ++i) {
        if (m_allSurfaces[i]->window() == window) {
            m_allSurfaces.removeAt(i);
            return;
        }
    }
}

void TopLevelWindowModel::removeAt(int index)
{
    if (m_modelState == IdleState) {
        beginRemoveRows(QModelIndex(), index, index);
        m_modelState = RemovingState;
    } else {
        Q_ASSERT(m_modelState == ResettingState);
        // No point in signaling anything if we're resetting the whole model
    }

    auto window = m_windowModel[index].window;

    window->setSurface(nullptr);
    window->setFocused(false);

    m_windowModel.removeAt(index);

    if (m_modelState == RemovingState) {
        endRemoveRows();
        Q_EMIT countChanged();
        Q_EMIT listChanged();
        m_modelState = IdleState;
    }

    disconnect(window, 0, this, 0);
    if (m_focusedWindow == window) {
        setFocusedWindow(nullptr);
    }
    delete window;

    DEBUG_MSG << " after " << toString();
}

void TopLevelWindowModel::onWindowReady(const miral::WindowInfo &windowInfo)
{
    if (auto mirSurface = find(windowInfo)) {
        mirSurface->setReady();
    }
}

void TopLevelWindowModel::onWindowMoved(const miral::WindowInfo &windowInfo, const QPoint topLeft)
{
    if (auto mirSurface = find(windowInfo)) {
        mirSurface->setPosition(topLeft);
    }
}

void TopLevelWindowModel::onWindowFocusChanged(const miral::WindowInfo &windowInfo, bool focused)
{
    if (auto mirSurface = find(windowInfo)) {
        mirSurface->setFocused(focused);
    }
}

void TopLevelWindowModel::onWindowStateChanged(const miral::WindowInfo &windowInfo, Mir::State state)
{
    if (auto mirSurface = find(windowInfo)) {
        mirSurface->updateState(state);
    }
}

void TopLevelWindowModel::setInputMethodWindow(Window *window)
{
    if (m_inputMethodWindow) {
        qWarning("Multiple Input Method Surfaces created, removing the old one!");
        delete m_inputMethodWindow;
    }
    m_inputMethodWindow = window;
    Q_EMIT inputMethodSurfaceChanged(m_inputMethodWindow->surface());
}

void TopLevelWindowModel::removeInputMethodWindow()
{
    if (m_inputMethodWindow) {
        delete m_inputMethodWindow;
        m_inputMethodWindow = nullptr;
        Q_EMIT inputMethodSurfaceChanged(nullptr);
    }
}

void TopLevelWindowModel::onWindowsRaised(const std::vector<miral::Window> &windows)
{
    const int raiseCount = windows.size();
    for (int i = 0; i < raiseCount; i++) {
        int fromIndex = findIndexOf(windows[i]);
        if (fromIndex != -1) {
            move(fromIndex, 0);
        }
    }
}

int TopLevelWindowModel::rowCount(const QModelIndex &/*parent*/) const
{
    return m_windowModel.count();
}

QVariant TopLevelWindowModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= m_windowModel.size())
        return QVariant();

    if (role == WindowRole) {
        unityapi::WindowInterface *window = m_windowModel.at(index.row()).window;
        return QVariant::fromValue(window);
    } else if (role == ApplicationRole) {
        return QVariant::fromValue(m_windowModel.at(index.row()).application);
    } else {
        return QVariant();
    }
}

MirSurface *TopLevelWindowModel::find(const miral::WindowInfo &needle) const
{
    auto window = needle.window();
    Q_FOREACH(const auto surface, m_allSurfaces) {
        if (surface->window() == window) {
            return surface;
        }
    }
    return nullptr;
}

int TopLevelWindowModel::findIndexOf(const miral::Window &needle) const
{
    for (int i=0; i<m_windowModel.count(); i++) {
        auto surface = static_cast<MirSurface*>(m_windowModel[i].window->surface());
        if (surface && surface->window() == needle) {
            return i;
        }
    }
    return -1;
}

int TopLevelWindowModel::generateId()
{
    int id = m_nextId;
    m_nextId = nextFreeId(m_nextId + 1);
    return id;
}

int TopLevelWindowModel::nextFreeId(int candidateId)
{
    if (candidateId > m_maxId) {
        return nextFreeId(1);
    } else {
        if (indexForId(candidateId) == -1) {
            // it's indeed free
            return candidateId;
        } else {
            return nextFreeId(candidateId + 1);
        }
    }
}

QString TopLevelWindowModel::toString()
{
    QString str;
    for (int i = 0; i < m_windowModel.count(); ++i) {
        auto item = m_windowModel.at(i);

        QString itemStr = QString("(index=%1,appId=%2,surface=0x%3,id=%4)")
            .arg(i)
            .arg(item.application->appId())
            .arg((qintptr)item.window->surface(), 0, 16)
            .arg(item.window->id());

        if (i > 0) {
            str.append(",");
        }
        str.append(itemStr);
    }
    return str;
}

int TopLevelWindowModel::indexOf(MirSurfaceInterface *surface)
{
    for (int i = 0; i < m_windowModel.count(); ++i) {
        if (m_windowModel.at(i).window->surface() == surface) {
            return i;
        }
    }
    return -1;
}

int TopLevelWindowModel::indexForId(int id) const
{
    for (int i = 0; i < m_windowModel.count(); ++i) {
        if (m_windowModel[i].window->id() == id) {
            return i;
        }
    }
    return -1;
}

unityapi::WindowInterface *TopLevelWindowModel::windowAt(int index) const
{
    if (index >=0 && index < m_windowModel.count()) {
        return m_windowModel[index].window;
    } else {
        return nullptr;
    }
}

unityapi::MirSurfaceInterface *TopLevelWindowModel::surfaceAt(int index) const
{
    if (index >=0 && index < m_windowModel.count()) {
        return m_windowModel[index].window->surface();
    } else {
        return nullptr;
    }
}

unityapi::ApplicationInfoInterface *TopLevelWindowModel::applicationAt(int index) const
{
    if (index >=0 && index < m_windowModel.count()) {
        return m_windowModel[index].application;
    } else {
        return nullptr;
    }
}

int TopLevelWindowModel::idAt(int index) const
{
    if (index >=0 && index < m_windowModel.count()) {
        return m_windowModel[index].window->id();
    } else {
        return 0;
    }
}

void TopLevelWindowModel::raiseId(int id)
{
    if (m_modelState == IdleState) {
        DEBUG_MSG << "(id=" << id << ") - do it now.";
        doRaiseId(id);
    } else {
        DEBUG_MSG << "(id=" << id << ") - Model busy (modelState=" << m_modelState << "). Try again in the next event loop.";
        // The model has just signalled some change. If we have a Repeater responding to this update, it will get nuts
        // if we perform yet another model change straight away.
        //
        // A bad sympton of this problem is a Repeater.itemAt(index) call returning null event though Repeater.count says
        // the index is definitely within bounds.
        QMetaObject::invokeMethod(this, "raiseId", Qt::QueuedConnection, Q_ARG(int, id));
    }
}

void TopLevelWindowModel::doRaiseId(int id)
{
    int fromIndex = indexForId(id);
    if (fromIndex != -1) {
        auto surface = static_cast<MirSurface*>(m_windowModel[fromIndex].window->surface());
        if (surface) {
            m_windowController->raise(surface->window());
        } else {
            // move it ourselves. Since there's no mir::scene::Surface/miral::Window, there's nothing
            // miral can do about it.
            move(fromIndex, 0);
        }
    }
}

void TopLevelWindowModel::setFocusedWindow(Window *window)
{
    if (window != m_focusedWindow) {
        DEBUG_MSG << "(" << window << ")";

        Window* previousWindow = m_focusedWindow;

        m_focusedWindow = window;
        Q_EMIT focusedWindowChanged(m_focusedWindow);

        if (previousWindow && previousWindow->focused() && !previousWindow->surface()) {
            // do it ourselves. miral doesn't know about this window
            previousWindow->setFocused(false);
        }
    }
}

unityapi::MirSurfaceInterface* TopLevelWindowModel::inputMethodSurface() const
{
    return m_inputMethodWindow ? m_inputMethodWindow->surface() : nullptr;
}

unityapi::WindowInterface* TopLevelWindowModel::focusedWindow() const
{
    return m_focusedWindow;
}

void TopLevelWindowModel::move(int from, int to)
{
    if (from == to) return;
    DEBUG_MSG << " from=" << from << " to=" << to;

    if (from >= 0 && from < m_windowModel.size() && to >= 0 && to < m_windowModel.size()) {
        QModelIndex parent;
        /* When moving an item down, the destination index needs to be incremented
           by one, as explained in the documentation:
           http://qt-project.org/doc/qt-5.0/qtcore/qabstractitemmodel.html#beginMoveRows */

        Q_ASSERT(m_modelState == IdleState);
        m_modelState = MovingState;

        beginMoveRows(parent, from, from, parent, to + (to > from ? 1 : 0));
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
        const auto &window = m_windowModel.takeAt(from);
        m_windowModel.insert(to, window);
#else
        m_windowModel.move(from, to);
#endif
        endMoveRows();

        Q_EMIT listChanged();
        m_modelState = IdleState;

        DEBUG_MSG << " after " << toString();
    }
}
void TopLevelWindowModel::onModificationsStarted()
{
}

void TopLevelWindowModel::onModificationsEnded()
{
    if (m_focusedWindowChanged) {
        setFocusedWindow(m_newlyFocusedWindow);
    }
    // reset
    m_focusedWindowChanged = false;
    m_newlyFocusedWindow = nullptr;
}
