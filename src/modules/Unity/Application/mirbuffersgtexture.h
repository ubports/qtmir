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

#ifndef MIRBUFFERSGTEXTURE_H
#define MIRBUFFERSGTEXTURE_H

#include "miral/mirbuffer.h"

#include <QSGTexture>

#include <QtGui/qopengl.h>

#include <QMutex>

class MirBufferSGTexture : public QSGTexture
{
    Q_OBJECT
public:
    MirBufferSGTexture();
    virtual ~MirBufferSGTexture();

    void setBuffer(const std::shared_ptr<mir::graphics::Buffer>& buffer);
    void freeBuffer();
    bool hasBuffer() const;

    int textureId() const override;
    QSize textureSize() const override;
    bool hasAlphaChannel() const override;
    bool hasMipmaps() const override { return false; }

    void bind() override;

private:
    mutable miral::GLBuffer m_mirBuffer;
    int m_width;
    int m_height;
    mutable GLint m_textureId;
    mutable bool m_needsUpdate;
    mutable QMutex m_mutex;
};

#endif // MIRBUFFERSGTEXTURE_H
