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

#ifndef INPUTDEVICEOBSERVER_H
#define INPUTDEVICEOBSERVER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMutex>

namespace mir { namespace input { class Device; }}

namespace qtmir {

class MirInputDeviceObserver: public QObject
{
    Q_OBJECT
public:
    MirInputDeviceObserver(QObject * parent = nullptr);
    ~MirInputDeviceObserver() = default;

private Q_SLOTS:
    void setKeymap(const QString &keymap);

protected:
    void applyKeymap();
    void applyKeymap(const std::shared_ptr<mir::input::Device> &device);

    QString m_keymap;
    QVector<std::shared_ptr<mir::input::Device>> m_devices;
    QMutex m_mutex;
};

} // namespace qtmir

#endif
