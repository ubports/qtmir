/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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

#include "mirbuffersgtexture.h"

// Mir
#include <mir/geometry/size.h>

#include <QMutexLocker>

namespace mg = mir::geometry;

MirBufferSGTexture::MirBufferSGTexture()
    : QSGTexture()
    , m_width(0)
    , m_height(0)
    , m_textureId(0)
    , m_needsUpdate(false)
{
    setFiltering(QSGTexture::Linear);
    setHorizontalWrapMode(QSGTexture::ClampToEdge);
    setVerticalWrapMode(QSGTexture::ClampToEdge);
}

MirBufferSGTexture::~MirBufferSGTexture()
{
}

void MirBufferSGTexture::freeBuffer()
{
    QMutexLocker locker(&m_mutex);

    m_mirBuffer.reset();
    m_width = 0;
    m_height = 0;
}

void MirBufferSGTexture::setBuffer(const std::shared_ptr<mir::graphics::Buffer>& buffer)
{
    QMutexLocker locker(&m_mutex);

    m_mirBuffer.reset(buffer);
    mg::Size size = m_mirBuffer.size();
    m_height = size.height.as_int();
    m_width = size.width.as_int();
    m_needsUpdate = true;
}

bool MirBufferSGTexture::hasBuffer() const
{
    return m_mirBuffer;
}

int MirBufferSGTexture::textureId() const
{
    QMutexLocker locker(&m_mutex);

    if (m_needsUpdate)
    {
        GLint existing_binding;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &existing_binding);
        m_mirBuffer.bind();
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_textureId);
        glBindTexture(GL_TEXTURE_2D, existing_binding);
        m_needsUpdate = false;
    }
    return m_textureId;
}

QSize MirBufferSGTexture::textureSize() const
{
    return QSize(m_width, m_height);
}

bool MirBufferSGTexture::hasAlphaChannel() const
{
    return m_mirBuffer.has_alpha_channel();
}

void MirBufferSGTexture::bind()
{
    QMutexLocker locker(&m_mutex);

    Q_ASSERT(hasBuffer());
    updateBindOptions(true/* force */);

    m_mirBuffer.bind();
}
