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

#ifndef STANDARD_PATHS_H
#define STANDARD_PATHS_H

#include <QDir>
#include <QSettings>
#include <memory>

class StandardPaths
{
public:
    enum class Mode {
        Standard,
        Portable,
        Developer,
    };

#if defined BUILD_PORTABLE
    static constexpr Mode DefaultMode = Mode::Portable;
#elif defined DEVMODE_PATH
    static constexpr Mode DefaultMode = Mode::Developer;
#else
    static constexpr Mode DefaultMode = Mode::Standard;
#endif

    static void init_paths(Mode mode = DefaultMode, QString source_datadir = QString());

    static std::unique_ptr<QSettings> settings();
    static QString locate_data(const QString &filename);
    static QStringList data_locations();
    static QString writable_data_location();
    static QStringList doc_locations();

private:
    static Mode mode;
    static QDir source_datadir;
    static QDir appdir;
    static QDir custom_datadir, custom_configdir;
};

#endif
