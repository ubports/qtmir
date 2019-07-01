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

#ifndef COMPOSITORTEXTUREPROVIDER_H
#define COMPOSITORTEXTUREPROVIDER_H

#include <functional>
#include <qglobal.h>
#include <QSharedPointer>

class QSGTexture;

namespace qtmir {

class CompositorTexture;

class CompositorTextureProvider
{
public:
    CompositorTextureProvider();
    virtual ~CompositorTextureProvider();

    CompositorTexture *compositorTextureForId(qintptr userId) const;

    QSharedPointer<QSGTexture> texture(qintptr userId);

    void forEachCompositorTexture(std::function<void(qintptr, CompositorTexture*)> f);

protected:
    // for testing
    virtual QSGTexture *createTexture() const;

private:
    QHash<qintptr, CompositorTexture*> m_textures;
};

class CompositorTexture
{
public:
    ~CompositorTexture() = default;

    const QWeakPointer<QSGTexture>& texture() const { return m_texture; }
    unsigned int currentFrame() const { return m_currentFrameNumber; }
    void incrementFrame();

    bool isUpToDate() const { return m_textureUpdated; }
    void setUpToDate(bool updated);

private:
    CompositorTexture();

    void setTexture(const QWeakPointer<QSGTexture>& texture);

private:
    QWeakPointer<QSGTexture> m_texture;
    unsigned int m_currentFrameNumber;
    bool m_textureUpdated;

    friend class CompositorTextureProvider;
};

}

#endif // COMPOSITORTEXTUREPROVIDER_H
