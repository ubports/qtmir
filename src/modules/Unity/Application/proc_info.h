/*
 * Copyright (C) 2014-2017 Canonical, Ltd.
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

// Process Information

#ifndef PROC_INFO_H
#define PROC_INFO_H

// std
#include <memory>

// Qt
#include <QByteArray>
#include <QStringList>

class QString;

namespace qtmir
{

class ProcInfo
{
public:
    struct CommandLine {
        QByteArray m_command;

        bool startsWith(const char* prefix) const;
        bool contains(const char* prefix) const;
        QString getParameter(const char* name) const;
        QStringList asStringList() const;
        QString getExec() const;
    };

    struct Environment {
        QByteArray m_environment;

        bool contains(const char* prefix) const;
        QString getParameter(const char* name) const;
    };

    virtual std::unique_ptr<CommandLine> commandLine(pid_t pid);
    virtual std::unique_ptr<Environment> environment(pid_t pid);
    virtual ~ProcInfo() = default;
};

} // namespace qtmir

#endif
