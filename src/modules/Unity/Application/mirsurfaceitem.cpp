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

// local
#include "application.h"
#include "session.h"
#include "mirsurfaceitem.h"
#include "logging.h"
#include "tracepoints.h" // generated from tracepoints.tp
#include "timestamp.h"

// common
#include <debughelpers.h>

// Qt
#include <QDebug>
#include <QGuiApplication>
#include <QMutexLocker>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QScreen>
#include <QTimer>
#include <QSGTextureProvider>
#include <QtQuick/qsgmaterial.h>
#include <QtQuick/qsgnode.h>

#include <QRunnable>

#include <mir/graphics/texture.h>
#include <mir/graphics/program.h>
#include <mir/graphics/program_factory.h>
#include <array>
#include <sstream>
#include <memory>

namespace qtmir {

namespace {
class MirTextureQSGMaterial;

class MirTextureQSGMaterialShader : public QSGMaterialShader
{
public:
    MirTextureQSGMaterialShader(std::string fragment_source)
        : fragment_source{std::move(fragment_source)}
    {
    }

    char const* vertexShader() const override
    {
        return
            "attribute highp vec4 a_position;\n"
            "attribute mediump vec2 a_texCoord;\n"
            "uniform highp mat4 matrix;\n"
            "varying mediump vec2 v_texCoord;\n"
            "void main() {\n"
            "  gl_Position = matrix * a_position;\n"
            "  v_texCoord = a_texCoord;\n"
            "}";
    }

    char const* fragmentShader() const override
    {
        return fragment_source.c_str();
    }

    char const* const* attributeNames() const override
    {
        static char const* attribs[] = {
            "a_position",
            "a_texCoord",
            nullptr
        };
        return attribs;
    }

    void initialize() override
    {
        QSGMaterialShader::initialize();
        std::array<std::pair<int, char const*>, std::tuple_size<decltype(tex_uniforms)>::value> const
            uniform_specs = {
            std::make_pair(0, "tex"),
            std::make_pair(1, "tex1"),
            std::make_pair(2, "tex2"),
            std::make_pair(3, "tex3")
        };
        for (auto const& uniform_spec : uniform_specs)
        {
            tex_uniforms[uniform_spec.first] = program()->uniformLocation(uniform_spec.second);
        }
        alpha_uniform = program()->uniformLocation("alpha");
        matrix_uniform = program()->uniformLocation("matrix");
    }

    void updateState(RenderState const& state, QSGMaterial* to, QSGMaterial* from) override;

    void deactivate() override
    {
        QSGMaterialShader::deactivate();

        if (last_used_texture)
        {
            // We've finished rendering the scene, so we're *definitely* done.
            last_used_texture->add_syncpoint();
            last_used_texture = nullptr;
        }
    }

    QSGMaterialType const* type() const
    {
        return &type_;
    }

private:
    std::string const fragment_source;
    QSGMaterialType const type_{};

    // Used to insert post-rendering syncpoints.
    std::shared_ptr<mir::graphics::gl::Texture> last_used_texture{};

    std::array<int, 4> tex_uniforms;
    int alpha_uniform;
    int matrix_uniform;
};

std::string build_opaque_fragment_shader_string(
    char const* extension_fragment,
    char const* fragment_fragment)
{
    std::stringstream opaque_fragment;
    opaque_fragment
        << extension_fragment
        << "precision mediump float;\n"
	<< "\n"
        << fragment_fragment
        << "\n"
        <<
        "varying vec2 v_texCoord;\n"
        "void main() {\n"
        "    gl_FragColor = sample_to_rgba(v_texCoord);\n"
        "}\n";

    return opaque_fragment.str();
}

std::string build_alpha_fragment_shader_string(
    char const* extension_fragment,
    char const* fragment_fragment)
{
    std::stringstream alpha_fragment;
    alpha_fragment
        << extension_fragment
        << "precision mediump float;\n"
	<< "\n"
        << fragment_fragment
        << "\n"
        <<
        "varying vec2 v_texCoord;\n"
        "uniform lowp float alpha;\n"
        "void main() {\n"
        "    gl_FragColor = alpha * sample_to_rgba(v_texCoord);\n"
        "}\n";

    return alpha_fragment.str();
}

class QtMirProgram : public mir::graphics::gl::Program
{
public:
    QtMirProgram(
        char const* extension_fragment,
        char const* fragment_fragment)
            : alpha(build_alpha_fragment_shader_string(extension_fragment, fragment_fragment)),
              opaque(build_opaque_fragment_shader_string(extension_fragment, fragment_fragment))
    {
    }

