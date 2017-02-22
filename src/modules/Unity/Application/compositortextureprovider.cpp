/*
 * Copyright (C) 2017 Canonical, Ltd.
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

#include "compositortextureprovider.h"
#include "mirbuffersgtexture.h"

using namespace qtmir;

CompositorTextureProvider::CompositorTextureProvider()
{
}

CompositorTextureProvider::~CompositorTextureProvider()
{
    qDeleteAll(m_textures);
    m_textures.clear();
}

CompositorTexture *CompositorTextureProvider::compositorTextureForId(qintptr userId) const
{
    return m_textures.value(userId, nullptr);
}

QSharedPointer<QSGTexture> CompositorTextureProvider::texture(qintptr userId)
{
    CompositorTexture* compositorTexture = compositorTextureForId(userId);
    if (!compositorTexture || !compositorTexture->texture()) {
        QSharedPointer<QSGTexture> texture(createTexture());
        if (!compositorTexture) {
            compositorTexture = new CompositorTexture();
            m_textures[userId] = compositorTexture;
        }
        compositorTexture->setTexture(texture);
        return texture;
    } else {
        return compositorTexture->texture();
    }
}

void CompositorTextureProvider::forEachCompositorTexture(std::function<void (qintptr, CompositorTexture *)> f)
{
    for (auto iter = m_textures.constBegin(); iter != m_textures.constEnd(); ++iter) {
        const qintptr userId = iter.key();
        CompositorTexture* compositorTexture = iter.value();

        f(userId, compositorTexture);
    }
}

QSGTexture *CompositorTextureProvider::createTexture() const
{
    return new MirBufferSGTexture;
}
