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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "urlbuilderdispatcher.h"
#include "mock_services.h"

#include <QUrl>

using namespace ::testing;

class URLBuilderDispatcherTest : public ::testing::Test
{
};

TEST_F(URLBuilderDispatcherTest, SinglePartURL)
{
    auto services = std::make_shared<MockServices>();
    auto urlbuilder = qtmir::URLBuilderDispatcher{services};

    EXPECT_CALL(*services, openUrl(QUrl("url://url"))).Times(1);
    urlbuilder.urlInput(0, "url://url");
}

// Ensure that multiple single-part URLs are dispatched
TEST_F(URLBuilderDispatcherTest, MultiSinglePart)
{
    auto services = std::make_shared<MockServices>();
    auto urlbuilder = qtmir::URLBuilderDispatcher{services};

    EXPECT_CALL(*services, openUrl(QUrl("url://url"))).Times(1);
    urlbuilder.urlInput(0, "url://url");

    EXPECT_CALL(*services, openUrl(QUrl("url://my-new-neat-url"))).Times(1);
    urlbuilder.urlInput(0, "url://my-new-neat-url");
}

// Ensure that a single multi-part URL is dispatched
TEST_F(URLBuilderDispatcherTest, MultiPartURL)
{
    auto services = std::make_shared<MockServices>();
    auto urlbuilder = qtmir::URLBuilderDispatcher{services};

    EXPECT_CALL(*services, openUrl(QUrl("url://url-that-continues"))).Times(1);
    urlbuilder.urlInput(2, "url://url");
    urlbuilder.urlInput(1, "-that");
    urlbuilder.urlInput(0, "-continues");
}

// Ensure that a single URL with percent encoding is dispatched
TEST_F(URLBuilderDispatcherTest, PercentEncoding)
{
    auto services = std::make_shared<MockServices>();
    auto urlbuilder = qtmir::URLBuilderDispatcher{services};

    EXPECT_CALL(*services, openUrl(QUrl("url://url%20-that-%26going"))).Times(1);
    urlbuilder.urlInput(2, "url://url%20");
    urlbuilder.urlInput(1, "-that");
    urlbuilder.urlInput(0, "%26going");
}

// Ensure that multiple multi-part URLs are dispatched
TEST_F(URLBuilderDispatcherTest, MultiMultiPart)
{
    auto services = std::make_shared<MockServices>();
    auto urlbuilder = qtmir::URLBuilderDispatcher{services};

    EXPECT_CALL(*services, openUrl(QUrl("url://url-that-continues"))).Times(1);
    urlbuilder.urlInput(2, "url://url");
    urlbuilder.urlInput(1, "-that");
    urlbuilder.urlInput(0, "-continues");

    EXPECT_CALL(*services, openUrl(QUrl("url://this-is-a-url"))).Times(1);
    urlbuilder.urlInput(2, "url://this-is");
    urlbuilder.urlInput(1, "-a");
    urlbuilder.urlInput(0, "-url");
}

// Ensure that multiple single or multi-part URLs are dispatched
TEST_F(URLBuilderDispatcherTest, ManyURLs)
{
    auto services = std::make_shared<MockServices>();
    auto urlbuilder = qtmir::URLBuilderDispatcher{services};

    EXPECT_CALL(*services, openUrl(QUrl("url://url-that-continues"))).Times(1);
    urlbuilder.urlInput(2, "url://url");
    urlbuilder.urlInput(1, "-that");
    urlbuilder.urlInput(0, "-continues");

    EXPECT_CALL(*services, openUrl(QUrl("url://url"))).Times(1);
    urlbuilder.urlInput(0, "url://url");

    EXPECT_CALL(*services, openUrl(QUrl("url://this-is-a-url"))).Times(1);
    urlbuilder.urlInput(2, "url://this-is");
    urlbuilder.urlInput(1, "-a");
    urlbuilder.urlInput(0, "-url");

    EXPECT_CALL(*services, openUrl(QUrl("url://my-new-neat-url"))).Times(1);
    urlbuilder.urlInput(0, "url://my-new-neat-url");
}
