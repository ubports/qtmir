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
class SharedPointerData : public QObject
{
    Q_OBJECT
public:
    SharedPointerData()
        : m_customCursor(nullptr)
    {
        connect(Mir::instance(), &Mir::cursorNameChanged, this, &SharedPointerData::setMirCursorName);
        m_mirCursorName = Mir::instance()->cursorName();
    }

    static QSharedPointer<SharedPointerData> instance()
    {
        if (!sharedData) {
            QSharedPointer<SharedPointerData> data(new SharedPointerData);
            sharedData = data.toWeakRef();
            return data;
        }
        return sharedData.toStrongRef();
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

private:
    QCursor* m_customCursor;
    QString m_mirCursorName;
    QString m_qtCursorName;
    QPoint m_position;

    static QWeakPointer<SharedPointerData> sharedData;
};

QWeakPointer<SharedPointerData> SharedPointerData::sharedData;

Cursor::Cursor()
    : m_sharedPointer(SharedPointerData::instance())
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
}

void Cursor::changeCursor(QCursor *windowCursor, QWindow * /*window*/)
{
    if (windowCursor) {
        if (windowCursor->pixmap().isNull()) {
            QString qtCursorName = m_shapeToCursorName.value(windowCursor->shape(), QLatin1String("left_ptr"));
            m_sharedPointer->setCustomCursor(nullptr, qtCursorName);
        } else {
            // Ensure we get different names for consecutive custom cursors.
            // The name doesn't have to be unique (ie, different from all custom cursor names generated so far),
            // just different from the previous custom cursor name, which is enough to trigger a change in the cursor
            // source image URL in the QML side which on is turn makes QML request the new cursor image.
            static quint8 serialNumber = 1;
            QString qtCursorName = QStringLiteral("custom%1").arg(serialNumber++);
            m_sharedPointer->setCustomCursor(new QCursor(*windowCursor), qtCursorName);
        }
    } else {
        m_sharedPointer->setCustomCursor(nullptr, QString());
    }
}

void Cursor::registerMousePointer(MirMousePointerInterface *mousePointer)
{
    QMutexLocker locker(&m_mutex);

    auto updatePositionFunction = [mousePointer](const QPoint& screenPos) {
        if (!mousePointer->window()) return;
        if (!mousePointer->isEnabled()) return;

        QPoint localPos = screenPos - mousePointer->window()->geometry().topLeft();
        mousePointer->setPosition(localPos);
    };

    connect(m_sharedPointer.data(), &SharedPointerData::screenPositionChanged, mousePointer, updatePositionFunction, Qt::UniqueConnection);
    connect(m_sharedPointer.data(), &SharedPointerData::cursorChanged, mousePointer, [this, mousePointer](const QCursor& cursor, const QString& cursorName) {
        mousePointer->setCursor(cursor);
        mousePointer->setCursorName(cursorName);
    }, Qt::UniqueConnection);

    mousePointer->setCursor(m_sharedPointer->customCursor());
    mousePointer->setCursorName(m_sharedPointer->cursorName());
}

void Cursor::unregisterMousePointer(MirMousePointerInterface *mousePointer)
{
    QMutexLocker locker(&m_mutex);

    disconnect(m_sharedPointer.data(), &SharedPointerData::screenPositionChanged, mousePointer, 0);
    disconnect(m_sharedPointer.data(), &SharedPointerData::cursorChanged, mousePointer, 0);
}

void Cursor::pointerEvent(const QMouseEvent &event)
{
    QMutexLocker locker(&m_mutex);

    auto pos = event.globalPos();
    qCDebug(QTMIR_MIR_INPUT) << "Cursor::pointerEvent(x=" << pos.x() << ", y=" << pos.y() << ")";

    m_sharedPointer->setScreenPosition(pos);
}

void Cursor::setPos(const QPoint &pos)
{
    QMutexLocker locker(&m_mutex);
    qCDebug(QTMIR_MIR_INPUT) << "Cursor::setPos(x=" << pos.x() << ", y=" << pos.y() << ")";

    m_sharedPointer->setScreenPosition(pos);
}

QPoint Cursor::pos() const
{
    return m_sharedPointer->screenPosition();
}

} // namespace qtmir

#include "cursor.moc"
