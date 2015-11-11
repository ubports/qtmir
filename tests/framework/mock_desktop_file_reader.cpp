/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include "mock_desktop_file_reader.h"

namespace qtmir
{

MockDesktopFileReader::MockDesktopFileReader(const QString &appId, const QFileInfo &fileInfo)
    : DesktopFileReader(appId, fileInfo)
{
    using namespace ::testing;

    ON_CALL(*this, file()).WillByDefault(Invoke(this, &MockDesktopFileReader::doFile));
    ON_CALL(*this, appId()).WillByDefault(Invoke(this, &MockDesktopFileReader::doAppId));
    ON_CALL(*this, name()).WillByDefault(Invoke(this, &MockDesktopFileReader::doName));
    ON_CALL(*this, comment()).WillByDefault(Invoke(this, &MockDesktopFileReader::doComment));
    ON_CALL(*this, icon()).WillByDefault(Invoke(this, &MockDesktopFileReader::doIcon));
    ON_CALL(*this, exec()).WillByDefault(Invoke(this, &MockDesktopFileReader::doExec));
    ON_CALL(*this, path()).WillByDefault(Invoke(this, &MockDesktopFileReader::doPath));
    ON_CALL(*this, stageHint()).WillByDefault(Invoke(this, &MockDesktopFileReader::doStageHint));
    ON_CALL(*this, isTouchApp()).WillByDefault(Invoke(this, &MockDesktopFileReader::doIsTouchApp));
    ON_CALL(*this, loaded()).WillByDefault(Invoke(this, &MockDesktopFileReader::doLoaded));
}

MockDesktopFileReader::~MockDesktopFileReader()
{
}

QString MockDesktopFileReader::doFile() const
{
    return DesktopFileReader::file();
}

QString MockDesktopFileReader::doAppId() const
{
    return DesktopFileReader::appId();
}

QString MockDesktopFileReader::doName() const
{
    return DesktopFileReader::name();
}

QString MockDesktopFileReader::doComment() const
{
    return DesktopFileReader::comment();
}

QString MockDesktopFileReader::doIcon() const
{
    return DesktopFileReader::icon();
}

QString MockDesktopFileReader::doExec() const
{
    return DesktopFileReader::exec();
}

QString MockDesktopFileReader::doPath() const
{
    return DesktopFileReader::path();
}

QString MockDesktopFileReader::doStageHint() const
{
    return DesktopFileReader::stageHint();
}

bool MockDesktopFileReader::doIsTouchApp() const
{
    return DesktopFileReader::isTouchApp();
}

bool MockDesktopFileReader::doLoaded() const
{
    return DesktopFileReader::loaded();
}


MockDesktopFileReaderFactory::MockDesktopFileReaderFactory()
{
    using namespace ::testing;
    ON_CALL(*this, createInstance(_, _))
            .WillByDefault(
                Invoke(
                    this,
                    &MockDesktopFileReaderFactory::doCreateInstance));
}

MockDesktopFileReaderFactory::~MockDesktopFileReaderFactory()
{
}

qtmir::DesktopFileReader *MockDesktopFileReaderFactory::doCreateInstance(const QString &appId, const QFileInfo &fi)
{
    using namespace ::testing;
    auto instance = new NiceMock<MockDesktopFileReader>(appId, fi);
    ON_CALL(*instance, loaded()).WillByDefault(Return(true));

    return instance;
}

} // namespace qtmir
