/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QCoreApplication>
#include <QDebug>

using namespace qtmir;

TEST(TimestampTest, TestCompressAndUncompress)
{
    using namespace testing;

    int argc = 0;
    QCoreApplication app(argc, NULL);

    std::chrono::time_point<std::chrono::system_clock> now;
    now = std::chrono::system_clock::now();
    auto original_timestamp = now.time_since_epoch();

    std::chrono::nanoseconds addToTimestamp(0);
    for (int i = 0; i < 100; i++) {
        auto timestamp = original_timestamp + addToTimestamp;

        ulong compressedTimestamp = qtmir::compressTimestamp<ulong>(timestamp);

        EXPECT_EQ(addToTimestamp.count(), compressedTimestamp);
        EXPECT_EQ(qtmir::uncompressTimestamp<ulong>(compressedTimestamp), timestamp);

        addToTimestamp += std::chrono::milliseconds(1);
    }
}

TEST(TimestampTest, TestOverflowWhenExceeding32bitCompression)
{
    using namespace testing;

    int argc = 0;
    QCoreApplication app(argc, NULL);

    std::chrono::time_point<std::chrono::system_clock> now;
    now = std::chrono::system_clock::now();
    auto timestamp = now.time_since_epoch();

    // Do first compression. This will result in qield of 0 as seen in TestCompressUncompress
    quint32 compressedTimestamp = qtmir::compressTimestamp<quint32>(timestamp);

    // Add the quint32 limit +1 to get an overflow when we compress the timestamp
    timestamp += std::chrono::nanoseconds(std::numeric_limits<quint32>::max()) + std::chrono::nanoseconds(1);
    compressedTimestamp = qtmir::compressTimestamp<quint32>(timestamp);

    EXPECT_EQ(0, compressedTimestamp);
    // ensure the uncompression will yields the original timestamp
    EXPECT_EQ(qtmir::uncompressTimestamp<quint32>(compressedTimestamp), timestamp);
}