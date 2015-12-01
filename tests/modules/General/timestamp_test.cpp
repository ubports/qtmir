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

class TimestampTest: public ::testing::Test
{
protected:
    virtual void SetUp() {
        resetStartTime(std::chrono::nanoseconds(0));
    }
};

TEST_F(TimestampTest, TestCompressAndUncompress)
{
    using namespace testing;

    std::chrono::time_point<std::chrono::system_clock> now;
    now = std::chrono::system_clock::now();
    auto original_timestamp = now.time_since_epoch();

    qtmir::Timestamp addToTimestamp(0);

    for (int i = 0; i < 100; i++) {
        auto timestamp = original_timestamp + addToTimestamp;

        qtmir::Timestamp compressedTimestamp = qtmir::compressTimestamp<qtmir::Timestamp>(timestamp);

        EXPECT_EQ(addToTimestamp, compressedTimestamp);
        EXPECT_EQ(qtmir::uncompressTimestamp<qtmir::Timestamp>(compressedTimestamp), timestamp);

        addToTimestamp += std::chrono::seconds(1);
    }
}

TEST_F(TimestampTest, TestOverflowWhenExceeding32bitCompression)
{
    using namespace testing;

    std::chrono::time_point<std::chrono::system_clock> now;
    now = std::chrono::system_clock::now();
    auto timestamp = now.time_since_epoch();

    typedef std::chrono::duration<quint32, std::milli> Timestamp32bit;

    // Do first compression. This will result in qield of 0 as seen in TestCompressUncompress
    auto compressedTimestamp = qtmir::compressTimestamp<Timestamp32bit>(timestamp);

    // Add the quint32 limit +1 to get an overflow when we compress the timestamp
    timestamp += Timestamp32bit::max() + std::chrono::nanoseconds(1);
    
    compressedTimestamp = qtmir::compressTimestamp<Timestamp32bit>(timestamp);

    EXPECT_EQ(0, compressedTimestamp.count());
    // ensure the uncompression will yields the original timestamp
    EXPECT_EQ(qtmir::uncompressTimestamp<Timestamp32bit>(compressedTimestamp), timestamp);
}