/*
 * Copyright (C) 2017 Canonical, Ltd.
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

#include "windowmodelnotifier.h"

#include <QMultiHash>
#include <QMutex>

using namespace qtmir;

namespace {

QMultiHash<const miral::Window, WindowNotifierObserver*> windowToObserverMap;
QMutex mutex;

} // anonymous namespace

WindowModelNotifier::WindowModelNotifier()
{
    connect(this, &WindowModelNotifier::windowAdded,                this, [](const qtmir::NewWindow &window) {
        WindowNotifierObserver::foreachObserverForWindow(window.windowInfo.window(), [](WindowNotifierObserver* observer) {
            Q_EMIT observer->surfaceCreated();
        });
    }, Qt::QueuedConnection);

    connect(this, &WindowModelNotifier::windowRemoved,              this, [](const miral::WindowInfo &windowInfo) {
        WindowNotifierObserver::foreachObserverForWindow(windowInfo.window(), [](WindowNotifierObserver* observer) {
            Q_EMIT observer->surfaceRemoved();
        });
    }, Qt::QueuedConnection);

    connect(this, &WindowModelNotifier::windowReady,                this, [](const miral::WindowInfo &windowInfo) {
        WindowNotifierObserver::foreachObserverForWindow(windowInfo.window(), [](WindowNotifierObserver* observer) {
            Q_EMIT observer->surfaceReady();
        });
    }, Qt::QueuedConnection);

    connect(this, &WindowModelNotifier::windowMoved,                this, [](const miral::WindowInfo &windowInfo, const QPoint &top_left) {
        WindowNotifierObserver::foreachObserverForWindow(windowInfo.window(), [top_left](WindowNotifierObserver* observer) {
            Q_EMIT observer->surfaceMoved(top_left);
        });
    }, Qt::QueuedConnection);

    connect(this, &WindowModelNotifier::windowResized,              this, [](const miral::WindowInfo &windowInfo, const QSize &size) {
        WindowNotifierObserver::foreachObserverForWindow(windowInfo.window(), [size](WindowNotifierObserver* observer) {
            Q_EMIT observer->surfaceResized(size);
        });
    }, Qt::QueuedConnection);

    connect(this, &WindowModelNotifier::windowStateChanged,         this, [](const miral::WindowInfo &windowInfo, Mir::State state) {
        WindowNotifierObserver::foreachObserverForWindow(windowInfo.window(), [state](WindowNotifierObserver* observer) {
            Q_EMIT observer->surfaceStateChanged(state);
        });
    }, Qt::QueuedConnection);

    connect(this, &WindowModelNotifier::windowFocusChanged,         this, [](const miral::WindowInfo &windowInfo, bool focused) {
        WindowNotifierObserver::foreachObserverForWindow(windowInfo.window(), [focused](WindowNotifierObserver* observer) {
            Q_EMIT observer->surfaceFocusChanged(focused);
        });
    }, Qt::QueuedConnection);

    connect(this, &WindowModelNotifier::windowRequestedRaise,       this, [this](const miral::WindowInfo &windowInfo) {
        WindowNotifierObserver::foreachObserverForWindow(windowInfo.window(), [](WindowNotifierObserver* observer) {
            Q_EMIT observer->surfaceRequestedRaise();
        });
    }, Qt::QueuedConnection);
}

WindowNotifierObserver::WindowNotifierObserver(const miral::Window &window)
{
    QMutexLocker locker(&mutex);
    windowToObserverMap.insert(window, this);
}

WindowNotifierObserver::~WindowNotifierObserver()
{
    QMutexLocker locker(&mutex);
    QMutableHashIterator<const miral::Window, WindowNotifierObserver*> i(windowToObserverMap);
    while (i.hasNext()) {
        i.next();
        if (i.value() == this) {
            i.remove();
            return;
        }
    }
}

void WindowNotifierObserver::foreachObserverForWindow(const miral::Window &window,
                                                      std::function<void(WindowNotifierObserver*)> fn)
{
    QMutexLocker locker(&mutex);
    auto observers = windowToObserverMap.values(window);
    Q_FOREACH(auto observer, observers) {
        fn(observer);
    }
}
