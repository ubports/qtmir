/*
 * Copyright (C) 2014-2016 Canonical, Ltd.
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
#include "logging.h"

#include <QImage>
#include <QMetaObject>
#include <QMutableMapIterator>
#include <QMutexLocker>
#include <QPixmap>

#include <mir/geometry/size.h>
#include <mir/shell/surface_specification.h>


namespace {

QRect calculateBoundingRect(const std::vector<mir::geometry::Rectangle> &rectVector)
{
    QRect boundingRect;
    for (auto mirRect : rectVector) {
        boundingRect |= QRect(mirRect.top_left.x.as_int(),
                mirRect.top_left.y.as_int(),
                mirRect.size.width.as_int(),
                mirRect.size.height.as_int());
    }
    return boundingRect;
}

} // anonymous namespace

QHash<const mir::scene::Surface*, SurfaceObserver*> SurfaceObserver::m_surfaceToObserverMap;
QMutex SurfaceObserver::mutex;

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

    // Used by Mir client API (mir_*_cursor_name strings)
    m_cursorNameToShape["default"] = Qt::ArrowCursor;
    m_cursorNameToShape["disabled"] = Qt::BlankCursor;
    m_cursorNameToShape["arrow"] = Qt::ArrowCursor;
    m_cursorNameToShape["busy"] = Qt::WaitCursor;
    m_cursorNameToShape["caret"] = Qt::IBeamCursor;
    m_cursorNameToShape["pointing-hand"] = Qt::PointingHandCursor;
    m_cursorNameToShape["open-hand"] = Qt::OpenHandCursor;
    m_cursorNameToShape["closed-hand"] = Qt::ClosedHandCursor;
    m_cursorNameToShape["horizontal-resize"] = Qt::SizeHorCursor;
    m_cursorNameToShape["vertical-resize"] = Qt::SizeVerCursor;
    m_cursorNameToShape["diagonal-resize-bottom-to-top"] = Qt::SizeBDiagCursor;
    m_cursorNameToShape["diagonal-resize-top_to_bottom"] = Qt::SizeFDiagCursor; // current string with typo
    m_cursorNameToShape["diagonal-resize-top-to-bottom"] = Qt::SizeFDiagCursor; // how it will be when they fix it (if ever)
    m_cursorNameToShape["omnidirectional-resize"] = Qt::SizeAllCursor;
    m_cursorNameToShape["vsplit-resize"] = Qt::SplitVCursor;
    m_cursorNameToShape["hsplit-resize"] = Qt::SplitHCursor;
    m_cursorNameToShape["crosshair"] = Qt::CrossCursor;

    qRegisterMetaType<MirShellChrome>("MirShellChrome");
}

SurfaceObserver::~SurfaceObserver()
{
    QMutexLocker locker(&mutex);
    QMutableHashIterator<const mir::scene::Surface*, SurfaceObserver*> i(m_surfaceToObserverMap);
    while (i.hasNext()) {
        i.next();
        if (i.value() == this) {
            i.remove();
            return;
        }
    }
}

void SurfaceObserver::setListener(QObject *listener)
{
    m_listener = listener;
    if (m_framesPosted) {
        Q_EMIT framesPosted();
    }
}

void SurfaceObserver::frame_posted(int /*frames_available*/, mir::geometry::Size const& /*size*/)
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

void SurfaceObserver::notifySurfaceModifications(const mir::shell::SurfaceSpecification &modifications)
{
    if (modifications.min_width.is_set()) {
        Q_EMIT minimumWidthChanged(modifications.min_width.value().as_int());
    }
    if (modifications.min_height.is_set()) {
        Q_EMIT minimumHeightChanged(modifications.min_height.value().as_int());
    }
    if (modifications.max_width.is_set()) {
        Q_EMIT maximumWidthChanged(modifications.max_width.value().as_int());
    }
    if (modifications.max_height.is_set()) {
        Q_EMIT maximumHeightChanged(modifications.max_height.value().as_int());
    }
    if (modifications.width_inc.is_set()) {
        Q_EMIT widthIncrementChanged(modifications.width_inc.value().as_int());
    }
    if (modifications.height_inc.is_set()) {
        Q_EMIT heightIncrementChanged(modifications.height_inc.value().as_int());
    }
    if (modifications.shell_chrome.is_set()) {
        Q_EMIT shellChromeChanged(modifications.shell_chrome.value());
    }
    if (modifications.input_shape.is_set()) {
        QRect qRect = calculateBoundingRect(modifications.input_shape.value());
        Q_EMIT inputBoundsChanged(qRect);
    }
    if (modifications.confine_pointer.is_set()) {
        Q_EMIT confinesMousePointerChanged(modifications.confine_pointer.value() == mir_pointer_confined_to_surface);
    }
}

SurfaceObserver *SurfaceObserver::observerForSurface(const mir::scene::Surface *surface)
{
    if (m_surfaceToObserverMap.contains(surface)) {
        return m_surfaceToObserverMap.value(surface);
    } else {
        return nullptr;
    }
}

void SurfaceObserver::registerObserverForSurface(SurfaceObserver *observer, const mir::scene::Surface *surface)
{
    QMutexLocker locker(&mutex);
    m_surfaceToObserverMap[surface] = observer;
}

void SurfaceObserver::cursor_image_set_to(const mir::graphics::CursorImage &cursorImage)
{
    QCursor qcursor = createQCursorFromMirCursorImage(cursorImage);
    Q_EMIT cursorChanged(qcursor);
}

// TODO Implement
void SurfaceObserver::placed_relative(mir::geometry::Rectangle const& /*placement*/)
{
}

QCursor SurfaceObserver::createQCursorFromMirCursorImage(const mir::graphics::CursorImage &cursorImage) {
    if (cursorImage.as_argb_8888() == nullptr) {
        // Must be a named cursor
        auto namedCursor = dynamic_cast<const qtmir::NamedCursor*>(&cursorImage);
        Q_ASSERT(namedCursor != nullptr);
        if (namedCursor) {
            // NB: If we need a named cursor not covered by Qt::CursorShape, we won't be able to
            //     used Qt's cursor API anymore for transmitting MirSurface's cursor image.

            Qt::CursorShape cursorShape = Qt::ArrowCursor;
            {
                auto iterator = m_cursorNameToShape.constFind(namedCursor->name());
                if (iterator == m_cursorNameToShape.constEnd()) {
                    qCWarning(QTMIR_SURFACES).nospace() << "SurfaceObserver: unrecognized cursor name "
                        << namedCursor->name();
                } else {
                    cursorShape = iterator.value();
                }
            }
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
