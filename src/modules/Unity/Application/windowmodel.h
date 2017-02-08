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

#ifndef WINDOWMODEL_H
#define WINDOWMODEL_H

#include <QAbstractListModel>

#include "mirsurface.h"
#include "qtmir/windowmodelnotifier.h"

namespace qtmir {

class WindowControllerInterface;

class WindowModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

    Q_PROPERTY(MirSurfaceInterface* inputMethodSurface READ inputMethodSurface NOTIFY inputMethodSurfaceChanged)

public:
    enum Roles {
        SurfaceRole = Qt::UserRole
    };

    WindowModel();
    explicit WindowModel(WindowModelNotifier *notifier,
                         WindowControllerInterface *controller); // For testing

    // QAbstractItemModel methods
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    int count() const { return rowCount(); }

    MirSurface* inputMethodSurface() const { return m_inputMethodSurface; }

Q_SIGNALS:
    void countChanged();
    void inputMethodSurfaceChanged(MirSurfaceInterface* inputMethodSurface);

private Q_SLOTS:
    void onWindowAdded(const qtmir::NewWindow &windowInfo);
    void onWindowRemoved(const miral::WindowInfo &windowInfo);
    void onWindowReady(const miral::WindowInfo &windowInfo);
    void onWindowMoved(const miral::WindowInfo &windowInfo, const QPoint topLeft);
    void onWindowStateChanged(const miral::WindowInfo &windowInfo, Mir::State state);
    void onWindowFocusChanged(const miral::WindowInfo &windowInfo, bool focused);
    void onWindowsRaised(const std::vector<miral::Window> &windows);

private:
    void connectToWindowModelNotifier(WindowModelNotifier *notifier);

    void addInputMethodWindow(const NewWindow &windowInfo);
    void removeInputMethodWindow();
    MirSurface* find(const miral::WindowInfo &needle) const;
    int findIndexOf(const miral::Window &needle) const;

    QVector<MirSurface*> m_windowModel;
    WindowControllerInterface *m_windowController;
    MirSurface* m_inputMethodSurface{nullptr};
};

} // namespace qtmir
#endif // WINDOWMODEL_H