    MirTextureQSGMaterialShader mutable alpha;
    MirTextureQSGMaterialShader mutable opaque;
};

class QSGProgramFactory : public mir::graphics::gl::ProgramFactory
{
public:
    std::unique_ptr<mir::graphics::gl::Program> compile_fragment_shader(
        char const* extension_fragment,
        char const* fragment_fragment) override
    {
        return std::make_unique<QtMirProgram>(extension_fragment, fragment_fragment);
    }
};
QSGProgramFactory programFactory;

class MirTextureQSGMaterial : public QSGMaterial
{
public:
    enum class Opacity
    {
        Alpha,
        Solid
    };

    MirTextureQSGMaterial(Opacity opacity)
        : opacity{opacity}
    {
        setFlag(QSGMaterial::RequiresFullMatrix);
    }

    QSGMaterialType* type() const override
    {
        return const_cast<QSGMaterialType*>(shader->type());
    }

    QSGMaterialShader* createShader() const override
    {
        return shader;
    }

    int compare(QSGMaterial const* other) const override
    {
        /*
         * We don't have to care about matching opacities - Qt guarantees that this will only be called
         * with type() == other->type(), and opaque shaders have a different type to alpha shaders.
         *
         * With that out of the way, two materials are equal iff they're sampling from the same Texture.
         */
        auto const* rhs = static_cast<MirTextureQSGMaterial const*>(other);
        if (buffer < rhs->buffer)
            return -1;
        if (buffer == rhs->buffer)
            return 0;
        return 1;
    }

