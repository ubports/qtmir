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
    CompositorTexture()
        : m_currentFrameNumber(0)
        , m_textureUpdated(false)
    {
    }

    const QWeakPointer<QSGTexture>& texture() const { return m_texture; }
    void setTexture(const QWeakPointer<QSGTexture>& texture) {
        m_texture = texture;
    }

    int curentFrame() const { return m_currentFrameNumber; }
    void incrementFrame() { m_currentFrameNumber++; }

    bool isUpToDate() const { return m_textureUpdated; }
    void setUpToDate(bool updated) { m_textureUpdated = updated; }

private:
    QWeakPointer<QSGTexture> m_texture;
    int m_currentFrameNumber;
    bool m_textureUpdated;
};

}

#endif // COMPOSITORTEXTUREPROVIDER_H
