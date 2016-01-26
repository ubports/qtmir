/*
 * Copyright (C) 2014,2015 Canonical, Ltd.
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
 *
 */

#include "applicationinfo.h"

namespace qtmir
{
namespace upstart
{

ApplicationInfo::ApplicationInfo(const QString &appId, std::shared_ptr<Ubuntu::AppLaunch::Application::Info> info)
    : qtmir::ApplicationInfo(),
      m_appId(appId),
      m_info(info)
{
}

QString ApplicationInfo::appId() const
{
    if (m_info) {
        return m_appId;
    } else {
        return QString();
    }
}

QString ApplicationInfo::name() const
{
    if (m_info) {
        return QString::fromStdString(m_info->name().value());
    } else {
        return QString();
    }
}

QString ApplicationInfo::comment() const
{
    if (m_info) {
        return QString::fromStdString(m_info->description().value());
    } else {
        return QString();
    }
}

QUrl ApplicationInfo::icon() const
{
    if (m_info) {
        return QUrl::fromLocalFile(QString::fromStdString(m_info->iconPath().value()));
    } else {
        return QUrl();
    }
}

QString ApplicationInfo::splashTitle() const
{
    if (m_info) {
        return QString::fromStdString(m_info->splash().title.value());
    } else {
        return QString();
    }
}

QUrl ApplicationInfo::splashImage() const
{
    if (m_info) {
        return QUrl::fromLocalFile(QString::fromStdString(m_info->splash().image.value()));
    } else {
        return QUrl();
    }
}

bool ApplicationInfo::splashShowHeader() const
{
    if (m_info) {
        return m_info->splash().showHeader.value();
    } else {
        return false;
    }
}

QString ApplicationInfo::splashColor() const
{
    if (m_info) {
        return QString::fromStdString(m_info->splash().backgroundColor.value());
    } else {
        return QString();
    }
}

QString ApplicationInfo::splashColorHeader() const
{
    if (m_info) {
        return QString::fromStdString(m_info->splash().headerColor.value());
    } else {
        return QString();
    }
}

QString ApplicationInfo::splashColorFooter() const
{
    if (m_info) {
        return QString::fromStdString(m_info->splash().footerColor.value());
    } else {
        return QString();
    }
}

Qt::ScreenOrientations ApplicationInfo::supportedOrientations() const
{
    Ubuntu::AppLaunch::Application::Info::Orientations orientations;
    if (m_info) {
        orientations = m_info->supportedOrientations();
    }
    Qt::ScreenOrientations response = Qt::PrimaryOrientation;
    if (orientations.portrait)
        response |= Qt::PortraitOrientation;
    if (orientations.landscape)
        response |= Qt::LandscapeOrientation;
    if (orientations.invertedPortrait)
        response |= Qt::InvertedPortraitOrientation;
    if (orientations.invertedLandscape)
        response |= Qt::InvertedLandscapeOrientation;
    return response;
}

bool ApplicationInfo::rotatesWindowContents() const
{
    if (m_info) {
        return m_info->rotatesWindowContents().value();
    } else {
        return false;
    }
}


bool ApplicationInfo::isTouchApp() const
{
    if (m_info) {
        return m_info->ubuntuLifecycle().value();
    } else {
        return true;
    }
}

} // namespace upstart
} // namespace qtmir
