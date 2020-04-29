/*
 * Copyright (C) 2020 UBports Foundation
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

#include "urlbuilderdispatcher.h"
#include "logging.h"

#include <QDebug>
#include <QUrl>

#define DEBUG_MSG qCDebug(QTMIR_URLBUILDER).nospace().noquote() << __func__

namespace qtmir
{
URLBuilderDispatcher::URLBuilderDispatcher(std::shared_ptr<QPlatformServices> const services)
    : mLastRemaining{0},
      mCurrentUrl{},
      mServices(services)
{}

void URLBuilderDispatcher::urlInput(uint32_t remaining, std::string const &url) {

    QByteArray bytes = QByteArray::fromStdString(url);
    DEBUG_MSG << "(" << remaining << ", \"" << bytes << "\")";

    if (remaining > mLastRemaining) {
        mCurrentUrl = "";
    } 
    
    if (remaining == 0) {
        mLastRemaining = 0;
        mCurrentUrl.append(bytes);
        QUrl myUrl = QUrl::fromEncoded(mCurrentUrl);
        mServices->openUrl(myUrl);
        mCurrentUrl = QByteArray();
        return;
    } else {
        mCurrentUrl.append(bytes);
        mLastRemaining = remaining;
        return;
    }
}
}