    void setTextureSource(std::shared_ptr<mir::graphics::gl::Texture> source)
    {
        buffer = std::move(source);
        auto const& programs =
            static_cast<QtMirProgram const&>(
                buffer->shader(programFactory));

        switch (opacity)
        {
        case Opacity::Alpha:
            shader = &programs.alpha;
            break;
        case Opacity::Solid:
            shader = &programs.opaque;
            break;
        }
    }
private:
    friend class MirTextureQSGMaterialShader;
    Opacity const opacity;
    std::shared_ptr<mir::graphics::gl::Texture> buffer;
    MirTextureQSGMaterialShader* shader;
};

void MirTextureQSGMaterialShader::updateState(RenderState const& state, QSGMaterial* to, QSGMaterial* from)
{
    /*
     * The compare() implementation on our QSGMaterial implementation *should*
     * prevent Qt from calling updateState() when the underlying Texture object
     * hasn't changed, and we don't really have any other state to crib from
     * the existing state.
     */
    Q_UNUSED(from);

    if (last_used_texture)
    {
        // We're on to the next draw, so we've definitely finished using the last
        // texture.
        last_used_texture->add_syncpoint();
    }

    last_used_texture = static_cast<MirTextureQSGMaterial*>(to)->buffer;

    for (auto i = 0u; i < tex_uniforms.size() ; ++i)
    {
        if (tex_uniforms[i] != -1)
        {
            program()->setUniformValue(tex_uniforms[i], GL_TEXTURE0 + i);
        }
    }
    last_used_texture->bind();

    if (state.isOpacityDirty() && (alpha_uniform != -1))
    {
        program()->setUniformValue(alpha_uniform, state.opacity());
    }
    if (state.isMatrixDirty())
    {
        program()->setUniformValue(matrix_uniform, state.combinedMatrix());
    }
}

} // namespace {


MirSurfaceItem::MirSurfaceItem(QQuickItem *parent)
    : MirSurfaceItemInterface(parent)
    , m_surface(nullptr)
    , m_window(nullptr)
    , m_lastTouchEvent(nullptr)
    , m_lastFrameNumberRendered(nullptr)
    , m_surfaceWidth(0)
    , m_surfaceHeight(0)
    , m_orientationAngle(nullptr)
    , m_consumesInput(false)
    , m_fillMode(Stretch)
{
    qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::MirSurfaceItem";

    setSmooth(true);
    setFlag(QQuickItem::ItemHasContents, true); //so scene graph will render this item

    m_updateMirSurfaceSizeTimer.setSingleShot(true);
    m_updateMirSurfaceSizeTimer.setInterval(1);
    connect(&m_updateMirSurfaceSizeTimer, &QTimer::timeout, this, &MirSurfaceItem::updateMirSurfaceSize);

    connect(this, &QQuickItem::activeFocusChanged, this, &MirSurfaceItem::updateMirSurfaceActiveFocus);
    connect(this, &QQuickItem::visibleChanged, this, &MirSurfaceItem::updateMirSurfaceExposure);
    connect(this, &QQuickItem::windowChanged, this, &MirSurfaceItem::onWindowChanged);
}

MirSurfaceItem::~MirSurfaceItem()
{
    qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::~MirSurfaceItem - this=" << this;

    setSurface(nullptr);

    delete m_lastTouchEvent;
    delete m_lastFrameNumberRendered;
    delete m_orientationAngle;

    // Belongs to the scene graph thread. Can't delete here.
    // Scene graph should call MirSurfaceItem::releaseResources() or invalidateSceneGraph()
    // delete m_textureProvider;
}

Mir::Type MirSurfaceItem::type() const
{
    if (m_surface) {
        return m_surface->type();
    } else {
        return Mir::UnknownType;
    }
}

Mir::OrientationAngle MirSurfaceItem::orientationAngle() const
{
    if (m_orientationAngle) {
        Q_ASSERT(!m_surface);
        return *m_orientationAngle;
    } else if (m_surface) {
        return m_surface->orientationAngle();
    } else {
        return Mir::Angle0;
    }
}

void MirSurfaceItem::setOrientationAngle(Mir::OrientationAngle angle)
{
    qCDebug(QTMIR_SURFACES, "MirSurfaceItem::setOrientationAngle(%d)", angle);

    if (m_surface) {
        Q_ASSERT(!m_orientationAngle);
        m_surface->setOrientationAngle(angle);
    } else if (!m_orientationAngle) {
        m_orientationAngle = new Mir::OrientationAngle;
        *m_orientationAngle = angle;
        Q_EMIT orientationAngleChanged(angle);
    } else if (*m_orientationAngle != angle) {
        *m_orientationAngle = angle;
        Q_EMIT orientationAngleChanged(angle);
    }
}

QString MirSurfaceItem::name() const
{
    if (m_surface) {
        return m_surface->name();
    } else {
        return QString();
    }
}

bool MirSurfaceItem::live() const
{
    return m_surface && m_surface->live();
}

Mir::ShellChrome MirSurfaceItem::shellChrome() const
{
    return m_surface ? m_surface->shellChrome() : Mir::NormalChrome;
}

QSGNode *MirSurfaceItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)    // called by render thread
{
    QMutexLocker mutexLocker(&m_mutex);

    if (!m_surface) {
        delete oldNode;
        return nullptr;
    }

    auto textures = m_surface->updateTexture();
    if (textures.empty()) {
        delete oldNode;
        return nullptr;
    }

    if (m_surface->numBuffersReadyForCompositor() > 0) {
        QTimer::singleShot(0, this, &MirSurfaceItem::update);
    }

    auto* const node = oldNode ? static_cast<QSGGeometryNode*>(oldNode) : new QSGGeometryNode{};
    auto* const opaque_material =
        [node]()
        {
            if (node->opaqueMaterial())
            {
                return static_cast<MirTextureQSGMaterial*>(node->opaqueMaterial());
            }
            auto const material = new MirTextureQSGMaterial{MirTextureQSGMaterial::Opacity::Solid};
            node->setOpaqueMaterial(material);
            node->setFlag(QSGNode::OwnsMaterial);
            return material;
        }();
    auto* const material =
        [node]()
        {
            if (node->material())
            {
                return static_cast<MirTextureQSGMaterial*>(node->material());
            }
            auto const material = new MirTextureQSGMaterial{MirTextureQSGMaterial::Opacity::Alpha};
            node->setMaterial(material);
            node->setFlag(QSGNode::OwnsMaterial);
            return material;
        }();
    auto* const geometry =
        [node]()
        {
            if (!node->geometry())
            {
                node->setFlag(QSGNode::OwnsGeometry);
                node->setGeometry(new QSGGeometry{QSGGeometry::defaultAttributes_TexturedPoint2D(), 4});
            }
            return node->geometry();
        }();

    // If the surface content has changed since the last render pass, we need to update it.
    if (!m_lastFrameNumberRendered  || (*m_lastFrameNumberRendered != m_surface->currentFrameNumber())) {
        opaque_material->setTextureSource(textures[0].texture);
        material->setTextureSource(textures[0].texture);

        node->markDirty(QSGNode::DirtyMaterial);
    }

    QRectF const newSize{0, 0, width(), height()};
    QRectF const newTextureRect =
        [this](QRectF const& textureSize)
        {
            if (m_fillMode == PadOrCrop) {
                // Sample out of the appropriate subrect of the texture
                auto const clampedWidth = qMin(width(), textureSize.width());
                auto const clampedHeight = qMin(height(), textureSize.height());

                return QRectF{0, 0, clampedWidth / textureSize.width(), clampedHeight / textureSize.height()};
            }
            else {
                // Sample out of the whole texture
                return QRectF{0, 0, 1, 1};
            }
        }(textures[0].extent);

    QSGGeometry::updateTexturedRectGeometry(geometry, newSize, newTextureRect);
    node->markDirty(QSGNode::DirtyGeometry);

    if (!m_lastFrameNumberRendered) {
        m_lastFrameNumberRendered = new unsigned int;
    }
    *m_lastFrameNumberRendered = m_surface->currentFrameNumber();

    return node;
}

