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

#include <Unity/Application/desktopfilereader.h>

namespace qtmir {

class FakeDesktopFileReader : public qtmir::DesktopFileReader
{
public:
    FakeDesktopFileReader();
    virtual ~FakeDesktopFileReader();

    QString file() const override;
    QString appId() const override;
    QString name() const override;
    QString comment() const override;
    QString icon() const override;
    QString exec() const override;
    QString path() const override;
    QString stageHint() const override;
    QString splashTitle() const override;
    QString splashImage() const override;
    QString splashShowHeader() const override;
    QString splashColor() const override;
    QString splashColorHeader() const override;
    QString splashColorFooter() const override;
    Qt::ScreenOrientations supportedOrientations() const override;
    bool rotatesWindowContents() const override;
    bool loaded() const override;

    QString m_appId;
};

} // namespace qtmir

#endif // FAKE_DESKTOPFILEREADER_H
