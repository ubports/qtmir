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

#include "windowmodel.h"

#include "mirsurface.h"

// mirserver
#include "nativeinterface.h"

// Qt
#include <QGuiApplication>
#include <QDebug>

using namespace qtmir;

WindowModel::WindowModel()
{
    auto nativeInterface = dynamic_cast<NativeInterface*>(QGuiApplication::platformNativeInterface());

    if (!nativeInterface) {
        qFatal("ERROR: Unity.Application QML plugin requires use of the 'mirserver' QPA plugin");
    }

    m_windowController = static_cast<WindowControllerInterface*>(nativeInterface->nativeResourceForIntegration("WindowController"));

    auto windowModel = static_cast<WindowModelNotifier*>(nativeInterface->nativeResourceForIntegration("WindowModelNotifier"));
    connectToWindowModelNotifier(windowModel);
}

WindowModel::WindowModel(WindowModelNotifier *notifier,
                         WindowControllerInterface *controller)
    : m_windowController(controller)
{
    connectToWindowModelNotifier(notifier);
}

void WindowModel::connectToWindowModelNotifier(WindowModelNotifier *notifier)
{
    connect(notifier, &WindowModelNotifier::windowAdded,        this, &WindowModel::onWindowAdded,        Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowRemoved,      this, &WindowModel::onWindowRemoved,      Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowsRaised,      this, &WindowModel::onWindowsRaised,      Qt::QueuedConnection);
}

QHash<int, QByteArray> WindowModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(SurfaceRole, "surface");
    return roleNames;
}

void WindowModel::onWindowAdded(const NewWindow &window)
{
    if (window.windowInfo.type() == mir_window_type_inputmethod) {
        addInputMethodWindow(window);
        return;
    }

    const int index = m_windowModel.count();
    beginInsertRows(QModelIndex(), index, index);
    m_windowModel.append(new MirSurface(window.windowInfo, m_windowController));
    endInsertRows();
    Q_EMIT countChanged();
}

void WindowModel::onWindowRemoved(const miral::WindowInfo &windowInfo)
{
    if (windowInfo.type() == mir_window_type_inputmethod) {
        removeInputMethodWindow();
        return;
    }

    const int index = findIndexOf(windowInfo.window());

    beginRemoveRows(QModelIndex(), index, index);
    m_windowModel.takeAt(index);
    endRemoveRows();
    Q_EMIT countChanged();
}

void WindowModel::addInputMethodWindow(const NewWindow &window)
{
    if (m_inputMethodSurface) {
        qDebug("Multiple Input Method Surfaces created, removing the old one!");
        delete m_inputMethodSurface;
    }
    m_inputMethodSurface = new MirSurface(window.windowInfo, m_windowController);
    Q_EMIT inputMethodSurfaceChanged(m_inputMethodSurface);
}

void WindowModel::removeInputMethodWindow()
{
    if (m_inputMethodSurface) {
        delete m_inputMethodSurface;
        m_inputMethodSurface = nullptr;
        Q_EMIT inputMethodSurfaceChanged(m_inputMethodSurface);
    }
}

void WindowModel::onWindowsRaised(const std::vector<miral::Window> &windows)
{
    // Reminder: last item in the "windows" list should end up at the top of the model
    const int modelCount = m_windowModel.count();
    const int raiseCount = windows.size();

    // Assumption: no NO-OPs are in this list - Qt will crash on endMoveRows() if you try NO-OPs!!!
    // A NO-OP is if
    //    1. "indices" is an empty list
    //    2. "indices" of the form (..., modelCount - 2, modelCount - 1) which results in an unchanged list

    // Precompute the list of indices of Windows/Surfaces to raise, including the offsets due to
    // indices which have already been moved.
    QVector<QPair<int /*from*/, int /*to*/>> moveList;

    for (int i=raiseCount-1; i>=0; i--) {
        int from = findIndexOf(windows[i]);
        const int to = modelCount - raiseCount + i;

        int moveCount = 0;
        // how many list items under "index" have been moved so far, correct "from" to suit
        for (int j=raiseCount-1; j>i; j--) {
            if (findIndexOf(windows[j]) < from) {
                moveCount++;
            }
        }
        from -= moveCount;

        if (from == to) {
            // is NO-OP, would result in moving element to itself
        } else {
            moveList.prepend({from, to});
        }
    }

    // Perform the moving, trusting the moveList is correct for each iteration.
    QModelIndex parent;
    for (int i=moveList.count()-1; i>=0; i--) {
        const int from = moveList[i].first;
        const int to = moveList[i].second;

        beginMoveRows(parent, from, from, parent, to+1);
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
        const auto &window = m_windowModel.takeAt(from);
        m_windowModel.insert(to, window);
#else
        m_windowModel.move(from, to);
#endif

        endMoveRows();
    }
}

int WindowModel::rowCount(const QModelIndex &/*parent*/) const
{
    return m_windowModel.count();
}

QVariant WindowModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_windowModel.count())
        return QVariant();

    if (role == SurfaceRole) {
        auto &surface = m_windowModel.at(index.row());
        return QVariant::fromValue(surface);
    } else {
        return QVariant();
    }
}

MirSurface *WindowModel::find(const miral::WindowInfo &needle) const
{
    auto window = needle.window();
    Q_FOREACH(const auto mirSurface, m_windowModel) {
        if (mirSurface->window() == window) {
            return mirSurface;
        }
    }
    return nullptr;
}

int WindowModel::findIndexOf(const miral::Window &needle) const
{
    for (int i=0; i<m_windowModel.count(); i++) {
        if (m_windowModel[i]->window() == needle) {
            return i;
        }
    }
    return -1;
}
