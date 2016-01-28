/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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

#ifndef MOCK_APPLICATION_INFO_H
#define MOCK_APPLICATION_INFO_H

#include <Unity/Application/applicationinfo.h>

#include <gmock/gmock.h>

namespace testing
{
struct MockApplicationInfo : public qtmir::ApplicationInfo
{
    MockApplicationInfo(const QString &appId)
        : ApplicationInfo()
    {
        using namespace ::testing;

        ON_CALL(*this, appId()).WillByDefault(Return(appId));
        ON_CALL(*this, name()).WillByDefault(Return(QString()));
        ON_CALL(*this, comment()).WillByDefault(Return(QString()));
        ON_CALL(*this, icon()).WillByDefault(Return(QUrl()));
        ON_CALL(*this, splashTitle()).WillByDefault(Return(QString()));
        ON_CALL(*this, splashImage()).WillByDefault(Return(QUrl()));
        ON_CALL(*this, splashShowHeader()).WillByDefault(Return(false));
        ON_CALL(*this, splashColor()).WillByDefault(Return(QString()));
        ON_CALL(*this, splashColorHeader()).WillByDefault(Return(QString()));
        ON_CALL(*this, splashColorFooter()).WillByDefault(Return(QString()));
        ON_CALL(*this, supportedOrientations()).WillByDefault(Return(Qt::PrimaryOrientation));
        ON_CALL(*this, rotatesWindowContents()).WillByDefault(Return(false));
        ON_CALL(*this, isTouchApp()).WillByDefault(Return(true));
    }

    MOCK_CONST_METHOD0(appId, QString());
    MOCK_CONST_METHOD0(name, QString());
    MOCK_CONST_METHOD0(comment, QString());
    MOCK_CONST_METHOD0(icon, QUrl());
    MOCK_CONST_METHOD0(splashTitle, QString());
    MOCK_CONST_METHOD0(splashImage, QUrl());
    MOCK_CONST_METHOD0(splashShowHeader, bool());
    MOCK_CONST_METHOD0(splashColor, QString());
    MOCK_CONST_METHOD0(splashColorHeader, QString());
    MOCK_CONST_METHOD0(splashColorFooter, QString());
    MOCK_CONST_METHOD0(supportedOrientations, Qt::ScreenOrientations());
    MOCK_CONST_METHOD0(rotatesWindowContents, bool());
    MOCK_CONST_METHOD0(isTouchApp, bool());
};
}

#endif // MOCK_APPLICATION_INFO_H
