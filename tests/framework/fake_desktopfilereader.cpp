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

#include "fake_desktopfilereader.h"

#include <QDebug>

namespace qtmir
{

FakeDesktopFileReader::FakeDesktopFileReader(const QString &appId)
    : DesktopFileReader()
    , m_appId(appId)
{
}

FakeDesktopFileReader::FakeDesktopFileReader()
	: DesktopFileReader()
	, m_appId("foo-app")
{
}

FakeDesktopFileReader::~FakeDesktopFileReader()
{
}

QString FakeDesktopFileReader::file() const { return m_appId + ".desktop"; }

QString FakeDesktopFileReader::appId() const { return m_appId; }

QString FakeDesktopFileReader::name() const { return QString(); }

QString FakeDesktopFileReader::comment() const { return QString(); }

QString FakeDesktopFileReader::icon() const { return QString(); }

QString FakeDesktopFileReader::exec() const { return QString(); }

QString FakeDesktopFileReader::path() const { return QString(); }

QString FakeDesktopFileReader::stageHint() const { return QString(); }

QString FakeDesktopFileReader::splashTitle() const { return QString(); }

QString FakeDesktopFileReader::splashImage() const { return QString(); }

QString FakeDesktopFileReader::splashShowHeader() const { return QString(); }

QString FakeDesktopFileReader::splashColor() const { return QString(); }

QString FakeDesktopFileReader::splashColorHeader() const { return QString(); }

QString FakeDesktopFileReader::splashColorFooter() const { return QString(); }

Qt::ScreenOrientations FakeDesktopFileReader::supportedOrientations() const { return Qt::PortraitOrientation; }

bool FakeDesktopFileReader::rotatesWindowContents() const { return false; }

bool FakeDesktopFileReader::isTouchApp() const { return true; }

bool FakeDesktopFileReader::loaded() const { return true; }

} // namespace qtmir
