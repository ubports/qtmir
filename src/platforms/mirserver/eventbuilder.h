/*
 * Copyright (C) 2016-2017 Canonical, Ltd.
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

#ifndef QTMIR_EVENT_REGISTRY_H
#define QTMIR_EVENT_REGISTRY_H

#include <QtGlobal>
#include <QHoverEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTouchEvent>
#include <QVector>

#include <mir/events/event_builders.h>

class MirPointerEvent;

namespace qtmir {

/*
    Creates Mir input events out of Qt input events

    One important feature is that it's able to match a QInputEvent with the MirInputEvent that originated it, so
    it can make a MirInputEvent version of a QInputEvent containing also information that the latter does not carry,
    such as relative axis movement for pointer devices.
 */
class EventBuilder {
public:
    static EventBuilder *instance();
    EventBuilder();
    virtual ~EventBuilder();

    /* Stores information that cannot be carried by QInputEvents so that it can be fully
       reconstructed later given the same qtTimestamp */
    void store(const MirInputEvent *mirInputEvent, ulong qtTimestamp);

    /*
        Builds a MirEvent version of the given QInputEvent using also extra data from the
        MirPointerEvent that caused it.

        See also: store()
     */
    mir::EventUPtr reconstructMirEvent(QMouseEvent *event);
    mir::EventUPtr reconstructMirEvent(QHoverEvent *qtEvent);

    /*
        Makes a MirEvent version of the given QInputEvent using also extra data from the
        MirInputEvent that caused it.

        See also: store()
     */
    mir::EventUPtr makeMirEvent(QWheelEvent *qtEvent);
    mir::EventUPtr makeMirEvent(QKeyEvent *qtEvent);
    mir::EventUPtr makeMirEvent(Qt::KeyboardModifiers qmods,
                                const QList<QTouchEvent::TouchPoint> &qtTouchPoints,
                                Qt::TouchPointStates /* qtTouchPointStates */,
                                ulong qtTimestamp);
private:
    class EventInfo {
    public:
        void store(const MirInputEvent *mirInputEvent, ulong qtTimestamp);
        ulong qtTimestamp;
        MirInputDeviceId deviceId;
        std::vector<uint8_t> cookie;
        float relativeX{0};
        float relativeY{0};
    };

    EventInfo *findInfo(ulong qtTimestamp);

    mir::EventUPtr makeMirEvent(QInputEvent *qtEvent, int x, int y, MirPointerButtons buttons);


    /*
      Ring buffer that stores information on recent MirInputEvents that cannot be carried by QInputEvents.

      When MirInputEvents are dispatched through a QML scene, not all of its information can be carried
      by QInputEvents. Some information is lost. Thus further on, if we want to transform a QInputEvent back into
      its original MirInputEvent so that it can be consumed by a mir::scene::Surface and properly handled by mir clients
      we have to reach out to this EventRegistry to get the missing bits.

      Given the objective of this EventRegistry (MirInputEvent reconstruction after having gone through QQuickWindow input dispatch
      as a QInputEvent), it stores information only about the most recent MirInputEvents.
     */
    QVector<EventInfo> m_eventInfoVector;
    int m_nextIndex{0};
    int m_count{0};

    static EventBuilder *m_instance;
};

} // namespace qtmir

#endif // QTMIR_EVENT_REGISTRY_H
