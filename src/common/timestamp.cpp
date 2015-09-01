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

#include "timestamp.h"
	
#include <QCoreApplication>
#include <QVariant>

namespace qtmir {

namespace {

qint64 resetStartTime(qint64 timestamp)
{
    if (qApp) {
        qApp->setProperty("appStartTime", timestamp);
    }
    return timestamp;
}

qint64 getStartTime_nsec(qint64 timestamp)
{
    if (!qApp) return 0;
    QVariant vData = qApp->property("appStartTime");
	if (!vData.isValid()) {
        return resetStartTime(timestamp);
	}
	return vData.value<qint64>();
}

}

ulong compressTimestamp(qint64 timestamp)
{	
	qint64 startTime = getStartTime_nsec(timestamp)/1000000;
	qint64 result = timestamp/1000000 - startTime;
	if (result > (qint64)std::numeric_limits<ulong>::max()) {
		startTime = resetStartTime(timestamp)/1000000;
		result = timestamp/1000000 - startTime;
	}

	return result;
}

// "Re-inflate" a truncated timestamp.
std::chrono::nanoseconds uncompressTimestamp(ulong timestamp)
{
	qint64 startTime = getStartTime_nsec(timestamp)/1000000;
	qint64 result = qint64(timestamp)*1000000 + startTime*1000000;
	return std::chrono::nanoseconds(result);
}

}
