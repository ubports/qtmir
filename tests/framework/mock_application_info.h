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

namespace qtmir
{

struct MockApplicationInfo : public qtmir::ApplicationInfo
{
    MockApplicationInfo(const QString &appId);
    virtual ~MockApplicationInfo();

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

} // namespace qtmir

#endif // MOCK_APPLICATION_INFO_H
