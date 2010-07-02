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

LogManager::LogManager(QObject *parent)
    : QObject(parent)
{
    // setup names for the logging levels
    m_level_names[LL_TRACE] = tr("TRACE");
    m_level_names[LL_DEBUG] = tr("DEBUG");
    m_level_names[LL_INFO] = tr("INFO");
    m_level_names[LL_WARN] = tr("WARNING");
    m_level_names[LL_ERROR] = tr("ERROR");
    m_level_names[LL_FATAL] = tr("FATAL");
}

TruncatingFileLogger *LogManager::add_logger(const QString &path) {
    TruncatingFileLogger *ret_val = get_logger(path);
    if (ret_val == NULL) {
        ret_val = new TruncatingFileLogger(path, this);
        m_loggers.insert(path, ret_val);
    }
    return ret_val;
}

TruncatingFileLogger *LogManager::get_logger(const QString &path) {
    return m_loggers.value(path, NULL);
}

LogAppender *LogManager::add_appender(const QString &module_name,
                                      TruncatingFileLogger *logger,
                                      LOG_LEVEL min_level) {
    LogAppender *ret_val = get_appender(module_name);
    if (ret_val == NULL) {
        ret_val = new LogAppender(module_name, logger, min_level);
        m_appenders.insert(module_name, ret_val);
    }
    return ret_val;
}

LogAppender *LogManager::get_appender(const QString &module_name) {
    return m_appenders.value(module_name, NULL);
}


QString LogManager::level_name(LOG_LEVEL lvl) {
    return m_level_names.value(lvl, QString("%1").arg(lvl));
}

/**************** APPENDER **********************/
LogAppender::LogAppender(const QString &module, TruncatingFileLogger *logger,
                         LOG_LEVEL min_level, LogAppender *parent_appender)
    : QObject(NULL)
    , m_module_name(module)
    , m_minimum_level(min_level)
    , m_parent_appender(parent_appender)
    , m_logger(logger)
{
}

QString LogAppender::module_name() {
    if (m_parent_appender) {
        return QString("%1.%2").arg(m_parent_appender->module_name())
                .arg(m_module_name);
    }
    return m_module_name;
}

void LogAppender::set_minimum_level(LOG_LEVEL lvl) {
    m_minimum_level = lvl;
    //LOGI << "Minimum log level set to" << m_manager->level_name(lvl);
}

void LogAppender::write(const QString &message, LOG_LEVEL lvl,
                        const QString &file, int lineno,
                        const QString &function) {
    QString out = QString("%1\t%2\t%3")
                  .arg(DT->get_log_manager()->level_name(lvl))
                  .arg(module_name())
                  .arg(message);
    if (m_logger) {
        m_logger->write(out, file, lineno, function);
    }
}

/**************** LOGGER **********************/
TruncatingFileLogger::TruncatingFileLogger(const QString &path, QObject *parent)
    : QObject(parent)
    , m_path(path)
    , m_file(0)
{
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
    }
}

TruncatingFileLogger::~TruncatingFileLogger() {
    if (m_file) {
        if (m_file->isOpen()) {
            m_file->flush();
            m_file->close();
        }
        m_file->deleteLater(); // the object not the file
    }
}

void TruncatingFileLogger::write(const QString &message,
                                 const QString &file, int lineno,
                                 const QString &func) {
    if (m_file && m_file->isWritable()) {
        QString stripped = message.trimmed();
        // make a string for this log level
        QDateTime now = QDateTime::currentDateTime();
        QString msg("%1 %2%3%4\n");
        msg = msg.arg(now.toString("yyyy-MMM-dd hh:mm:ss.zzz"))
              .arg(stripped).toAscii();

        QString location;
        QString function;
        if (!file.isEmpty()) {
            location = QString(" [%1:%2]").arg(file).arg(lineno);
        }
        if (!func.isEmpty()) {
            function = QString(" (%1)").arg(func);
        }
        msg = msg.arg(location);
        msg = msg.arg(function);
        m_file->write(msg.toAscii());
        m_file->flush();
    }
}

Streamer::Streamer(LogAppender *appender, LOG_LEVEL lvl, const QString &file,
                   int lineno, const QString &func)
    : m_appender(appender)
    , m_level(lvl)
    , m_buffer(QString())
    , m_dbg(&m_buffer)
    , m_file(file)
    , m_lineno(lineno)
    , m_function(func)
{}

void Streamer::write() {
    if (m_appender) {
        m_appender->write(m_buffer, m_level, m_file, m_lineno, m_function);
    } else {
        // this should be fatal to prevent us from making lots of bogus
        // appender names
        qCritical() << "LOG ENTRY FROM" << m_file << m_lineno << m_function <<
                "Has no valid log appender!";
        qFatal("FATAL ERROR: invalid log appender");
    }
}
