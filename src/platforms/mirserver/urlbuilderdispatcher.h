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

#pragma once

#include <stdint.h>
#include <memory>

#include <QByteArray>
#include <qpa/qplatformservices.h>
#include <QLoggingCategory>

#include "services.h"

// The QtWayland url_open IPC call will split all URLs into 128-byte chunks then
// call the event via Wayland IPC. This object un-splits the URL and sends it
// to URL-Dispatcher once it receives the last chunk (denoted by 'remaining'
// reaching zero).

namespace qtmir
{

class URLBuilderDispatcher {
public:
    URLBuilderDispatcher(std::shared_ptr<QPlatformServices> const);

    void urlInput(uint32_t, std::string const&);

private:
    uint32_t mLastRemaining;
    QByteArray mCurrentUrl;
    std::shared_ptr<QPlatformServices> mServices;
};
}
