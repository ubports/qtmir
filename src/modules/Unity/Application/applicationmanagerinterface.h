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

#ifndef QTMIR_APPLICATION_MANAGER_INTERFACE_H
#define QTMIR_APPLICATION_MANAGER_INTERFACE_H

// Unity API
#include <unity/shell/application/ApplicationManagerInterface.h>

namespace qtmir {

class ApplicationManagerInterface : public unity::shell::application::ApplicationManagerInterface
{
public:
    ApplicationManagerInterface(QObject *parent) : unity::shell::application::ApplicationManagerInterface(parent) {}

    virtual Application* findApplicationWithSession(const std::shared_ptr<mir::scene::Session> &session) = 0;
};

} // namespace qtmir

#endif // QTMIR_APPLICATION_MANAGER_INTERFACE_H
