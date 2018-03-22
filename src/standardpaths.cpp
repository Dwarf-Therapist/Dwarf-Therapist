/*
Dwarf Therapist
Copyright (c) 2018 Clement Vuchener

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "standardpaths.h"

#include <QStandardPaths>
#include <QCoreApplication>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
static constexpr auto AppDataLocation = QStandardPaths::AppDataLocation;
#else
static constexpr auto AppDataLocation = QStandardPaths::DataLocation;
#endif

#ifdef BUILD_PORTABLE
bool StandardPaths::portable = true;
#else
bool StandardPaths::portable = false;
#endif

QDir StandardPaths::appdir;
QDir StandardPaths::portable_datadir;
QDir StandardPaths::portable_configdir;

void StandardPaths::init_paths()
{
    appdir = QCoreApplication::applicationDirPath();
#if (defined Q_OS_WIN)
    portable_datadir = appdir.filePath("data");
    portable_configdir = appdir;
#elif (defined Q_OS_OSX)
    portable_datadir = appdir.filePath("../Resources");
    portable_configdir = appdir.filePath("../Resources");
#elif (defined Q_OS_LINUX)
    portable_datadir = appdir.filePath("../share");
    portable_configdir = appdir.filePath("../etc");
#else
#   error "Unsupported OS"
#endif
}

std::unique_ptr<QSettings> StandardPaths::settings()
{
    if (portable) {
        auto ini_file = QString("%1.ini").arg(QCoreApplication::applicationName());
        return std::make_unique<QSettings>(portable_configdir.filePath(ini_file),
                                           QSettings::IniFormat);
    }
    else {
        // Organization is not set on Linux/Windows to avoid issue with QStandardPaths
        // using "orgname/appname" folder instead "packagename". Force QSettings
        // to use the application name for the configuration directory.
        return std::make_unique<QSettings>(QSettings::IniFormat, QSettings::UserScope,
                                           QCoreApplication::applicationName(),
                                           QCoreApplication::applicationName());
    }
}

QString StandardPaths::locate_data(const QString &filename)
{
    if (portable) {
        QFileInfo file (portable_datadir, filename);
        return file.exists() ? file.filePath() : QString();
    }
    else {
        return QStandardPaths::locate(AppDataLocation, filename);
    }
}

QStringList StandardPaths::data_locations()
{
    if (portable)
        return { portable_datadir.path() };
    else
        return QStandardPaths::standardLocations(AppDataLocation);
}

QString StandardPaths::writable_data_location()
{
    if (portable)
        return portable_datadir.path();
    else
        return QStandardPaths::writableLocation(AppDataLocation);
}

QStringList StandardPaths::doc_locations()
{
    if (portable) {
#if (defined Q_OS_WIN)
        return { appdir.filePath("doc") };
#elif (defined Q_OS_OSX)
        return { appdir.filePath("../Resources") };
#elif (defined Q_OS_LINUX)
        return { appdir.filePath("../share/doc") };
#else
#   error "Unsupported OS"
#endif
    }
    else {
#if (defined Q_OS_WIN)
        auto dirs = QStandardPaths::standardLocations(AppDataLocation);
        for (auto &dir: dirs)
            dir += "/doc";
        return dirs;
#elif (defined Q_OS_OSX)
        return QStandardPaths::standardLocations(AppDataLocation);
#elif (defined Q_OS_LINUX)
        auto dirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        for (auto &dir: dirs)
            dir += "/doc/" + QCoreApplication::applicationName();
        return dirs;
#else
#   error "Unsupported OS"
#endif
    }
}