void MirSurfaceItem::mousePressEvent(QMouseEvent *event)
{
    auto mousePos = event->localPos().toPoint();
    if (m_consumesInput && m_surface && m_surface->live() && m_surface->inputAreaContains(mousePos)) {
        m_surface->mousePressEvent(event);
    } else {
        event->ignore();
    }
}

void MirSurfaceItem::mouseMoveEvent(QMouseEvent *event)
{
    if (m_consumesInput && m_surface && m_surface->live()) {
        m_surface->mouseMoveEvent(event);
    } else {
        event->ignore();
    }
}

void MirSurfaceItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_consumesInput && m_surface && m_surface->live()) {
        m_surface->mouseReleaseEvent(event);
    } else {
        event->ignore();
    }
}

void MirSurfaceItem::wheelEvent(QWheelEvent *event)
{
    if (m_consumesInput && m_surface && m_surface->live()) {
        m_surface->wheelEvent(event);
    } else {
        event->ignore();
    }
}

void MirSurfaceItem::hoverEnterEvent(QHoverEvent *event)
{
    if (m_consumesInput && m_surface && m_surface->live()) {
        m_surface->hoverEnterEvent(event);
    } else {
        event->ignore();
    }
}

void MirSurfaceItem::hoverLeaveEvent(QHoverEvent *event)
{
    if (m_consumesInput && m_surface && m_surface->live()) {
        m_surface->hoverLeaveEvent(event);
    } else {
        event->ignore();
    }
}

void MirSurfaceItem::hoverMoveEvent(QHoverEvent *event)
{
    if (m_consumesInput && m_surface && m_surface->live()) {
        m_surface->hoverMoveEvent(event);
    } else {
        event->ignore();
    }
}

void MirSurfaceItem::keyPressEvent(QKeyEvent *event)
{
    if (m_consumesInput && m_surface && m_surface->live()) {
        m_surface->keyPressEvent(event);
    } else {
        event->ignore();
    }
}

