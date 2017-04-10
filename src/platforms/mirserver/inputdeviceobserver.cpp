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
#include <mir/input/mir_keyboard_config.h>

#include <Qt>
#include <QTimer>

#include "inputdeviceobserver.h"
#include "mirsingleton.h"
#include "logging.h"

using namespace qtmir;
namespace mi = mir::input;

// Note: MirInputDeviceObserver has affinity to a Mir thread, but it is expected setKeymap will be called from the Qt GUI thread

MirInputDeviceObserver::MirInputDeviceObserver(QObject *parent):
    QObject(parent)
{
    // NB: have to use a Direct connection here, as it's called from Qt GUI thread
    connect(Mir::instance(), &Mir::currentKeymapChanged, this, &MirInputDeviceObserver::setKeymap, Qt::DirectConnection);
}

void MirInputDeviceObserver::setKeymap(const QString &keymap)
{
    QMutexLocker locker(&m_mutex); // lock so that Qt and Mir don't apply the keymap at the same time

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
        MirKeyboardConfig oldConfig;
        mi::Keymap keymap;
        if (device->keyboard_configuration().is_set()) { // preserve the model and options
            oldConfig = device->keyboard_configuration().value();
            keymap.model = oldConfig.device_keymap().model;
            keymap.options = oldConfig.device_keymap().options;
        }
        keymap.layout = layout.toStdString();
        keymap.variant = variant.toStdString();

        try
        {
            device->apply_keyboard_configuration(std::move(keymap));
            qCDebug(QTMIR_MIR_KEYMAP) << "Keymap applied";
        }
        catch(std::exception const& e)
        {
            qCWarning(QTMIR_MIR_KEYMAP) << "Keymap could not be applied:" << e.what();
        }
    }
}
