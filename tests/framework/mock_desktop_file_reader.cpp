#include "mock_desktop_file_reader.h"

testing::MockDesktopFileReader::MockDesktopFileReader(const QString &appId, const QFileInfo &fileInfo)
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
    ON_CALL(*this, loaded()).WillByDefault(Invoke(this, &MockDesktopFileReader::doLoaded));
}

testing::MockDesktopFileReader::~MockDesktopFileReader()
{
}

QString testing::MockDesktopFileReader::doFile() const
{
    return DesktopFileReader::file();
}

QString testing::MockDesktopFileReader::doAppId() const
{
    return DesktopFileReader::appId();
}

QString testing::MockDesktopFileReader::doName() const
{
    return DesktopFileReader::name();
}

QString testing::MockDesktopFileReader::doComment() const
{
    return DesktopFileReader::comment();
}

QString testing::MockDesktopFileReader::doIcon() const
{
    return DesktopFileReader::icon();
}

QString testing::MockDesktopFileReader::doExec() const
{
    return DesktopFileReader::exec();
}

QString testing::MockDesktopFileReader::doPath() const
{
    return DesktopFileReader::path();
}

QString testing::MockDesktopFileReader::doStageHint() const
{
    return DesktopFileReader::stageHint();
}

bool testing::MockDesktopFileReader::doLoaded() const
{
    return DesktopFileReader::loaded();
}


testing::MockDesktopFileReaderFactory::MockDesktopFileReaderFactory()
{
    using namespace ::testing;
    ON_CALL(*this, createInstance(_, _))
            .WillByDefault(
                Invoke(
                    this,
                    &MockDesktopFileReaderFactory::doCreateInstance));
}

testing::MockDesktopFileReaderFactory::~MockDesktopFileReaderFactory()
{
}

qtmir::DesktopFileReader *testing::MockDesktopFileReaderFactory::doCreateInstance(const QString &appId, const QFileInfo &fi)
{
    using namespace ::testing;
    auto instance = new NiceMock<MockDesktopFileReader>(appId, fi);
    ON_CALL(*instance, loaded()).WillByDefault(Return(true));

    return instance;
}
