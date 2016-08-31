/*
 * Copyright (C) 2014-2016 Canonical, Ltd.
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

#ifndef QTMIR_CLIPBOARD_H
#define QTMIR_CLIPBOARD_H

#include <qpa/qplatformclipboard.h>

#include <QMimeData>
#include <QScopedPointer>

namespace com {
    namespace ubuntu {
        namespace content {
            class Hub;
        }
    }
}

class QDBusPendingCallWatcher;

namespace qtmir {

class Clipboard : public QObject, public QPlatformClipboard
{
    Q_OBJECT
public:
    Clipboard();
    virtual ~Clipboard();

    // QPlatformClipboard methods.
    QMimeData* mimeData(QClipboard::Mode mode = QClipboard::Clipboard) override;
    void setMimeData(QMimeData* data, QClipboard::Mode mode = QClipboard::Clipboard) override;
    bool supportsMode(QClipboard::Mode mode) const override;
    bool ownsMode(QClipboard::Mode mode) const override;

private:
    void updateMimeData();
    void requestMimeData();

    QScopedPointer<QMimeData> m_mimeData;

    enum {
        OutdatedClipboard, // Our mimeData is outdated, need to fetch latest from ContentHub
        SyncingClipboard, // Our mimeData is outdated and we are waiting for ContentHub to reply with the latest paste
        SyncedClipboard // Our mimeData is in sync with what ContentHub has
    } m_clipboardState{OutdatedClipboard};

    com::ubuntu::content::Hub *m_contentHub;

    QDBusPendingCallWatcher *m_pasteReply{nullptr};
};

} // namespace qtmir

#endif // QTMIR_CLIPBOARD_H
