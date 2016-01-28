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

namespace qtmir {

class FakeApplicationInfo : public ApplicationInfo
{
public:
    FakeApplicationInfo() : ApplicationInfo()
        , m_appId("foo-app")
    {}

    QString appId() const override { return m_appId; }
    QString name() const override { return QString(); }
    QString comment() const override { return QString(); }
    QUrl icon() const override { return QUrl(); }
    QString splashTitle() const override { return QString(); }
    QUrl splashImage() const override { return QUrl(); }
    bool splashShowHeader() const override { return false; }
    QString splashColor() const override { return QString(); }
    QString splashColorHeader() const override { return QString(); }
    QString splashColorFooter() const override { return QString(); }
    Qt::ScreenOrientations supportedOrientations() const override { return Qt::PortraitOrientation; }
    bool rotatesWindowContents() const override { return false; }
    bool isTouchApp() const override { return true; }

    QString m_appId;
};

} // namespace qtmir

#endif // FAKE_APPLICATION_INFO_H
