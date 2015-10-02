/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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

#ifndef MOCK_DESKTOP_FILE_READER_H
#define MOCK_DESKTOP_FILE_READER_H

#include <Unity/Application/desktopfilereader.h>

#include <gmock/gmock.h>

namespace qtmir
{

struct MockDesktopFileReader : public qtmir::DesktopFileReader
{
    MockDesktopFileReader(const QString &appId, const QFileInfo& fileInfo);
    virtual ~MockDesktopFileReader();

    MOCK_CONST_METHOD0(file, QString());
    MOCK_CONST_METHOD0(appId, QString ());
    MOCK_CONST_METHOD0(name, QString());
    MOCK_CONST_METHOD0(comment, QString());
    MOCK_CONST_METHOD0(icon, QString());
    MOCK_CONST_METHOD0(exec, QString());
    MOCK_CONST_METHOD0(path, QString());
    MOCK_CONST_METHOD0(stageHint, QString());
    MOCK_CONST_METHOD0(loaded, bool());

    QString doFile() const;
    QString doAppId() const;
    QString doName() const;
    QString doComment() const;
    QString doIcon() const;
    QString doExec() const;
    QString doPath() const;
    QString doStageHint() const;
    bool doLoaded() const;
};

struct MockDesktopFileReaderFactory : public qtmir::DesktopFileReader::Factory
{
    MockDesktopFileReaderFactory();
    virtual ~MockDesktopFileReaderFactory();

    virtual qtmir::DesktopFileReader* doCreateInstance(const QString &appId, const QFileInfo &fi);

    MOCK_METHOD2(createInstance, qtmir::DesktopFileReader*(const QString &appId, const QFileInfo &fi));
};

} // namespace qtmir

#endif // MOCK_DESKTOP_FILE_READER_H
