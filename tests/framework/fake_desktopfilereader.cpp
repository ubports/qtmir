#include "fake_desktopfilereader.h"

namespace qtmir
{

FakeDesktopFileReader::FakeDesktopFileReader()
	: DesktopFileReader()
	, m_appId("foo-app")
{
}

FakeDesktopFileReader::~FakeDesktopFileReader()
{
}

QString FakeDesktopFileReader::file() const { return QString(); }

QString FakeDesktopFileReader::appId() const { return m_appId; }

QString FakeDesktopFileReader::name() const { return QString(); }

QString FakeDesktopFileReader::comment() const { return QString(); }

QString FakeDesktopFileReader::icon() const { return QString(); }

QString FakeDesktopFileReader::exec() const { return QString(); }

QString FakeDesktopFileReader::path() const { return QString(); }

QString FakeDesktopFileReader::stageHint() const { return QString(); }

QString FakeDesktopFileReader::splashTitle() const { return QString(); }

QString FakeDesktopFileReader::splashImage() const { return QString(); }

QString FakeDesktopFileReader::splashShowHeader() const { return QString(); }

QString FakeDesktopFileReader::splashColor() const { return QString(); }

QString FakeDesktopFileReader::splashColorHeader() const { return QString(); }

QString FakeDesktopFileReader::splashColorFooter() const { return QString(); }

Qt::ScreenOrientations FakeDesktopFileReader::supportedOrientations() const { return Qt::PortraitOrientation; }

bool FakeDesktopFileReader::rotatesWindowContents() const { return false; }

bool FakeDesktopFileReader::loaded() const { return true; }

} // namespace qtmir
