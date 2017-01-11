/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
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

#ifndef PROMPTSESSIONLISTENER_H
#define PROMPTSESSIONLISTENER_H

// Qt
#include <QHash>
#include <QObject>

#include "promptsession.h"

#include <miral/application.h>

class PromptSessionListener : public QObject
{
Q_OBJECT
protected:
    explicit PromptSessionListener(QObject *parent = 0);
    ~PromptSessionListener();

public:
Q_SIGNALS:
    void promptSessionStarting(qtmir::PromptSession const &session);
    void promptSessionStopping(qtmir::PromptSession const &session);
    void promptSessionSuspending(qtmir::PromptSession const &session);
    void promptSessionResuming(qtmir::PromptSession const &session);

    void promptProviderAdded(qtmir::PromptSession const&, miral::Application const&);
    void promptProviderRemoved(qtmir::PromptSession const&, miral::Application const&);
};

Q_DECLARE_METATYPE(qtmir::PromptSession)
Q_DECLARE_METATYPE(miral::Application)

#endif // SESSIONLISTENER_H
