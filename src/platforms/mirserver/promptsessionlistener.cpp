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

#include "promptsessionlistener.h"
#include "logging.h"

namespace ms = mir::scene;

PromptSessionListener::PromptSessionListener(QObject *parent) :
    QObject(parent)
{
    qCDebug(QTMIR_MIR_MESSAGES) << "PromptSessionListener::PromptSessionListener - this=" << this;
    qRegisterMetaType<qtmir::PromptSession>("qtmir::PromptSession");
    qRegisterMetaType<miral::Application>("miral::Application");
}

PromptSessionListener::~PromptSessionListener()
{
    qCDebug(QTMIR_MIR_MESSAGES) << "PromptSessionListener::~PromptSessionListener - this=" << this;
}
