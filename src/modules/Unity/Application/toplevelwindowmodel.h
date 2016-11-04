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

#ifndef TOPLEVELWINDOWMODEL_H
#define TOPLEVELWINDOWMODEL_H

#include <QLoggingCategory>

#include "mirsurface.h"
#include "window.h"
#include "windowmodelnotifier.h"

// Unity API
#include <unity/shell/application/TopLevelWindowModelInterface.h>

Q_DECLARE_LOGGING_CATEGORY(QTMIR_TOPLEVELWINDOWMODEL)

namespace unity {
    namespace shell {
        namespace application {
            class ApplicationInfoInterface;
            class ApplicationManagerInterface;
            class MirSurfaceInterface;
        }
    }
}

namespace qtmir {

class Application;
class ApplicationManagerInterface;
class SessionManager;
class WindowControllerInterface;

class TopLevelWindowModel : public unity::shell::application::TopLevelWindowModelInterface
{
    Q_OBJECT

public:
    TopLevelWindowModel();
    explicit TopLevelWindowModel(WindowModelNotifier *notifier,
                         WindowControllerInterface *controller); // For testing

    // From unity::shell::application::TopLevelWindowModelInterface
    unity::shell::application::MirSurfaceInterface* inputMethodSurface() const override;
    unity::shell::application::WindowInterface* focusedWindow() const override;

    // From QAbstractItemModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    // Own API
    void setApplicationManager(ApplicationManagerInterface*);

public Q_SLOTS:
    // From unity::shell::application::TopLevelWindowModelInterface
    unity::shell::application::MirSurfaceInterface *surfaceAt(int index) const override;
    unity::shell::application::WindowInterface *windowAt(int index) const override;
    unity::shell::application::ApplicationInfoInterface *applicationAt(int index) const override;
    int idAt(int index) const override;
    int indexForId(int id) const override;
    void raiseId(int id) override;

private Q_SLOTS:
    void onWindowAdded(const qtmir::NewWindow &windowInfo);
    void onWindowRemoved(const miral::WindowInfo &windowInfo);
    void onWindowReady(const miral::WindowInfo &windowInfo);
    void onWindowMoved(const miral::WindowInfo &windowInfo, const QPoint topLeft);
    void onWindowStateChanged(const miral::WindowInfo &windowInfo, Mir::State state);
    void onWindowFocusChanged(const miral::WindowInfo &windowInfo, bool focused);
    void onWindowsRaised(const std::vector<miral::Window> &windows);
    void onModificationsStarted();
    void onModificationsEnded();

private:
    void doRaiseId(int id);
    int generateId();
    int nextFreeId(int candidateId);
    void connectToWindowModelNotifier(WindowModelNotifier *notifier);
    QString toString();
    int indexOf(MirSurfaceInterface *surface);

    void setInputMethodWindow(Window *window);
    void setFocusedWindow(Window *window);
    void removeInputMethodWindow();
    MirSurface* find(const miral::WindowInfo &needle) const;
    int findIndexOf(const miral::Window &needle) const;
    void removeAt(int index);

    void addApplication(Application *application);
    void removeApplication(Application *application);

    void prependPlaceholder(Application *application);
    void prependSurface(MirSurface *surface, Application *application);
    void prependSurfaceHelper(MirSurface *surface, Application *application);

    void connectWindow(Window *window);
    void connectSurface(MirSurfaceInterface *surface);

    void onSurfaceDied(MirSurfaceInterface *surface);
    void onSurfaceDestroyed(MirSurfaceInterface *surface);

    void move(int from, int to);

    void rememberMirSurface(MirSurface *surface);
    void forgetMirSurface(const miral::Window &window);

    struct ModelEntry {
        ModelEntry() {}
        ModelEntry(Window *window,
                   Application *application)
            : window(window), application(application) {}
        Window *window{nullptr};
        Application *application{nullptr};
        bool removeOnceSurfaceDestroyed{false};
    };

    QVector<ModelEntry> m_windowModel;
    WindowControllerInterface *m_windowController;
    SessionManager* m_sessionManager;
    Window* m_inputMethodWindow{nullptr};
    Window* m_focusedWindow{nullptr};
    QVector<MirSurface*> m_allSurfaces;

    int m_nextId{1};
    // Just something big enough that we don't risk running out of unused id numbers.
    // Not sure if QML int type supports something close to std::numeric_limits<int>::max() and
    // there's no reason to try out its limits.
    static const int m_maxId{1000000};

    ApplicationManagerInterface* m_applicationManager{nullptr};

    enum ModelState {
        IdleState,
        InsertingState,
        RemovingState,
        MovingState,
        ResettingState
    };
    ModelState m_modelState{IdleState};

    // Valid between modificationsStarted and modificationsEnded
    bool m_focusedWindowChanged{false};
    Window *m_newlyFocusedWindow{nullptr};
};

} // namespace qtmir
#endif // TOPLEVELWINDOWMODEL_H
