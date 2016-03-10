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

#ifndef MIROPENGLCONTEXT_H
#define MIROPENGLCONTEXT_H

#include <qpa/qplatformopenglcontext.h>

#ifndef QT_NO_DEBUG
#include <QOpenGLDebugLogger>
#endif


class MirServer;
class ScreenWindow;

class MirOpenGLContext : public QObject, public QPlatformOpenGLContext
{
    Q_OBJECT
public:
    MirOpenGLContext(const QSharedPointer<MirServer> &, const QSurfaceFormat &);
    ~MirOpenGLContext() = default;

    QSurfaceFormat format() const override;
    void swapBuffers(QPlatformSurface *surface) override;

    bool makeCurrent(QPlatformSurface *surface) override;
    void doneCurrent() override;

    bool isSharing() const override { return false; }

    QFunctionPointer getProcAddress(const QByteArray &procName) override;

#ifndef QT_NO_DEBUG
    Q_SLOT void onGlDebugMessageLogged(QOpenGLDebugMessage m) { qDebug() << m; }
#endif

private:
    QSurfaceFormat m_format;
    ScreenWindow *m_currentWindow;
#ifndef QT_NO_DEBUG
    QOpenGLDebugLogger *m_logger;
#endif
};

#endif // MIROPENGLCONTEXT_H
