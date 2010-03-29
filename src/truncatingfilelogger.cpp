/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

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

http://www.opensource.org/licenses/mit-license.php
*/
#include "truncatingfilelogger.h"

TruncatingFileLogger::TruncatingFileLogger(const QString &path, LOG_LEVEL lvl,
                                           QObject *parent)
    : QObject(parent)
    , m_path(path)
    , m_minimum_level(lvl)
    , m_file(0)
{
    // setup names for the logging levels
    m_level_names[LL_TRACE] = tr("TRACE");
    m_level_names[LL_DEBUG] = tr("DEBUG");
    m_level_names[LL_INFO] = tr("INFO");
    m_level_names[LL_WARN] = tr("WARNING");
    m_level_names[LL_ERROR] = tr("ERROR");
    m_level_names[LL_FATAL] = tr("FATAL");

    QDir d;
    if (!d.isAbsolutePath(m_path)) {
        m_path = d.absoluteFilePath(path);
    }

    // open the path we were given...
    m_file = new QFile(m_path);
    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Truncate |
                      QIODevice::Text)) {
        qCritical() << "Could not open logfile for writing!"
                << m_file->errorString();
    } else {
        write(LL_INFO, tr("Log Opened Successfully"));
    }
}

TruncatingFileLogger::~TruncatingFileLogger() {
    if (m_file) {
        if (m_file->isOpen()) {
            write(LL_INFO, tr("Closing log"));
            m_file->flush();
            m_file->close();
        }
        m_file->deleteLater();
    }
}

QString TruncatingFileLogger::level_name(LOG_LEVEL lvl) {
    return m_level_names.value(lvl, QString("%1").arg(lvl));
}

void TruncatingFileLogger::set_minimum_level(LOG_LEVEL lvl) {
    m_minimum_level = lvl;
    LOGI <<"Minimum log level set to" << level_name(lvl);
}

void TruncatingFileLogger::write(LOG_LEVEL lvl, const QString &message,
                                 const QString &file, int lineno,
                                 const QString &func) {
    if (m_file && m_file->isWritable() && lvl >= m_minimum_level) {
        // make a string for this log level
        QDateTime now = QDateTime::currentDateTime();
        QString msg("%1 %2\t%3 %4 %5\n");
        msg = msg.arg(now.toString("yyyy-MMM-dd hh:mm:ss.zzz"))
              .arg(level_name(lvl))
              .arg(message).toAscii();

        QString location;
        QString function;
        if (lvl != LL_INFO && !file.isEmpty()) {
            location = QString("[%1:%2]").arg(file).arg(lineno);
        }
        if (lvl > LL_INFO && !func.isEmpty()) {
            function = QString("(%1)").arg(func);
        }
        msg = msg.arg(location);
        msg = msg.arg(function);
        m_file->write(msg.toAscii());
        m_file->flush();
    }
}

Streamer::Streamer(TruncatingFileLogger *logger, LOG_LEVEL lvl, const QString &file,
         int lineno, const QString &func)
    : m_logger(logger)
    , m_level(lvl)
    , m_buffer(QString())
    , m_dbg(&m_buffer)
    , m_file(file)
    , m_lineno(lineno)
    , m_function(func)
{}

void Streamer::write() {
    m_logger->write(m_level, m_buffer, m_file, m_lineno, m_function);
}
