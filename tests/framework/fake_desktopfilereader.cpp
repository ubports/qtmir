#include "fake_desktopfilereader.h"

qtmir::FakeDesktopFileReader::FakeDesktopFileReader()
	: DesktopFileReader()
	, m_appId("foo-app")
{
}

qtmir::FakeDesktopFileReader::~FakeDesktopFileReader()
{
}

QString qtmir::FakeDesktopFileReader::file() const { return QString(); }

QString qtmir::FakeDesktopFileReader::appId() const { return m_appId; }

QString qtmir::FakeDesktopFileReader::name() const { return QString(); }

QString qtmir::FakeDesktopFileReader::comment() const { return QString(); }

QString qtmir::FakeDesktopFileReader::icon() const { return QString(); }

QString qtmir::FakeDesktopFileReader::exec() const { return QString(); }

QString qtmir::FakeDesktopFileReader::path() const { return QString(); }

QString qtmir::FakeDesktopFileReader::stageHint() const { return QString(); }

QString qtmir::FakeDesktopFileReader::splashTitle() const { return QString(); }

QString qtmir::FakeDesktopFileReader::splashImage() const { return QString(); }

QString qtmir::FakeDesktopFileReader::splashShowHeader() const { return QString(); }

QString qtmir::FakeDesktopFileReader::splashColor() const { return QString(); }

QString qtmir::FakeDesktopFileReader::splashColorHeader() const { return QString(); }

QString qtmir::FakeDesktopFileReader::splashColorFooter() const { return QString(); }

Qt::ScreenOrientations qtmir::FakeDesktopFileReader::supportedOrientations() const { return Qt::PortraitOrientation; }

bool qtmir::FakeDesktopFileReader::rotatesWindowContents() const { return false; }

bool qtmir::FakeDesktopFileReader::loaded() const { return true; }