void MirSurfaceItem::keyReleaseEvent(QKeyEvent *event)
{
    if (m_consumesInput && m_surface && m_surface->live()) {
        m_surface->keyReleaseEvent(event);
    } else {
        event->ignore();
    }
}

QString MirSurfaceItem::appId() const
{
    if (m_surface) {
        return m_surface->appId();
    } else {
        return QStringLiteral("-");
    }
}

void MirSurfaceItem::endCurrentTouchSequence(ulong timestamp)
{
    Q_ASSERT(m_lastTouchEvent);
    Q_ASSERT(m_lastTouchEvent->type != QEvent::TouchEnd);
    Q_ASSERT(m_lastTouchEvent->touchPoints.count() > 0);

    TouchEvent touchEvent = *m_lastTouchEvent;
    touchEvent.timestamp = timestamp;

    // Remove all already released touch points
    int i = 0;
    while (i < touchEvent.touchPoints.count()) {
        if (touchEvent.touchPoints[i].state() == Qt::TouchPointReleased) {
            touchEvent.touchPoints.removeAt(i);
        } else {
            ++i;
        }
    }

    // And release the others one by one as Mir expects one press/release per event
    while (touchEvent.touchPoints.count() > 0) {
        touchEvent.touchPoints[0].setState(Qt::TouchPointReleased);

        touchEvent.updateTouchPointStatesAndType();

        m_surface->touchEvent(touchEvent.modifiers, touchEvent.touchPoints,
                               touchEvent.touchPointStates, touchEvent.timestamp);

        *m_lastTouchEvent = touchEvent;

        touchEvent.touchPoints.removeAt(0);
    }
}

void MirSurfaceItem::validateAndDeliverTouchEvent(int eventType,
            ulong timestamp,
            Qt::KeyboardModifiers mods,
            const QList<QTouchEvent::TouchPoint> &touchPoints,
            Qt::TouchPointStates touchPointStates)
{
    if (eventType == QEvent::TouchBegin && m_lastTouchEvent && m_lastTouchEvent->type != QEvent::TouchEnd) {
        qCWarning(QTMIR_SURFACES) << qPrintable(QStringLiteral("MirSurfaceItem(%1) - Got a QEvent::TouchBegin while "
            "there's still an active/unfinished touch sequence.").arg(appId()));
        // Qt forgot to end the last touch sequence. Let's do it ourselves.
        endCurrentTouchSequence(timestamp);
    }

    m_surface->touchEvent(mods, touchPoints, touchPointStates, timestamp);

    if (!m_lastTouchEvent) {
        m_lastTouchEvent = new TouchEvent;
    }
    m_lastTouchEvent->type = eventType;
    m_lastTouchEvent->timestamp = timestamp;
    m_lastTouchEvent->touchPoints = touchPoints;
    m_lastTouchEvent->touchPointStates = touchPointStates;

    tracepoint(qtmir, touchEventConsume_end, uncompressTimestamp<ulong>(timestamp).count());
}

void MirSurfaceItem::touchEvent(QTouchEvent *event)
{
    tracepoint(qtmir, touchEventConsume_start, uncompressTimestamp<ulong>(event->timestamp()).count());

    bool accepted = processTouchEvent(event->type(),
            event->timestamp(),
            event->modifiers(),
            event->touchPoints(),
            event->touchPointStates());
    event->setAccepted(accepted);
}

bool MirSurfaceItem::processTouchEvent(
        int eventType,
        ulong timestamp,
        Qt::KeyboardModifiers mods,
        const QList<QTouchEvent::TouchPoint> &touchPoints,
        Qt::TouchPointStates touchPointStates)
{

    if (!m_consumesInput || !m_surface || !m_surface->live()) {
        return false;
    }

    if (eventType == QEvent::TouchBegin && !hasTouchInsideInputRegion(touchPoints)) {
        return false;
    }

    validateAndDeliverTouchEvent(eventType, timestamp, mods, touchPoints, touchPointStates);

    return true;
}

