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

#include "windowcontroller.h"

#include "windowmanagementpolicy.h"
#include "mirqtconversion.h"

using namespace qtmir;


WindowController::WindowController()
    : m_policy(nullptr)
{
}

void WindowController::activate(const miral::Window &window)
{
    if (m_policy) {
        m_policy->activate(window);
    }
}

void WindowController::raise(const miral::Window &window)
{
    if (m_policy) {
        m_policy->raise(window);
    }
}

void WindowController::resize(const miral::Window &window, const QSize &size)
{
    if (m_policy) {
        m_policy->resize(window, toMirSize(size));
    }
}

void WindowController::move(const miral::Window &window, const QPoint &topLeft)
{
    if (m_policy) {
        m_policy->move(window, toMirPoint(topLeft));
    }
}

void WindowController::requestClose(const miral::Window &window)
{
    if (m_policy) {
        m_policy->ask_client_to_close(window);
    }
}

void WindowController::forceClose(const miral::Window &window)
{
    if (m_policy) {
        m_policy->forceClose(window);
    }
}

void WindowController::requestState(const miral::Window &window, const Mir::State state)
{
    if (m_policy) {
        m_policy->requestState(window, state);
    }
}

void WindowController::setActiveFocus(const miral::Window &window, const bool focus)
{
      if (m_policy) {
        m_policy->setActiveFocus(window, focus);
    }
}

void WindowController::deliverKeyboardEvent(const miral::Window &window, const MirKeyboardEvent *event)
{
    if (m_policy) {
        m_policy->deliver_keyboard_event(event, window);
    }
}

void WindowController::deliverTouchEvent(const miral::Window &window, const MirTouchEvent *event)
{
    if (m_policy) {
        m_policy->deliver_touch_event(event, window);
    }
}

void WindowController::deliverPointerEvent(const miral::Window &window, const MirPointerEvent *event)
{
    if (m_policy) {
        m_policy->deliver_pointer_event(event, window);
    }
}

void WindowController::setWindowConfinementRegions(const QVector<QRect> &regions)
{
    if (m_policy) {
        m_policy->set_window_confinement_regions(regions);
    }
}

void WindowController::setWindowMargins(Mir::Type windowType, const QMargins &margins)
{
    if (m_policy) {
        m_policy->set_window_margins(toMirType(windowType), margins);
    }
}

void WindowController::setPolicy(WindowManagementPolicy * const policy)
{
    m_policy = policy;
}
