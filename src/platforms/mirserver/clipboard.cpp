/*
 * Copyright (C) 2014,2016 Canonical, Ltd.
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

#include "clipboard.h"
#include "shelluuid.h"

#include <QDBusPendingCallWatcher>
#include <QSignalBlocker>

// content-hub
#include <com/ubuntu/content/hub.h>

// get this cumbersome nested namespace out of the way
using namespace com::ubuntu::content;

using namespace qtmir;

Clipboard::Clipboard()
    : QObject(nullptr)
    , m_mimeData(new QMimeData)
    , m_contentHub(Hub::Client::instance())
{
    connect(m_contentHub, &Hub::pasteboardChanged, this, [this]() {
        if (m_clipboardState == Clipboard::SyncedClipboard) {
            m_clipboardState = Clipboard::OutdatedClipboard;
            requestMimeData();
        }
    });

    requestMimeData();
}

Clipboard::~Clipboard()
{
}

QMimeData* Clipboard::mimeData(QClipboard::Mode mode)
{
    if (mode != QClipboard::Clipboard)
        return nullptr;

    return m_mimeData.data();
}

void Clipboard::setMimeData(QMimeData* mimeData, QClipboard::Mode mode)
{
    if (mode == QClipboard::Clipboard && mimeData != nullptr) {
        QDBusPendingCall reply = m_contentHub->createPaste(ShellUuId::toString(), *mimeData);

        // Don't care whether it succeeded
        // But I have to keep a QDBusPendingCall instance around (such as this watcher) until
        // the call is finished otherwise the QDBusPendingCall destructor will cancel the call
        // if it's still ongoing.
        auto *watcher = new QDBusPendingCallWatcher(reply, this);
        connect(watcher, &QDBusPendingCallWatcher::finished,
                watcher, &QObject::deleteLater);

        m_mimeData.reset(mimeData);
        m_clipboardState = SyncedClipboard;
        emitChanged(QClipboard::Clipboard);
    }
}

bool Clipboard::supportsMode(QClipboard::Mode mode) const
{
    return mode == QClipboard::Clipboard;
}

bool Clipboard::ownsMode(QClipboard::Mode mode) const
{
    Q_UNUSED(mode);
    return false;
}

void Clipboard::updateMimeData()
{
    m_mimeData.reset(m_contentHub->latestPaste(ShellUuId::toString()));
    m_clipboardState = SyncedClipboard;
    emitChanged(QClipboard::Clipboard);
}

void Clipboard::requestMimeData()
{
    QDBusPendingCall reply = m_contentHub->requestLatestPaste(ShellUuId::toString());
    m_clipboardState = SyncingClipboard;

    m_pasteReply = new QDBusPendingCallWatcher(reply, this);
    connect(m_pasteReply, &QDBusPendingCallWatcher::finished,
            this, [this]() {
        m_mimeData.reset(m_contentHub->paste(*m_pasteReply));
        m_clipboardState = SyncedClipboard;
        m_pasteReply->deleteLater();
        m_pasteReply = nullptr;
        emitChanged(QClipboard::Clipboard);
    });
}
