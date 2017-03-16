/*
 * Copyright (C) 2013-2017 Canonical, Ltd.
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

#include "plugin.h"
#include "mirserverintegration.h"

QPlatformIntegration *MirServerIntegrationPlugin::create(const QString &system, const QStringList &/*paramList*/)
{
    if (system.toLower() == QLatin1String("mirserver"))
        return new MirServerIntegration;
    return 0;
}
