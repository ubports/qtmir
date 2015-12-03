/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
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

#include "surfaceobserver.h"

#include "namedcursor.h"

#include <QMetaObject>
#include <QImage>
#include <QPixmap>

#include <mir/geometry/size.h>

SurfaceObserver::SurfaceObserver()
    : m_listener(nullptr)
    , m_framesPosted(false)
{
    m_cursorNameToShape["left_ptr"] = Qt::ArrowCursor;
    m_cursorNameToShape["up_arrow"] = Qt::UpArrowCursor;
    m_cursorNameToShape["cross"] = Qt::CrossCursor;
    m_cursorNameToShape["watch"] = Qt::WaitCursor;
    m_cursorNameToShape["xterm"] = Qt::IBeamCursor;
    m_cursorNameToShape["size_ver"] = Qt::SizeVerCursor;
    m_cursorNameToShape["size_hor"] = Qt::SizeHorCursor;
    m_cursorNameToShape["size_bdiag"] = Qt::SizeBDiagCursor;
    m_cursorNameToShape["size_fdiag"] = Qt::SizeFDiagCursor;
    m_cursorNameToShape["size_all"] = Qt::SizeAllCursor;
    m_cursorNameToShape["blank"] = Qt::BlankCursor;
    m_cursorNameToShape["split_v"] = Qt::SplitVCursor;
    m_cursorNameToShape["split_h"] = Qt::SplitHCursor;
    m_cursorNameToShape["hand"] = Qt::PointingHandCursor;
    m_cursorNameToShape["forbidden"] = Qt::ForbiddenCursor;
    m_cursorNameToShape["whats_this"] = Qt::WhatsThisCursor;
    m_cursorNameToShape["left_ptr_watch"] = Qt::BusyCursor;
    m_cursorNameToShape["openhand"] = Qt::OpenHandCursor;
    m_cursorNameToShape["closedhand"] = Qt::ClosedHandCursor;
    m_cursorNameToShape["dnd-copy"] = Qt::DragCopyCursor;
    m_cursorNameToShape["dnd-move"] = Qt::DragMoveCursor;
    m_cursorNameToShape["dnd-link"] = Qt::DragLinkCursor;
}

void SurfaceObserver::setListener(QObject *listener)
{
    m_listener = listener;
    if (m_framesPosted) {
        Q_EMIT framesPosted();
    }
}

void SurfaceObserver::frame_posted(int /*frames_available*/)
{
    m_framesPosted = true;
    if (m_listener) {
        Q_EMIT framesPosted();
    }
}

void SurfaceObserver::renamed(char const * name)
{
    Q_EMIT nameChanged(QString::fromUtf8(name));
}

void SurfaceObserver::cursor_image_removed()
{
    Q_EMIT cursorChanged(QCursor());
}

void SurfaceObserver::attrib_changed(MirSurfaceAttrib attribute, int value)
{
    if (m_listener) {
        Q_EMIT attributeChanged(attribute, value);
    }
}

void SurfaceObserver::resized_to(mir::geometry::Size const&size)
{
    Q_EMIT resized(QSize(size.width.as_int(), size.height.as_int()));
}

void SurfaceObserver::cursor_image_set_to(const mir::graphics::CursorImage &cursorImage)
{
    QCursor qcursor = createQCursorFromMirCursorImage(cursorImage);
    Q_EMIT cursorChanged(qcursor);
}

QCursor SurfaceObserver::createQCursorFromMirCursorImage(const mir::graphics::CursorImage &cursorImage) {
    if (cursorImage.as_argb_8888() == nullptr) {
        // Must be a named cursor
        auto namedCursor = dynamic_cast<const qtmir::NamedCursor*>(&cursorImage);
        Q_ASSERT(namedCursor != nullptr);
        if (namedCursor) {
            // NB: If we need a named cursor not covered by Qt::CursorShape, we won't be able to
            //     used Qt's cursor API anymore for transmitting MirSurface's cursor image.
            Qt::CursorShape cursorShape = m_cursorNameToShape.value(namedCursor->name(), Qt::ArrowCursor);
            return QCursor(cursorShape);
        } else {
            // shouldn't happen
            return QCursor();
        }
    } else {
        QImage image((const uchar*)cursorImage.as_argb_8888(),
                cursorImage.size().width.as_int(), cursorImage.size().height.as_int(), QImage::Format_ARGB32);

        return QCursor(QPixmap::fromImage(image), cursorImage.hotspot().dx.as_int(), cursorImage.hotspot().dy.as_int());
    }
}
