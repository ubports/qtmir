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
 */

#include "offscreensurface.h"

// Mir
#include <mir/graphics/display.h>
#include <mir/graphics/gl_context.h>

//Qt
#include <QOffscreenSurface>
#include <QOpenGLFramebufferObject>
#include <QSurfaceFormat>

#if QT_VERSION >= 0x050800
#include <QtEglSupport/private/qeglconvenience_p.h>
#else
#include <QtPlatformSupport/private/qeglconvenience_p.h>
#endif

namespace mg = mir::graphics;

OffscreenSurface::OffscreenSurface(QOffscreenSurface *offscreenSurface)
    : QPlatformOffscreenSurface(offscreenSurface)
    , m_buffer(nullptr)
    , m_format(offscreenSurface->requestedFormat())
{
}

QSurfaceFormat OffscreenSurface::format() const
{
    return m_format;
}

bool OffscreenSurface::isValid() const
{
    if (m_buffer) {
        return m_buffer->isValid();
    }
    return false;
}

QOpenGLFramebufferObject* OffscreenSurface::buffer() const
{
    return m_buffer;
}

void OffscreenSurface::setBuffer(QOpenGLFramebufferObject *buffer)
{
    m_buffer = buffer;
}