bool MirSurfaceItem::hasTouchInsideInputRegion(const QList<QTouchEvent::TouchPoint> &touchPoints)
{
    for (int i = 0; i < touchPoints.count(); ++i) {
        QPoint pos = touchPoints.at(i).pos().toPoint();
        if (m_surface->inputAreaContains(pos)) {
            return true;
        }
    }
    return false;
}

Mir::State MirSurfaceItem::surfaceState() const
{
    if (m_surface) {
        return m_surface->state();
    } else {
        return Mir::UnknownState;
    }
}

void MirSurfaceItem::scheduleMirSurfaceSizeUpdate()
{
    if (!m_updateMirSurfaceSizeTimer.isActive()) {
        m_updateMirSurfaceSizeTimer.start();
    }
}

void MirSurfaceItem::updateMirSurfaceSize()
{
    if (!m_surface || !m_surface->live() || (m_surfaceWidth <= 0 && m_surfaceHeight <= 0)) {
        return;
    }

    // If one dimension is not set, fallback to the current value
    int width = m_surfaceWidth > 0 ? m_surfaceWidth : m_surface->size().width();
    int height = m_surfaceHeight > 0 ? m_surfaceHeight : m_surface->size().height();

    m_surface->resize(width, height);
}

void MirSurfaceItem::updateMirSurfaceExposure()
{
    if (!m_surface || !m_surface->live()) {
        return;
    }

    m_surface->setViewExposure((qintptr)this, isVisible());
}

void MirSurfaceItem::updateMirSurfaceActiveFocus()
{
    if (m_surface && m_surface->live()) {
        m_surface->setViewActiveFocus(qintptr(this), m_consumesInput && hasActiveFocus());
    }
}

void MirSurfaceItem::TouchEvent::updateTouchPointStatesAndType()
{
    touchPointStates = 0;
    for (int i = 0; i < touchPoints.count(); ++i) {
        touchPointStates |= touchPoints.at(i).state();
    }

    if (touchPointStates == Qt::TouchPointReleased) {
        type = QEvent::TouchEnd;
    } else if (touchPointStates == Qt::TouchPointPressed) {
        type = QEvent::TouchBegin;
    } else {
        type = QEvent::TouchUpdate;
    }
}

bool MirSurfaceItem::consumesInput() const
{
    return m_consumesInput;
}

void MirSurfaceItem::setConsumesInput(bool value)
{
    if (m_consumesInput == value) {
        return;
    }

    m_consumesInput = value;
    if (m_consumesInput) {
        setAcceptedMouseButtons(Qt::LeftButton | Qt::MiddleButton | Qt::RightButton |
            Qt::ExtraButton1 | Qt::ExtraButton2 | Qt::ExtraButton3 | Qt::ExtraButton4 |
            Qt::ExtraButton5 | Qt::ExtraButton6 | Qt::ExtraButton7 | Qt::ExtraButton8 |
            Qt::ExtraButton9 | Qt::ExtraButton10 | Qt::ExtraButton11 |
            Qt::ExtraButton12 | Qt::ExtraButton13);
        setAcceptHoverEvents(true);
    } else {
        setAcceptedMouseButtons(Qt::NoButton);
        setAcceptHoverEvents(false);
    }

    updateMirSurfaceActiveFocus();
    Q_EMIT consumesInputChanged(value);
}

unity::shell::application::MirSurfaceInterface* MirSurfaceItem::surface() const
{
    return m_surface;
}

