/*
 * Copyright © 2016 Canonical Ltd.
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

#ifndef QTMIR_SCREEN_TYPES_H
#define QTMIR_SCREEN_TYPES_H

#include <mir/int_wrapper.h>

#include <QtCore/qmetatype.h>
#include <QSize>
#include <QObject>

namespace mir { namespace graphics { namespace detail { struct GraphicsConfOutputIdTag; } } }

namespace qtmir
{
using OutputId = mir::IntWrapper<mir::graphics::detail::GraphicsConfOutputIdTag>;

enum OutputTypes {
    Unknown,
    VGA,
    DVII,
    DVID,
    DVIA,
    Composite,
    SVideo,
    LVDS,
    Component,
    NinePinDIN,
    DisplayPort,
    HDMIA,
    HDMIB,
    TV,
    EDP
};

enum FormFactor {
    FormFactorUnknown,
    FormFactorPhone,
    FormFactorTablet,
    FormFactorMonitor,
    FormFactorTV,
    FormFactorProjector,
};

class ScreenMode : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal refreshRate MEMBER refreshRate CONSTANT)
    Q_PROPERTY(QSize size MEMBER size CONSTANT)
public:
    ScreenMode():refreshRate(-1) {}
    ScreenMode(const ScreenMode& other)
        : QObject(nullptr),
          refreshRate{other.refreshRate},size{other.size}
    {}

    qreal refreshRate;
    QSize size;
};

}

Q_DECLARE_METATYPE(qtmir::OutputTypes)
Q_DECLARE_METATYPE(qtmir::FormFactor)
Q_DECLARE_METATYPE(qtmir::ScreenMode)

#endif //QTMIR_SCREEN_TYPES_H
