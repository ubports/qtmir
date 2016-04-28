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

#include "mock_application_info.h"

namespace qtmir
{

MockApplicationInfo::MockApplicationInfo(const QString &appId)
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

MockApplicationInfo::~MockApplicationInfo()
{
}

} // namespace qtmir
