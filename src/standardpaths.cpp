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

constexpr StandardPaths::Mode StandardPaths::DefaultMode;

StandardPaths::Mode StandardPaths::mode;
#ifdef DEVMODE_PATH
QDir StandardPaths::source_datadir = QDir(DEVMODE_PATH);
#else
QDir StandardPaths::source_datadir;
#endif
QDir StandardPaths::appdir;
QDir StandardPaths::custom_datadir;
QDir StandardPaths::custom_configdir;

void StandardPaths::init_paths(Mode mode, QString source_datadir)
{
    StandardPaths::mode = mode;
    if (!source_datadir.isEmpty())
        StandardPaths::source_datadir.setPath(source_datadir);

    appdir = QCoreApplication::applicationDirPath();
    switch (mode) {
    case Mode::Portable:
#if (defined Q_OS_WIN)
        custom_datadir = appdir.filePath("data");
        custom_configdir = appdir;
#elif (defined Q_OS_OSX)
        custom_datadir = appdir.filePath("../Resources");
        custom_configdir = appdir.filePath("../Resources");
#elif (defined Q_OS_LINUX)
        custom_datadir = appdir.filePath("../share");
        custom_configdir = appdir.filePath("../etc");
#else
#   error "Unsupported OS"
#endif
        break;
    case Mode::Developer:
        custom_datadir = appdir.filePath("share");
        custom_configdir = appdir;
        break;
    default:
        break;
    }
}

std::unique_ptr<QSettings> StandardPaths::settings()
{
    switch (mode) {
    case Mode::Portable:
    case Mode::Developer: {
        auto ini_file = QString("%1.ini").arg(QCoreApplication::applicationName());
        return std::make_unique<QSettings>(custom_configdir.filePath(ini_file),
                                           QSettings::IniFormat);
    }
    case Mode::Standard:
    default:
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
    switch (mode) {
    case Mode::Portable:
        return custom_datadir.exists(filename)
            ? custom_datadir.filePath(filename)
            : QString();
    case Mode::Developer:
        if (custom_datadir.exists(filename))
            return custom_datadir.filePath(filename);
        else if (source_datadir.exists(filename))
            return source_datadir.filePath(filename);
        else
            return QString();
    case Mode::Standard:
    default:
        return QStandardPaths::locate(AppDataLocation, filename);
    }
}

QStringList StandardPaths::data_locations()
{
    switch (mode) {
    case Mode::Portable:
        return { custom_datadir.path() };
    case Mode::Developer:
        return { custom_datadir.path(), source_datadir.path() };
    case Mode::Standard:
    default:
        return QStandardPaths::standardLocations(AppDataLocation);
    }
}

QString StandardPaths::writable_data_location()
{
    switch (mode) {
    case Mode::Portable:
    case Mode::Developer:
        return custom_datadir.path();
    case Mode::Standard:
    default:
        return QStandardPaths::writableLocation(AppDataLocation);
    }
}

QStringList StandardPaths::doc_locations()
{
    switch (mode) {
    case Mode::Portable:
    case Mode::Developer:
#if (defined Q_OS_WIN)
        return { appdir.filePath("doc") };
#elif (defined Q_OS_OSX)
        return { appdir.filePath("../Resources") };
#elif (defined Q_OS_LINUX)
        return { appdir.filePath("../share/doc") };
#else
#   error "Unsupported OS"
#endif
    case Mode::Standard:
    default: {
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
}
