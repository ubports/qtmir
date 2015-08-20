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

#ifndef FAKE_DESKTOPFILEREADER_H
#define FAKE_DESKTOPFILEREADER_H

namespace qtmir {

class FakeDesktopFileReader : public qtmir::DesktopFileReader
{
public:
    FakeDesktopFileReader() : DesktopFileReader()
        , m_appId("foo-app")
    {}

    QString file() const override { return QString(); }
    QString appId() const override { return m_appId; }
    QString name() const override { return QString(); }
    QString comment() const override { return QString(); }
    QString icon() const override { return QString(); }
    QString exec() const override { return QString(); }
    QString path() const override { return QString(); }
    QString stageHint() const override { return QString(); }
    QString splashTitle() const override { return QString(); }
    QString splashImage() const override { return QString(); }
    QString splashShowHeader() const override { return QString(); }
    QString splashColor() const override { return QString(); }
    QString splashColorHeader() const override { return QString(); }
    QString splashColorFooter() const override { return QString(); }
    Qt::ScreenOrientations supportedOrientations() const override { return Qt::PortraitOrientation; }
    bool rotatesWindowContents() const override { return false; }
    bool loaded() const override { return true; }

    QString m_appId;
};

} // namespace qtmir

#endif // FAKE_DESKTOPFILEREADER_H
