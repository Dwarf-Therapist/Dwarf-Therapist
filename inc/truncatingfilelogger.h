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
#ifndef TRUNCATINGFILELOGGER_H
#define TRUNCATINGFILELOGGER_H

#include <QtCore>
#include <QtDebug>
#include "dwarftherapist.h"

/*! The different log levels. Their value should go up as severity increases to
  aid in filtering.
  */
typedef enum {
    LL_TRACE    = 0,
    LL_DEBUG    = 10,
    LL_INFO     = 20,
    LL_WARN     = 30,
    LL_ERROR    = 40,
    LL_FATAL    = 50
} LOG_LEVEL;

class TruncatingFileLogger;
class LogAppender;
class LogManager;

/*! simple class that macros create on the stack. It works with an internal
  QDebug to format most types nicely into an internal string buffer. Its dtor
  write its buffer to the active logger. So you can make these on the stack
  and they will log on their way out of memory.
*/
class Streamer {
public:
    explicit Streamer(LogAppender *appender, LOG_LEVEL lvl,
                      const QString &file, int lineno, const QString &func);
    ~Streamer() {write();}
    QDebug &stream() {return m_dbg;}
private:
    void write();
    LogAppender *m_appender; //! parent logger
    LOG_LEVEL m_level; //! our logging level
    QString m_buffer; //! string storage
    QDebug m_dbg; //! our streamer backed by m_buffer

    /*! these three come from the __FILE__, __LINE__, and __FUNCTION__
      macros */
    QString m_file; //! the source file this message comes from
    int m_lineno; //! the line number in the source file
    QString m_function; //! the name of the function
};

/*! This class holds a QFile open as a log. Streamers will write to it while
  it remains open. You can change the accepted log level to filter down how
  noisy the system is
  */
class TruncatingFileLogger : public QObject {
Q_OBJECT
public:
    explicit TruncatingFileLogger(const QString &path, QObject *parent = 0);
    virtual ~TruncatingFileLogger();
    void write(const QString &message, const QString &file = QString(),
               int lineno = -1, const QString &function = QString());
private:
    QString m_path; // absolute path to the current logfile
    QFile *m_file; // the handle we use to log to
};

class LogAppender : public QObject {
    Q_OBJECT
public:
    explicit LogAppender(const QString &module, TruncatingFileLogger *logger,
                         LOG_LEVEL min_level, LogAppender *parent_appender = 0);
    virtual QString module_name();
    LOG_LEVEL minimum_level() {return m_minimum_level;}
    void set_minimum_level(LOG_LEVEL lvl);
    void write(const QString &message, LOG_LEVEL lvl,
               const QString &file = QString(), int lineno = -1,
               const QString &function = QString());
private:
    QString m_module_name;
    LOG_LEVEL m_minimum_level; // ignore any messages below this level
    LogAppender *m_parent_appender;
    TruncatingFileLogger *m_logger;
};

/*! class for managing the various active appenders and loggers and their
  runtime levels
  */
class LogManager : public QObject {
    Q_OBJECT
public:
    explicit LogManager(QObject *parent = 0);
    TruncatingFileLogger *add_logger(const QString &path);
    TruncatingFileLogger *get_logger(const QString &path=QString());

    LogAppender *add_appender(const QString &module_name,
                              TruncatingFileLogger *logger,
                              LOG_LEVEL min_level);
    LogAppender *get_appender(const QString &module_name);

    QString level_name(LOG_LEVEL lvl);
private:
    QHash<QString, TruncatingFileLogger*> m_loggers;
    QHash<QString, LogAppender*> m_appenders;
    QHash<LOG_LEVEL, QString> m_level_names;
};


// this will go get the opened log from the main application

#define GET_APPENDER(module) DT->get_log_manager()->get_appender(module)

#define GET_LOG_BY_LEVEL_AND_MODULE(level, module) \
if (GET_APPENDER(module) && GET_APPENDER(module)->minimum_level() > level); \
else Streamer(GET_APPENDER(module), level, __FILE__, __LINE__, __FUNCTION__).stream()

#define TRACE GET_LOG_BY_LEVEL_AND_MODULE(LL_TRACE, "core")
#define LOGD GET_LOG_BY_LEVEL_AND_MODULE(LL_DEBUG, "core")
#define LOGI GET_LOG_BY_LEVEL_AND_MODULE(LL_INFO, "core")
#define LOGW GET_LOG_BY_LEVEL_AND_MODULE(LL_WARN, "core")
#define LOGE GET_LOG_BY_LEVEL_AND_MODULE(LL_ERROR, "core")
#define FATAL GET_LOG_BY_LEVEL_AND_MODULE(LL_FATAL, "core")

#define LOG_D(module) GET_LOG_BY_LEVEL_AND_MODULE(LL_DEBUG, module)

#endif // TRUNCATINGFILELOGGER_H
