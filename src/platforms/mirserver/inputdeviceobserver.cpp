/*
 * Copyright (C) 2016-2017 Canonical, Ltd.
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

#include <mir/input/device.h>
#include <mir/input/device_capability.h>
#include <mir/input/keyboard_configuration.h>

#include <Qt>
#include <QTimer>

#include "inputdeviceobserver.h"
#include "mirsingleton.h"
#include "logging.h"

using namespace qtmir;
namespace mi = mir::input;

MirInputDeviceObserver::MirInputDeviceObserver(QObject *parent):
    QObject(parent)
{
    connect(Mir::instance(), &Mir::currentKeymapChanged, this, &MirInputDeviceObserver::setKeymap, Qt::DirectConnection);
}

MirInputDeviceObserver::~MirInputDeviceObserver()
{
    m_devices.clear();
}

void MirInputDeviceObserver::setKeymap(const QString &keymap)
{
    QMutexLocker locker(&m_mutex);

    if (keymap != m_keymap) {
        qCDebug(QTMIR_MIR_KEYMAP) << "SET KEYMAP" << keymap;
        m_keymap = keymap;
        applyKeymap();
    }
}

void MirInputDeviceObserver::applyKeymap()
{
    Q_FOREACH(const auto &device, m_devices) {
        applyKeymap(device);
    }
}

void MirInputDeviceObserver::device_added(const std::shared_ptr<mi::Device> &device)
{
    QMutexLocker locker(&m_mutex);

    if (mir::contains(device->capabilities(), mi::DeviceCapability::keyboard) &&
            mir::contains(device->capabilities(), mi::DeviceCapability::alpha_numeric)) {
        qCDebug(QTMIR_MIR_KEYMAP) << "Device added" << device->id();
        m_devices.append(device);
        applyKeymap(device);
    }
}

void MirInputDeviceObserver::device_removed(const std::shared_ptr<mi::Device> &device)
{
    QMutexLocker locker(&m_mutex);

    if (device && m_devices.contains(device)) {
        qCDebug(QTMIR_MIR_KEYMAP) << "Device removed" << device->id();
        m_devices.removeAll(device);
    }
}

void MirInputDeviceObserver::applyKeymap(const std::shared_ptr<mi::Device> &device)
{
    if (!m_keymap.isEmpty()) {
        const QStringList stringList = m_keymap.split('+', QString::SkipEmptyParts);

        const QString &layout = stringList.at(0);
        QString variant;

        if (stringList.count() > 1) {
            variant = stringList.at(1);
        }

        qCDebug(QTMIR_MIR_KEYMAP) << "Applying keymap" <<  layout << variant << "on" << device->id() << QString::fromStdString(device->name());
        mi::KeyboardConfiguration oldConfig;
        mi::Keymap keymap;
        if (device->keyboard_configuration().is_set()) { // preserve the model and options
            oldConfig = device->keyboard_configuration().value();
            keymap.model = oldConfig.device_keymap.model;
            keymap.options = oldConfig.device_keymap.options;
        }
        keymap.layout = layout.toStdString();
        keymap.variant = variant.toStdString();

        device->apply_keyboard_configuration(std::move(keymap));
        qCDebug(QTMIR_MIR_KEYMAP) << "Keymap applied";
    }
}
