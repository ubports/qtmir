/*
 * Copyright (C) 2015,2016 Canonical, Ltd.
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

#ifndef FAKE_APPLICATION_INFO_H
#define FAKE_APPLICATION_INFO_H

#include <Unity/Application/applicationinfo.h>

namespace qtmir {

class FakeApplicationInfo : public ApplicationInfo
{
    Q_OBJECT
public:
    FakeApplicationInfo(const QString &appId);
    FakeApplicationInfo();
    virtual ~FakeApplicationInfo();

    QString appId() const override;
    QString name() const override;
    QString comment() const override;
    QUrl icon() const override;
    QString splashTitle() const override;
    QUrl splashImage() const override;
    bool splashShowHeader() const override;
    QString splashColor() const override;
    QString splashColorHeader() const override;
    QString splashColorFooter() const override;
    Qt::ScreenOrientations supportedOrientations() const override;
    bool rotatesWindowContents() const override;
    bool isTouchApp() const override;

    QString m_appId;
};

} // namespace qtmir

#endif // FAKE_APPLICATION_INFO_H
