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

#ifndef MIRQTCONVERSION_H
#define MIRQTCONVERSION_H

#include <QSize>
#include <QPoint>
#include <QRect>

#include <mir/geometry/size.h>
#include <mir/geometry/point.h>
#include <mir/geometry/rectangle.h>

#include <mir_toolkit/common.h>

#include <unity/shell/application/Mir.h>

namespace qtmir {

/*
 * Some handy conversions from Mir types to Qt types and back
 */

inline QSize toQSize(const mir::geometry::Size size)
{
    return QSize(size.width.as_int(), size.height.as_int());
}

inline mir::geometry::Size toMirSize(const QSize size)
{
    namespace mg = mir::geometry;
    return mg::Size{ mg::Width{ size.width()}, mg::Height{ size.height()} };
}

inline QPoint toQPoint(const mir::geometry::Point point)
{
    return QPoint(point.x.as_int(), point.y.as_int());
}

inline mir::geometry::Point toMirPoint(const QPoint point)
{
    namespace mg = mir::geometry;
    return mg::Point{ mg::X{ point.x()}, mg::Y{ point.y()} };
}

inline QRect toQRect(const mir::geometry::Rectangle rect)
{
    return QRect(rect.top_left.x.as_int(), rect.top_left.y.as_int(),
                 rect.size.width.as_int(), rect.size.height.as_int());
}

inline mir::geometry::Rectangle toMirRectangle(const QRect rect)
{
    namespace mg = mir::geometry;
    return mg::Rectangle{
        mg::Point{ mg::X{ rect.x()}, mg::Y{ rect.y()} },
        mg::Size{ mg::Width{ rect.width()}, mg::Height{ rect.height()} }
    };
}

inline Mir::State toQtState(MirWindowState state)
{
    switch (state) {
    case mir_window_state_unknown:         return Mir::UnknownState;
    case mir_window_state_restored:        return Mir::RestoredState;
    case mir_window_state_minimized:       return Mir::MinimizedState;
    case mir_window_state_maximized:       return Mir::MaximizedState;
    case mir_window_state_vertmaximized:   return Mir::VertMaximizedState;
    case mir_window_state_fullscreen:      return Mir::FullscreenState;
    case mir_window_state_horizmaximized:  return Mir::HorizMaximizedState;
    case mir_window_state_hidden:          return Mir::HiddenState;
    case mir_window_states:                Q_UNREACHABLE();
    }
    Q_UNREACHABLE();
}

inline MirWindowState toMirState(Mir::State state)
{
    switch (state) {
    case Mir::UnknownState:         return mir_window_state_unknown;
    case Mir::RestoredState:        return mir_window_state_restored;
    case Mir::MinimizedState:       return mir_window_state_minimized;
    case Mir::MaximizedState:       return mir_window_state_maximized;
    case Mir::VertMaximizedState:   return mir_window_state_vertmaximized;
    case Mir::FullscreenState:      return mir_window_state_fullscreen;
    case Mir::HorizMaximizedState:  return mir_window_state_horizmaximized;

    // FIXME: Map to the corresponding MirWindowState enum value once available
    case Mir::MaximizedLeftState:
    case Mir::MaximizedRightState:
    case Mir::MaximizedTopLeftState:
    case Mir::MaximizedTopRightState:
    case Mir::MaximizedBottomLeftState:
    case Mir::MaximizedBottomRightState:
        return mir_window_state_restored;

    case Mir::HiddenState:          return mir_window_state_hidden;
    default: Q_UNREACHABLE();
    }
}

} // namespace qtmir

#endif // MIRQTCONVERSION_H
