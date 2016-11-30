/*
 * Copyright (C) 2016 Canonical, Ltd.
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

#ifndef APPNOTIFIER_H
#define APPNOTIFIER_H

#include <QObject>
#include <miral/application_info.h>

namespace qtmir {

class AppNotifier : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void appAdded(const ::miral::ApplicationInfo &app);
    void appRemoved(const ::miral::ApplicationInfo &app);
    void appCreatedWindow(const ::miral::ApplicationInfo &app);
};

} // namespace qtmir

Q_DECLARE_METATYPE(miral::ApplicationInfo)

#endif // APPNOTIFIER_H
