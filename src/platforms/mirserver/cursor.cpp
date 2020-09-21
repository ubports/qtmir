/*
 * Copyright (C) 2015 Canonical, Ltd.
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
 *
 */

#include "cursor.h"
#include "logging.h"

#include "mirsingleton.h"

#include <QQuickWindow>

// Unity API
#include <unity/shell/application/MirMousePointerInterface.h>

namespace qtmir {

// Shares data between all pointers in the server
class Cursor::Private : public QObject
{
    Q_OBJECT
public:
    Private()
        : m_customCursor(nullptr)
    {
        m_shapeToCursorName[Qt::ArrowCursor] = QStringLiteral("left_ptr");
        m_shapeToCursorName[Qt::UpArrowCursor] = QStringLiteral("up_arrow");
        m_shapeToCursorName[Qt::CrossCursor] = QStringLiteral("cross");
        m_shapeToCursorName[Qt::WaitCursor] = QStringLiteral("watch");
        m_shapeToCursorName[Qt::IBeamCursor] = QStringLiteral("xterm");
        m_shapeToCursorName[Qt::SizeVerCursor] = QStringLiteral("size_ver");
        m_shapeToCursorName[Qt::SizeHorCursor] = QStringLiteral("size_hor");
        m_shapeToCursorName[Qt::SizeBDiagCursor] = QStringLiteral("size_bdiag");
        m_shapeToCursorName[Qt::SizeFDiagCursor] = QStringLiteral("size_fdiag");
        m_shapeToCursorName[Qt::SizeAllCursor] = QStringLiteral("size_all");
        m_shapeToCursorName[Qt::BlankCursor] = QStringLiteral("blank");
        m_shapeToCursorName[Qt::SplitVCursor] = QStringLiteral("split_v");
        m_shapeToCursorName[Qt::SplitHCursor] = QStringLiteral("split_h");
        m_shapeToCursorName[Qt::PointingHandCursor] = QStringLiteral("hand");
        m_shapeToCursorName[Qt::ForbiddenCursor] = QStringLiteral("forbidden");
        m_shapeToCursorName[Qt::WhatsThisCursor] = QStringLiteral("whats_this");
        m_shapeToCursorName[Qt::BusyCursor] = QStringLiteral("left_ptr_watch");
        m_shapeToCursorName[Qt::OpenHandCursor] = QStringLiteral("openhand");
        m_shapeToCursorName[Qt::ClosedHandCursor] = QStringLiteral("closedhand");
        m_shapeToCursorName[Qt::DragCopyCursor] = QStringLiteral("dnd-copy");
        m_shapeToCursorName[Qt::DragMoveCursor] = QStringLiteral("dnd-move");
        m_shapeToCursorName[Qt::DragLinkCursor] = QStringLiteral("dnd-link");

        connect(Mir::instance(), &Mir::cursorNameChanged, this, &Private::setMirCursorName);
        m_mirCursorName = Mir::instance()->cursorName();
    }

    void setScreenPosition(const QPoint& screenPos)
    {
        m_position = screenPos;
        Q_EMIT screenPositionChanged(m_position);
    }

    QPoint screenPosition() const { return m_position; }

    void setCustomCursor(QCursor* custom, const QString& qtCursorName)
    {
        delete m_customCursor;
        m_customCursor = custom;

        m_qtCursorName  = qtCursorName;

        Q_EMIT cursorChanged(customCursor(), cursorName());
    }

    QCursor customCursor() const { return m_customCursor ? *m_customCursor : QCursor(); }

    QString cursorName() const
    {
        if (m_mirCursorName.isEmpty()) {
            if (m_qtCursorName.isEmpty()) {
               return QStringLiteral("left_ptr");
            } else {
                return m_qtCursorName;
            }
        } else {
            return m_mirCursorName;
        }
    }

Q_SIGNALS:
    void screenPositionChanged(const QPoint& screenPos);
    void cursorChanged(const QCursor& cursor, const QString& cursorName);

private Q_SLOTS:
    void setMirCursorName(const QString &mirCursorName)
    {
        m_mirCursorName = mirCursorName;
        Q_EMIT cursorChanged(customCursor(), cursorName());
    }

public:
    QMutex m_mutex;
    QMap<int,QString> m_shapeToCursorName;

    QCursor* m_customCursor;
    QString m_mirCursorName;
    QString m_qtCursorName;
    QPoint m_position;
};

Cursor::Cursor()
    : d(new Cursor::Private)
{
}

void Cursor::changeCursor(QCursor *windowCursor, QWindow * /*window*/)
{
    if (windowCursor) {
        if (windowCursor->pixmap().isNull()) {
            QString qtCursorName = d->m_shapeToCursorName.value(windowCursor->shape(), QLatin1String("left_ptr"));
            d->setCustomCursor(nullptr, qtCursorName);
        } else {
            // Ensure we get different names for consecutive custom cursors.
            // The name doesn't have to be unique (ie, different from all custom cursor names generated so far),
            // just different from the previous custom cursor name, which is enough to trigger a change in the cursor
            // source image URL in the QML side which on is turn makes QML request the new cursor image.
            static quint8 serialNumber = 1;
            QString qtCursorName = QStringLiteral("custom%1").arg(serialNumber++);
            d->setCustomCursor(new QCursor(*windowCursor), qtCursorName);
        }
    } else {
        d->setCustomCursor(nullptr, QString());
    }
}

void Cursor::registerMousePointer(MirMousePointerInterface *mousePointer)
{
    QMutexLocker locker(&d->m_mutex);

    auto updatePositionFunction = [mousePointer](const QPoint& screenPos) {
        if (!mousePointer->window()) return;
        if (!mousePointer->isEnabled()) return;

        QPoint localPos = screenPos - mousePointer->window()->geometry().topLeft();
        mousePointer->moveTo(localPos);
    };

    connect(d.data(), &Private::screenPositionChanged, mousePointer, updatePositionFunction, Qt::UniqueConnection);
    connect(d.data(), &Private::cursorChanged, mousePointer, [this, mousePointer](const QCursor& cursor, const QString& cursorName) {
        mousePointer->setCustomCursor(cursor);
        mousePointer->setCursorName(cursorName);
    }, Qt::UniqueConnection);

    mousePointer->setCustomCursor(d->customCursor());
    mousePointer->setCursorName(d->cursorName());
}

void Cursor::unregisterMousePointer(MirMousePointerInterface *mousePointer)
{
    QMutexLocker locker(&d->m_mutex);

    disconnect(d.data(), &Private::screenPositionChanged, mousePointer, 0);
    disconnect(d.data(), &Private::cursorChanged, mousePointer, 0);
}

void Cursor::pointerEvent(const QMouseEvent &event)
{
    QMutexLocker locker(&d->m_mutex);

    auto pos = event.globalPos();
    qCDebug(QTMIR_MIR_INPUT) << "Cursor::pointerEvent(x=" << pos.x() << ", y=" << pos.y() << ")";

    d->setScreenPosition(pos);
}

void Cursor::setPos(const QPoint &pos)
{
    QMutexLocker locker(&d->m_mutex);
    qCDebug(QTMIR_MIR_INPUT) << "Cursor::setPos(x=" << pos.x() << ", y=" << pos.y() << ")";

    d->setScreenPosition(pos);
}

QPoint Cursor::pos() const
{
    return d->screenPosition();
}

} // namespace qtmir

#include "cursor.moc"
