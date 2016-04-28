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

#include "fake_application_info.h"

#include <QDebug>

namespace qtmir
{

FakeApplicationInfo::FakeApplicationInfo(const QString &appId)
    : ApplicationInfo()
    , m_appId(appId)
{
}

FakeApplicationInfo::FakeApplicationInfo()
	: ApplicationInfo()
	, m_appId("foo-app")
{
}

FakeApplicationInfo::~FakeApplicationInfo()
{
}

QString FakeApplicationInfo::appId() const { return m_appId; }

QString FakeApplicationInfo::name() const { return QString(); }

QString FakeApplicationInfo::comment() const { return QString(); }

QUrl FakeApplicationInfo::icon() const { return QUrl(); }

QString FakeApplicationInfo::splashTitle() const { return QString(); }

QUrl FakeApplicationInfo::splashImage() const { return QUrl(); }

bool FakeApplicationInfo::splashShowHeader() const { return false; }

QString FakeApplicationInfo::splashColor() const { return QString(); }

QString FakeApplicationInfo::splashColorHeader() const { return QString(); }

QString FakeApplicationInfo::splashColorFooter() const { return QString(); }

Qt::ScreenOrientations FakeApplicationInfo::supportedOrientations() const { return Qt::PortraitOrientation; }

bool FakeApplicationInfo::rotatesWindowContents() const { return false; }

bool FakeApplicationInfo::isTouchApp() const { return true; }

} // namespace qtmir
