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

// local
#include "desktopfilereader.h"
#include "gscopedpointer.h"
#include "logging.h"

// Qt
#include <QFile>
#include <QLocale>

// GIO
#include <gio/gdesktopappinfo.h>

namespace qtmir
{


DesktopFileReader* DesktopFileReader::Factory::createInstance(const QString &appId, const QFileInfo& fi)
{
    return new DesktopFileReader(appId, fi);
}

typedef GObjectScopedPointer<GAppInfo> GAppInfoPointer;

class DesktopFileReaderPrivate
{
public:
    DesktopFileReaderPrivate(DesktopFileReader *parent):
            q_ptr( parent )
    {}

    QString getKey(const char *key) const
    {
        if (!loaded()) return QString();

        return QString::fromUtf8(g_desktop_app_info_get_string((GDesktopAppInfo*)appInfo.data(), key));
    }

    bool loaded() const
    {
        return !appInfo.isNull();
    }

    DesktopFileReader * const q_ptr;
    Q_DECLARE_PUBLIC(DesktopFileReader)

    QString appId;
    QString file;
    GAppInfoPointer appInfo; // GAppInfo is actually implemented by GDesktopAppInfo
};


DesktopFileReader::DesktopFileReader(const QString &appId, const QFileInfo &desktopFile)
    : d_ptr(new DesktopFileReaderPrivate(this))
{
    Q_D(DesktopFileReader);
    qCDebug(QTMIR_APPLICATIONS) << "Loading desktop file" << desktopFile.absoluteFilePath()
            << "for appId" << appId;

    d->appId = appId;
    d->file = desktopFile.absoluteFilePath();
    d->appInfo.reset((GAppInfo*) g_desktop_app_info_new_from_filename(d->file.toUtf8().constData()));

    if (!d->loaded()) {
        if (!desktopFile.exists()) {
            qCWarning(QTMIR_APPLICATIONS) << "Desktop file for appId:" << appId << "at:" << d->file
                                          << "does not exist";
        } else {
            qCWarning(QTMIR_APPLICATIONS) << "Desktop file for appId:" << appId << "at:" << d->file
                                      << "is not valid - check its syntax, and that the binary specified"
                                      << "by the Exec line is installed!";
        }
    }
}

DesktopFileReader::~DesktopFileReader()
{
    delete d_ptr;
}

QString DesktopFileReader::file() const
{
    Q_D(const DesktopFileReader);
    return d->file;
}

QString DesktopFileReader::appId() const
{
    Q_D(const DesktopFileReader);
    return d->appId;
}

QString DesktopFileReader::name() const
{
    Q_D(const DesktopFileReader);
    if (!d->loaded()) return QString();

    return QString::fromUtf8(g_app_info_get_name(d->appInfo.data()));
}

QString DesktopFileReader::comment() const
{
    Q_D(const DesktopFileReader);
    if (!d->loaded()) return QString();

    return QString::fromUtf8(g_app_info_get_description(d->appInfo.data()));
}

QString DesktopFileReader::icon() const
{
    Q_D(const DesktopFileReader);
    return d->getKey("Icon");
}

QString DesktopFileReader::exec() const
{
    Q_D(const DesktopFileReader);
    if (!d->loaded()) return QString();

    return QString::fromUtf8(g_app_info_get_commandline(d->appInfo.data()));
}

QString DesktopFileReader::path() const
{
    Q_D(const DesktopFileReader);
    return d->getKey("Path");
}

QString DesktopFileReader::stageHint() const
{
    Q_D(const DesktopFileReader);
    return d->getKey("X-Ubuntu-StageHint");
}

QString DesktopFileReader::splashTitle() const
{
    Q_D(const DesktopFileReader);
    if (!d->loaded()) return QString();

    /* Sadly GDesktopAppInfo only considers Name, GenericName, Comments and Keywords to be keys
     * which can have locale-specific entries. So we need to work to make X-Ubuntu-Splash-Title
     * locale-aware, by generating a locale-correct key name and seeing if that exists. If yes,
     * get the value and return it. Else fallback to the non-localized value.
     */
    GDesktopAppInfo *info = (GDesktopAppInfo*)d->appInfo.data();
    QLocale defaultLocale;
    QStringList locales = defaultLocale.uiLanguages();

    QString keyTemplate(QStringLiteral("X-Ubuntu-Splash-Title[%1]"));
    for (QString locale: locales) {
        // Desktop files use local specifiers with underscore separators but Qt uses hyphens
        locale = locale.replace('-', '_');
        const char* key = keyTemplate.arg(locale).toUtf8().constData();
        if (g_desktop_app_info_has_key(info, key)) {
            return d->getKey(key);
        }
    }

    // Fallback to the non-localized string, if available
    return d->getKey("X-Ubuntu-Splash-Title");
}

QString DesktopFileReader::splashImage() const
{
    Q_D(const DesktopFileReader);
    return d->getKey("X-Ubuntu-Splash-Image");
}

QString DesktopFileReader::splashShowHeader() const
{
    Q_D(const DesktopFileReader);
    return d->getKey("X-Ubuntu-Splash-Show-Header");
}

QString DesktopFileReader::splashColor() const
{
    Q_D(const DesktopFileReader);
    return d->getKey("X-Ubuntu-Splash-Color");
}

QString DesktopFileReader::splashColorHeader() const
{
    Q_D(const DesktopFileReader);
    return d->getKey("X-Ubuntu-Splash-Color-Header");
}

QString DesktopFileReader::splashColorFooter() const
{
    Q_D(const DesktopFileReader);
    return d->getKey("X-Ubuntu-Splash-Color-Footer");
}

Qt::ScreenOrientations DesktopFileReader::supportedOrientations() const
{
    Q_D(const DesktopFileReader);
    Qt::ScreenOrientations result;

    if (!parseOrientations(d->getKey("X-Ubuntu-Supported-Orientations"), result)) {
        qCWarning(QTMIR_APPLICATIONS) << d->file << "has an invalid X-Ubuntu-Supported-Orientations entry.";
    }

    return result;
}

bool DesktopFileReader::rotatesWindowContents() const
{
    Q_D(const DesktopFileReader);
    bool result;

    if (!parseBoolean(d->getKey("X-Ubuntu-Rotates-Window-Contents"), result)) {
        qCWarning(QTMIR_APPLICATIONS) << d->file << "has an invalid X-Ubuntu-Rotates-Window-Contents entry.";
    }

    return result;
}

bool DesktopFileReader::isTouchApp() const
{
    Q_D(const DesktopFileReader);
    bool result;

    if (!parseBoolean(d->getKey("X-Ubuntu-Touch"), result)) {
        qCWarning(QTMIR_APPLICATIONS) << d->file << "has an invalid X-Ubuntu-Touch entry.";
    }

    return result;
}

bool DesktopFileReader::parseOrientations(const QString &rawString, Qt::ScreenOrientations &result)
{
    // Default to all orientations
    result = Qt::PortraitOrientation | Qt::LandscapeOrientation
        | Qt::InvertedPortraitOrientation | Qt::InvertedLandscapeOrientation;

    if (rawString.isEmpty()) {
        return true;
    }

    Qt::ScreenOrientations parsedOrientations = 0;
    bool ok = true;

    QStringList orientationsList = rawString
            .simplified()
            .replace(QChar(','), QStringLiteral(";"))
            .remove(QChar(' '))
            .remove(QChar('-'))
            .remove(QChar('_'))
            .toLower()
            .split(QStringLiteral(";"));

    for (int i = 0; i < orientationsList.count() && ok; ++i) {
        const QString &orientationString = orientationsList.at(i);
        if (orientationString.isEmpty()) {
            // skip it
            continue;
        }

        if (orientationString == QLatin1String("portrait")) {
            parsedOrientations |= Qt::PortraitOrientation;
        } else if (orientationString == QLatin1String("landscape")) {
            parsedOrientations |= Qt::LandscapeOrientation;
        } else if (orientationString == QLatin1String("invertedportrait")) {
            parsedOrientations |= Qt::InvertedPortraitOrientation;
        } else if (orientationString == QLatin1String("invertedlandscape")) {
            parsedOrientations |= Qt::InvertedLandscapeOrientation;
        } else if (orientationsList.count() == 1 && orientationString == QLatin1String("primary")) {
            // Special case: primary orientation must be alone
            // There's no sense in supporting primary orientation + other orientations
            // like "primary,landscape"
            parsedOrientations = Qt::PrimaryOrientation;
        } else {
            ok = false;
        }
    }

    if (ok) {
        result = parsedOrientations;
    }

    return ok;
}

bool DesktopFileReader::parseBoolean(const QString &rawString, bool &result)
{
    QString cookedString = rawString.trimmed().toLower();

    result = cookedString == QLatin1String("y")
          || cookedString == QLatin1String("1")
          || cookedString == QLatin1String("yes")
          || cookedString == QLatin1String("true");

    return result || rawString.isEmpty()
        || cookedString == QLatin1String("n")
        || cookedString == QLatin1String("0")
        || cookedString == QLatin1String("no")
        || cookedString == QLatin1String("false");
}

bool DesktopFileReader::loaded() const
{
    Q_D(const DesktopFileReader);
    return d->loaded();
}

} // namespace qtmir
