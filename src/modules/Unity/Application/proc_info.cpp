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

#include "proc_info.h"

// Qt
#include <QFile>
#include <QRegularExpression>

namespace qtmir
{

std::unique_ptr<ProcInfo::CommandLine> ProcInfo::commandLine(pid_t pid)
{
    QFile cmdline(QStringLiteral("/proc/%1/cmdline").arg(pid));
    if (!cmdline.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return nullptr;
    }

    return std::unique_ptr<CommandLine>(new CommandLine{ cmdline.readLine().replace('\0', ' ') });
}

QStringList ProcInfo::CommandLine::asStringList() const
{
    return QString(m_command.data()).split(' ');
}

bool ProcInfo::CommandLine::startsWith(char const* prefix) const
{
    return m_command.startsWith(prefix);
}

bool ProcInfo::CommandLine::contains(char const* prefix) const
{
    return m_command.contains(prefix);
}

QString ProcInfo::CommandLine::getParameter(const char* name) const
{
    QString pattern = QRegularExpression::escape(name) + "(\\S+)";
    QRegularExpression regExp(pattern);
    QRegularExpressionMatch regExpMatch = regExp.match(m_command);

    if (!regExpMatch.hasMatch()) {
        return QString();
    }

    return QString(regExpMatch.captured(1));
}

QString ProcInfo::CommandLine::getExec() const
{
    return asStringList()[0];
}


std::unique_ptr<ProcInfo::Environment> ProcInfo::environment(pid_t pid)
{
    QFile environment(QStringLiteral("/proc/%1/environ").arg(pid));
    if (!environment.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return nullptr;
    }

    return std::unique_ptr<Environment>(new Environment{ environment.readLine().replace('\0', ' ') });
}

bool ProcInfo::Environment::contains(char const* prefix) const
{
    return m_environment.contains(prefix);
}

QString ProcInfo::Environment::getParameter(const char* name) const
{
    QString pattern = QRegularExpression::escape(name) + "=(\\S+)";
    QRegularExpression regExp(pattern);
    QRegularExpressionMatch regExpMatch = regExp.match(m_environment);

    if (!regExpMatch.hasMatch()) {
        return QString();
    }

    return QString(regExpMatch.captured(1));
}

} // namespace qtmir
