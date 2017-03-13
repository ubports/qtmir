/*
 * Copyright (C) 2013-2017 Canonical, Ltd.
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

#ifndef NATIVEINTEGRATION_H
#define NATIVEINTEGRATION_H

// qt
#include <QSharedPointer>
#include <qpa/qplatformnativeinterface.h>

#include <memory>

// local
class QMirServer;

// mir
namespace qtmir { class PromptSessionManager; }
namespace mir { namespace shell { class PersistentSurfaceStore; }}

class NativeInterface : public QPlatformNativeInterface
{
    Q_OBJECT
public:
    NativeInterface(QMirServer *);

    void *nativeResourceForIntegration(const QByteArray &resource) override;

    QVariantMap windowProperties(QPlatformWindow *window) const override;
    QVariant windowProperty(QPlatformWindow *window, const QString &name) const override;
    QVariant windowProperty(QPlatformWindow *window, const QString &name, const QVariant &defaultValue) const override;
    void setWindowProperty(QPlatformWindow *window, const QString &name, const QVariant &value) override;

    std::shared_ptr<qtmir::PromptSessionManager> thePromptSessionManager() const;
    std::shared_ptr<mir::shell::PersistentSurfaceStore> thePersistentSurfaceStore() const;

private:
    QMirServer *m_qMirServer;
};

#endif // NATIVEINTEGRATION_H
