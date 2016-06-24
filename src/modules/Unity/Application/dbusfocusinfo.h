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

#include <QList>

#include "application.h"

namespace qtmir {

/*
  FIXME: This is a hack to provide OSK with needed info for avoiding input snooping.
         Remove when possible
 */
class DBusFocusInfo : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.canonical.Unity.FocusInfo")
public:
    explicit DBusFocusInfo(const QList<Application*> &applications);
    virtual ~DBusFocusInfo() {}

public Q_SLOTS:

    /*
        Returns true if the application with the given PID has input focus
     */
    Q_SCRIPTABLE bool isPidFocused(unsigned int pid);

private:
    SessionInterface* findSessionWithPid(unsigned int uintPid);

    const QList<Application*> &m_applications;
};

} // namespace qtmir
