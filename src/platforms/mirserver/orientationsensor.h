/*
 * Copyright (C) 2019 UBports
 * Authors: Marius Gripsgard <marius@ubports.com>
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

#pragma once

#include <QObject>
#include <QtSensors/QOrientationReading>
#include <QtDBus/QDBusInterface>

class OrientationSensor : public QObject
{
     Q_OBJECT
public:
    OrientationSensor(QObject *parent = 0);

    void start();
    void stop();
    void enable();

    // To make it testable
    static bool skipDBusRegistration;
    bool enabled();

Q_SIGNALS:
    void onOrientationChanged(QOrientationReading::Orientation);

public Q_SLOTS:
    void onDisplayPowerStateChanged(int, int);

private:
    void startIfNeeded();

    QOrientationSensor *m_orientationSensor;
    QDBusInterface *m_unityScreen;
    bool m_enabled;
    bool m_started;
 };