void MirSurfaceItem::setSurface(unity::shell::application::MirSurfaceInterface *unitySurface)
{
    QMutexLocker mutexLocker(&m_mutex);

    auto surface = static_cast<qtmir::MirSurfaceInterface*>(unitySurface);
    qCDebug(QTMIR_SURFACES).nospace() << "MirSurfaceItem::setSurface surface=" << surface;

    if (surface == m_surface) {
        return;
    }

    if (m_surface) {
        disconnect(m_surface, nullptr, this, nullptr);
        m_surface->unregisterView((qintptr)this);
        unsetCursor();
    }

    m_surface = surface;

    if (m_surface) {
        m_surface->registerView((qintptr)this);

        // When a new mir frame gets posted we notify the QML engine that this item needs redrawing,
        // schedules call to updatePaintNode() from the rendering thread
        connect(m_surface, &MirSurfaceInterface::framesPosted, this, &QQuickItem::update);

        connect(m_surface, &MirSurfaceInterface::stateChanged, this, &MirSurfaceItem::surfaceStateChanged);
        connect(m_surface, &MirSurfaceInterface::liveChanged, this, &MirSurfaceItem::liveChanged);
        connect(m_surface, &MirSurfaceInterface::sizeChanged, this, &MirSurfaceItem::onActualSurfaceSizeChanged);
        connect(m_surface, &MirSurfaceInterface::cursorChanged, this, &MirSurfaceItem::setCursor);
        connect(m_surface, &MirSurfaceInterface::shellChromeChanged, this, &MirSurfaceItem::shellChromeChanged);

        Q_EMIT typeChanged(m_surface->type());
        Q_EMIT liveChanged(true);
        Q_EMIT surfaceStateChanged(m_surface->state());

        updateMirSurfaceSize();
        setImplicitSize(m_surface->size().width(), m_surface->size().height());
        updateMirSurfaceExposure();

        // Qt::ArrowCursor is the default when no cursor has been explicitly set, so no point forwarding it.
        if (m_surface->cursor().shape() != Qt::ArrowCursor) {
            setCursor(m_surface->cursor());
        }

        if (m_orientationAngle) {
            m_surface->setOrientationAngle(*m_orientationAngle);
            connect(m_surface, &MirSurfaceInterface::orientationAngleChanged, this, &MirSurfaceItem::orientationAngleChanged);
            delete m_orientationAngle;
            m_orientationAngle = nullptr;
        } else {
            connect(m_surface, &MirSurfaceInterface::orientationAngleChanged, this, &MirSurfaceItem::orientationAngleChanged);
            Q_EMIT orientationAngleChanged(m_surface->orientationAngle());
        }

        updateMirSurfaceActiveFocus();
    }

    update();

    Q_EMIT surfaceChanged(m_surface);
}

void MirSurfaceItem::onCompositorSwappedBuffers()
{
    if (Q_LIKELY(m_surface)) {
        m_surface->onCompositorSwappedBuffers();
    }
}

void MirSurfaceItem::onWindowChanged(QQuickWindow *window)
{
    if (m_window) {
        disconnect(m_window, nullptr, this, nullptr);
    }
    m_window = window;
    if (m_window) {
        connect(m_window, &QQuickWindow::frameSwapped, this, &MirSurfaceItem::onCompositorSwappedBuffers,
                Qt::DirectConnection);
    }
}

int MirSurfaceItem::surfaceWidth() const
{
    return m_surfaceWidth;
}

void MirSurfaceItem::setSurfaceWidth(int value)
{
    if (value != m_surfaceWidth) {
        m_surfaceWidth = value;
        scheduleMirSurfaceSizeUpdate();
        Q_EMIT surfaceWidthChanged(value);
    }
}

void MirSurfaceItem::onActualSurfaceSizeChanged(QSize size)
{
    setImplicitSize(size.width(), size.height());
}

int MirSurfaceItem::surfaceHeight() const
{
    return m_surfaceHeight;
}

void MirSurfaceItem::setSurfaceHeight(int value)
{
    if (value != m_surfaceHeight) {
        m_surfaceHeight = value;
        scheduleMirSurfaceSizeUpdate();
        Q_EMIT surfaceHeightChanged(value);
    }
}

MirSurfaceItem::FillMode MirSurfaceItem::fillMode() const
{
    return m_fillMode;
}

void MirSurfaceItem::setFillMode(FillMode value)
{
    if (m_fillMode != value) {
        m_fillMode = value;
        Q_EMIT fillModeChanged(m_fillMode);
    }
}

} // namespace qtmir

#include "mirsurfaceitem.moc"
