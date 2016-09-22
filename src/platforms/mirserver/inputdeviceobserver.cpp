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

#include <mir/input/device.h>
#include <mir/input/device_capability.h>
#include <mir/input/keyboard_configuration.h>

#include <Qt>
#include <QDebug>
#include <QTimer>

#include "inputdeviceobserver.h"
#include "mirsingleton.h"

MirInputDeviceObserver::MirInputDeviceObserver(const QString &keymap, const std::shared_ptr<mir::input::InputDeviceHub> &hub, QObject * parent):
    QObject(parent), m_keymap(keymap), m_hub(hub)
{
    qDebug() << "!!! INIT, keymap:" << m_keymap;

    m_hub->for_each_input_device([this](const mir::input::Device &device){
        qDebug() << "!!! Probing device" << device.id();
        auto newDevice = std::shared_ptr<mir::input::Device>(const_cast<mir::input::Device *>(&device));
        device_added(newDevice);
    });

    qDebug() << "!!! INIT COMPLETE, devices:" << m_devices.count();

    bool result = connect(qtmir::Mir::instance(), &qtmir::Mir::currentKeymapChanged,
                          this, &MirInputDeviceObserver::setKeymap);

    qDebug() << "!!! Input device observer created and connected with status:" << result;
}

MirInputDeviceObserver::~MirInputDeviceObserver()
{
    m_devices.clear();
}

void MirInputDeviceObserver::setKeymap(const QString &keymap)
{
    qDebug() << "!!! SET KEYMAP" << keymap;
    m_keymap = keymap;
    applyKeymap();
}

void MirInputDeviceObserver::applyKeymap()
{
    Q_FOREACH(const auto &device, m_devices) {
        applyKeymap(device);
    }
}

void MirInputDeviceObserver::device_added(const std::shared_ptr<mir::input::Device> &device)
{
    if (mir::contains(device->capabilities(), mir::input::DeviceCapability::keyboard) &&
            mir::contains(device->capabilities(), mir::input::DeviceCapability::alpha_numeric)) {
        qDebug() << "!!! Device added" << device->id();
        m_devices.append(device);
        applyKeymap(device);
    }
}

void MirInputDeviceObserver::device_removed(const std::shared_ptr<mir::input::Device> &device)
{
    if (device && m_devices.contains(device)) {
        qDebug() << "!!! Device removed" << device->id();
        m_devices.removeAll(device);
    }
}

void MirInputDeviceObserver::applyKeymap(const std::shared_ptr<mir::input::Device> &device)
{
    if (!m_keymap.isEmpty()) {
        const QStringList stringList = m_keymap.split('+', QString::SkipEmptyParts);

        const QString layout = stringList.at(0);
        QString variant;

        if (stringList.count() > 1) {
            variant = stringList.at(1);
        }

        qDebug() << "!!! Applying keymap" <<  layout << variant << "on" << device->id() << QString::fromStdString(device->name());
        mir::input::KeyboardConfiguration oldConfig;
        mir::input::Keymap keymap;
        if (device->keyboard_configuration().is_set()) { // preserve the model and options
            oldConfig = device->keyboard_configuration().value();
            keymap.model = oldConfig.device_keymap.model;
            keymap.options = oldConfig.device_keymap.options;
        }
        keymap.layout = layout.toStdString();
        keymap.variant = variant.toStdString();

        device->apply_keyboard_configuration(std::move(keymap));
        qDebug() << "!!! Keymap applied";
    }
}
