/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#ifndef QTMIR_TIMESTAMP_H
#define QTMIR_TIMESTAMP_H

#include <QtCore/qglobal.h>
#include <chrono>

namespace qtmir {

// Converts a mir timestamp (in nanoseconds) to a timestamp in milliseconds.
// Qt system events only work with ulong timestamps, so we truncate the result by using time since "first call"
ulong compressTimestamp(qint64 timestamp);

// "Re-inflate" a truncated timestamp.
std::chrono::nanoseconds uncompressTimestamp(ulong timestamp);

}

#endif // QTMIR_TIMESTAMP_H
