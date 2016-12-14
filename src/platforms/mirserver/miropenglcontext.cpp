/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
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

#include "miropenglcontext.h"

#include "offscreensurface.h"
#include "mirglconfig.h"
#include "screenwindow.h"

#include <QDebug>

#include <QOpenGLFramebufferObject>
#include <QSurfaceFormat>
#include <QtPlatformSupport/private/qeglconvenience_p.h>
#include <QtGui/private/qopenglcontext_p.h>

// Mir
#include <mir/graphics/display.h>
#include <mir/renderer/gl/context.h>
#include <mir/renderer/gl/context_source.h>

// Qt supports one GL context per screen, but also shared contexts.
// The Mir "Display" generates a shared GL context for all DisplayBuffers
// (i.e. individual display output buffers) to use as a common base context.

MirOpenGLContext::MirOpenGLContext(
    mir::graphics::Display &display,
    mir::graphics::GLConfig &gl_config,
    const QSurfaceFormat &format)
    : m_currentWindow(nullptr)
#ifdef QGL_DEBUG
      , m_logger(new QOpenGLDebugLogger(this))
#endif
{
    // create a temporary GL context to fetch the EGL display and config, so Qt can determine the surface format
    std::unique_ptr<mir::renderer::gl::Context> mirContext = dynamic_cast<mir::renderer::gl::ContextSource*>(
                                                                display.native_display())->create_gl_context();
    mirContext->make_current();

    EGLDisplay eglDisplay = eglGetCurrentDisplay();
    if (eglDisplay == EGL_NO_DISPLAY) {
        qFatal("Unable to determine current EGL Display");
    }
    EGLContext eglContext = eglGetCurrentContext();
    if (eglContext == EGL_NO_CONTEXT) {
        qFatal("Unable to determine current EGL Context");
    }
    EGLint eglConfigId = -1;
    EGLBoolean result;
    result = eglQueryContext(eglDisplay, eglContext, EGL_CONFIG_ID, &eglConfigId);
    if (result != EGL_TRUE || eglConfigId < 0) {
        qFatal("Unable to determine current EGL Config ID");
    }

    EGLConfig eglConfig;
    EGLint matchingEglConfigCount;
    EGLint const attribList[] = {
        EGL_CONFIG_ID, eglConfigId,
        EGL_NONE
    };
    result = eglChooseConfig(eglDisplay, attribList, &eglConfig, 1, &matchingEglConfigCount);
    if (result != EGL_TRUE || eglConfig == nullptr || matchingEglConfigCount < 1) {
        qFatal("Unable to select EGL Config with the supposed current config ID");
    }

    QSurfaceFormat formatCopy = format;
    formatCopy.setRenderableType(QSurfaceFormat::OpenGLES);

    m_format = q_glFormatFromConfig(eglDisplay, eglConfig, formatCopy);
    mirContext->release_current(); // Need to release as it doesn't happen when GLContext goes out of scope

    // FIXME: the temporary gl context created by Mir does not have the attributes we specified
    // in the GLConfig, so need to set explicitly for now
    m_format.setDepthBufferSize(gl_config.depth_buffer_bits());
    m_format.setStencilBufferSize(gl_config.stencil_buffer_bits());
    m_format.setSamples(-1);

#ifdef QGL_DEBUG
    const char* string = (const char*) glGetString(GL_VENDOR);
    qDebug() << "OpenGL ES vendor: " << qPrintable(string);
    string = (const char*) glGetString(GL_RENDERER);
    qDebug() << "OpenGL ES renderer"  << qPrintable(string);
    string = (const char*) glGetString(GL_VERSION);
    qDebug() << "OpenGL ES version" << qPrintable(string);
    string = (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION);
    qDebug() << "OpenGL ES Shading Language version:" << qPrintable(string);
    string = (const char*) glGetString(GL_EXTENSIONS);
    qDebug() << "OpenGL ES extensions:" << qPrintable(string);
    q_printEglConfig(eglDisplay, eglConfig);

    QObject::connect(m_logger, &QOpenGLDebugLogger::messageLogged,
                     this, &MirOpenGLContext::onGlDebugMessageLogged, Qt::DirectConnection);
#endif // debug
    mirContext->release_current(); // Need to release as it doesn't happen when GLContext goes out of scope
}

QSurfaceFormat MirOpenGLContext::format() const
{
    return m_format;
}

void MirOpenGLContext::swapBuffers(QPlatformSurface *surface)
{
    if (surface->surface()->surfaceClass() == QSurface::Offscreen) {
        // NOOP
    } else {
        // ultimately calls Mir's DisplayBuffer::post_update()
        ScreenWindow *screenWindow = static_cast<ScreenWindow*>(surface);
        screenWindow->swapBuffers(); //blocks for vsync
    }
}

static bool needsFBOReadBackWorkaround()
{
    static bool set = false;
    static bool needsWorkaround = false;

    if (Q_UNLIKELY(!set)) {
        const char *rendererString = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
        // Keep in sync with qtubuntu
        needsWorkaround = qstrncmp(rendererString, "Mali-400", 8) == 0
                          || qstrncmp(rendererString, "Mali-T7", 7) == 0
                          || qstrncmp(rendererString, "PowerVR Rogue G6200", 19) == 0;
        set = true;
    }

    return needsWorkaround;
}

bool MirOpenGLContext::makeCurrent(QPlatformSurface *surface)
{
    if (surface->surface()->surfaceClass() == QSurface::Offscreen) {
        auto offscreen = static_cast<OffscreenSurface *>(surface);
        if (!offscreen->buffer()) {
            auto buffer = new QOpenGLFramebufferObject(surface->surface()->size());
            offscreen->setBuffer(buffer);
        }
        return offscreen->buffer()->bind();
    }

    // ultimately calls Mir's DisplayBuffer::make_current()
    ScreenWindow *screenWindow = static_cast<ScreenWindow*>(surface);
    if (Q_LIKELY(screenWindow)) {
        m_currentWindow = screenWindow;
        screenWindow->makeCurrent();

#ifdef QGL_DEBUG
        if (!m_logger->isLogging() && m_logger->initialize()) {
            m_logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
            m_logger->enableMessages();
        }
#endif

        QOpenGLContextPrivate *ctx_d = QOpenGLContextPrivate::get(context());
        if (!ctx_d->workaround_brokenFBOReadBack && needsFBOReadBackWorkaround())
            ctx_d->workaround_brokenFBOReadBack = true;

        return true;
    }

    return false;
}

void MirOpenGLContext::doneCurrent()
{
    if (m_currentWindow) {
        m_currentWindow->doneCurrent();
        m_currentWindow = nullptr;
    }
}

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
QFunctionPointer MirOpenGLContext::getProcAddress(const QByteArray &procName)
{
    return eglGetProcAddress(procName.constData());
}
#else
QFunctionPointer MirOpenGLContext::getProcAddress(const char *procName)
{
    return eglGetProcAddress(procName);
}
#endif
