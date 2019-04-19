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

#include "orientationsensor.h"
#include "logging.h"

// Qt sensors
#include <QtSensors/QOrientationReading>
#include <QtSensors/QOrientationSensor>

// Used by tests
bool OrientationSensor::skipDBusRegistration = false;

OrientationSensor::OrientationSensor(QObject *parent)
    : QObject(parent)
    , m_orientationSensor(nullptr)
    , m_unityScreen(nullptr)
    , m_enabled(false)
    , m_started(false)
{
}

void OrientationSensor::onDisplayPowerStateChanged(int status, int /*reason*/)
{
    qCDebug(QTMIR_SENSOR_MESSAGES) << "OrientationSensor::onDisplayPowerStateChanged";
    if (m_orientationSensor != nullptr) {
        if (status)
            start();
        else
            stop();
    }
}

// Used by tests
bool OrientationSensor::enabled() {
    if (m_orientationSensor != nullptr)
        return m_orientationSensor->isActive();
    return false;
}

void OrientationSensor::start() {
    m_started = true;
    startIfNeeded();
}

void OrientationSensor::stop() {
    if (m_orientationSensor != nullptr) {
        if (m_orientationSensor->isActive())
            m_orientationSensor->stop();
    }
}

void OrientationSensor::enable() {
    m_enabled = true;
    startIfNeeded();
}

// Make sure we dont start/enable the sensor or dbus if it's not needed
void OrientationSensor::startIfNeeded() {
    // We need to initiaze the sensor after the main mir thread has started
    if (m_orientationSensor == nullptr && m_enabled && m_started) {
        m_orientationSensor = new QOrientationSensor(this);
        QObject::connect(m_orientationSensor, &QOrientationSensor::readingChanged,
                        this, [this]() {
                        qCDebug(QTMIR_SENSOR_MESSAGES) << "OrientationSensor::readingChanged";
                        Q_EMIT onOrientationChanged(m_orientationSensor->reading()->orientation()); });

        if (!skipDBusRegistration) {
            // FIXME This is a unity8 specific dbus call and shouldn't be in qtmir
            m_unityScreen = new QDBusInterface(QStringLiteral("com.canonical.Unity.Screen"),
                                            QStringLiteral("/com/canonical/Unity/Screen"),
                                            QStringLiteral("com.canonical.Unity.Screen"),
                                            QDBusConnection::systemBus(), this);

            m_unityScreen->connection().connect(QStringLiteral("com.canonical.Unity.Screen"),
                                            QStringLiteral("/com/canonical/Unity/Screen"),
                                            QStringLiteral("com.canonical.Unity.Screen"),
                                            QStringLiteral("DisplayPowerStateChange"),
                                            this,
                                            SLOT(onDisplayPowerStateChanged(int, int)));
        }
    }
    if (m_orientationSensor != nullptr) {
        if (!m_orientationSensor->isActive())
            m_orientationSensor->start();
    }
}