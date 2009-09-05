/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtCore module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/

/*!
    \class QxtLogger QxtLogger
    \brief An easy to use, easy to extend logging tool.
    \ingroup QxtCore

    \section Overview
    QxtLogger is an easy to use, easy to extend, thread-safe  logging tool.  It was designed to be used "out of the box".
    \code
        #include <QxtLogger>
        ...
        QxtLogger::getInstance()->debug("Hi!"); // without using the macro
        qxtLog->debug("Hi!"); // using the macro
    \endcode
    \see getInstance()

    \section Usage
    QxtLogger is designed to work "out of the box".  The Logger itself is a singleton object that manages all of the logging
    that is requested.  It provides 8 methods to actually log content; they are listed from the most logically verbose to the
    most logically important:
        \li trace()
        \li debug()
        \li info()
        \li warning()
        \li error()
        \li critical()
        \li fatal()
        \li write()

    These named members only have meaning to the person who uses them.  For example, you could call qxtLog->trace() from
    many parts of a complicated, massively recursive function to trace it's output; use qxtLog->info() to log that
    an event such as "Logging has started" has happened; use qxtLog->fatal() when an unhandled exception is thrown.  Or,
    you could use qxtLog->write() for everything.

    Each of these members comes in two forms: the first takes up to ten QVariants (for moc compatibility), the second
    form takes a QList<QVariant>.  Thus, you can invoke the info() member in the following ways:

    \code
    // Using the 10-param members.
    qxtLog->info(15);
    qxtLog->info("I am a test");
    qxtLog->info(QTime::currentTime(), "something happened", 3.14);

    // Now with QList<QVariant>
    qxtLog->info(QList<QVariant>() << "test" << 15 << QTime::currentTime());
    \endcode

    The real power behind QxtLogger comes from telling it which log levels you actually want to see.  Calling qxtLog->enableAllLogLevels()
    can give you a lot of data if you need it.  But if you only want to see warnings and errors, qxtLog->setMinimumLogLevel(WarningLevel) might
    be more useful.

    \section Extending
    The functionality of QxtLogger can be extended by creating plugins derived from QxtLoggerEngine.  Logger Engines
    are the little workers that actually take the raw data, format it, and spit it out into meaningful forms.
*/

/*!
    \macro qxtLog
    \relates QxtLogger

    A global pointer referring to the unique logger object. It is
    equivalent to the pointer returned by the QxtLogger::instance() function.

    \sa QxtLogger::getInstance()
 */

/*!
    \fn void QxtLogger::loggerEngineAdded(const QString& engineName)

    This signal is emitted when an engine with \a engineName has been added.

    \sa loggerEngineRemoved()
 */

/*!
    \fn void QxtLogger::loggerEngineRemoved(const QString& engineName)

    This signal is emitted when an engine with \a engineName has been removed.

    \sa loggerEngineAdded()
 */

/*!
    \fn void QxtLogger::loggerEngineEnabled(const QString& engineName)

    This signal is emitted when an engine with \a engineName has been enabled.

    \sa loggerEngineDisabled()
 */

/*!
    \fn void QxtLogger::loggerEngineDisabled(const QString& engineName)

    This signal is emitted when an engine with \a engineName has been disabled.

    \sa loggerEngineEnabled()
 */

#include "qxtlogger.h"
#include "qxtlogger_p.h"
#include "qxtlogstream.h"
#include "logengines/qxtbasicstdloggerengine.h"
#include <QtDebug>
#include <QMutex>
#include <QMutexLocker>

/*******************************************************************************
Constructor for QxtLogger's private data
*******************************************************************************/
QxtLoggerPrivate::QxtLoggerPrivate()
{
    mut_lock = new QMutex(QMutex::Recursive);
}

/*******************************************************************************
Destructor for QxtLogger's private data
*******************************************************************************/
QxtLoggerPrivate::~QxtLoggerPrivate()
{
    Q_FOREACH(QxtLoggerEngine *eng, map_logEngineMap)
    {
        if (eng)
        {
            eng->killLoggerEngine();
            delete eng;
        }
    }
    delete mut_lock;
    mut_lock = NULL;
}

void QxtLoggerPrivate::log(QxtLogger::LogLevel level, const QList<QVariant>& msgList)
{
    Q_FOREACH(QxtLoggerEngine *eng, map_logEngineMap)
    {
        if (eng && eng->isInitialized() && eng->isLoggingEnabled() && eng->isLogLevelEnabled(level))
        {
            eng->writeFormatted(level, msgList);
        }
    }
}

void QxtLoggerPrivate::setQxtLoggerEngineMinimumLevel(QxtLoggerEngine *eng, QxtLogger::LogLevel level)
{
    QMutexLocker lock(mut_lock);
    if (!eng) return;
    (QxtLogger::TraceLevel    < level) ? eng->disableLogLevels(QxtLogger::TraceLevel)    : eng->enableLogLevels(QxtLogger::TraceLevel);
    (QxtLogger::DebugLevel    < level) ? eng->disableLogLevels(QxtLogger::DebugLevel)    : eng->enableLogLevels(QxtLogger::DebugLevel);
    (QxtLogger::InfoLevel     < level) ? eng->disableLogLevels(QxtLogger::InfoLevel)     : eng->enableLogLevels(QxtLogger::InfoLevel);
    (QxtLogger::WarningLevel     < level) ? eng->disableLogLevels(QxtLogger::WarningLevel)     : eng->enableLogLevels(QxtLogger::WarningLevel);
    (QxtLogger::ErrorLevel    < level) ? eng->disableLogLevels(QxtLogger::ErrorLevel)    : eng->enableLogLevels(QxtLogger::ErrorLevel);
    (QxtLogger::CriticalLevel < level) ? eng->disableLogLevels(QxtLogger::CriticalLevel) : eng->enableLogLevels(QxtLogger::CriticalLevel);
    (QxtLogger::FatalLevel    < level) ? eng->disableLogLevels(QxtLogger::FatalLevel)    : eng->enableLogLevels(QxtLogger::FatalLevel);
    (QxtLogger::WriteLevel    < level) ? eng->disableLogLevels(QxtLogger::WriteLevel)    : eng->enableLogLevels(QxtLogger::WriteLevel);
}

/*! \short Returns the named Engine.
 */
QxtLoggerEngine *QxtLogger::engine(const QString &engineName)
{
    if (! isLoggerEngine(engineName)) return 0;
    else return qxt_d().map_logEngineMap.value(engineName);
}

/*! \short Opens a stream to write a message to all Engines with the InfoLevel set.
    The parameterless logging functions return a QxtLogStream for use similar to qDebug().
    \code
    qxtLog->info() << "informational message";
    \endcode
*/
QxtLogStream QxtLogger::info()
{
    return stream(QxtLogger::InfoLevel);
}

/*! \short Opens a stream to write a message to all Engines with the TraceLevel set.
    The parameterless logging functions return a QxtLogStream for use similar to qDebug().
    \code
    qxtLog->trace() << "detailed trace message";
    \endcode
*/
QxtLogStream QxtLogger::trace()
{
    return stream(QxtLogger::TraceLevel);
}

/*! \short Opens a stream to write a message to all Engines with the ErrorLevel set.
    The parameterless logging functions return a QxtLogStream for use similar to qDebug().
    \code
    qxtLog->error() << "error message";
    \endcode
*/
QxtLogStream QxtLogger::error()
{
    return stream(QxtLogger::ErrorLevel);
}

/*! \short Opens a stream to write a message to all Engines with the WarningLevel set.
    The parameterless logging functions return a QxtLogStream for use similar to qDebug().
    \code
    qxtLog->warning() << "warning message";
    \endcode
*/
QxtLogStream QxtLogger::warning()
{
    return stream(QxtLogger::WarningLevel);
}

/*! \short Opens a stream to write a message to all Engines with the DebugLevel set.
    The parameterless logging functions return a QxtLogStream for use similar to qDebug().
    \code
    qxtLog->debug() << "debugging log message";
    \endcode
*/
QxtLogStream QxtLogger::debug()
{
    return stream(QxtLogger::DebugLevel);
}

/*! \short Opens a stream to write a message to all Engines with the CriticalLevel set.
    The parameterless logging functions return a QxtLogStream for use similar to qDebug().
    \code
    qxtLog->critical() << "critical error message";
    \endcode
*/
QxtLogStream QxtLogger::critical()
{
    return stream(QxtLogger::CriticalLevel);
}

/*! \short Opens a stream to write a message to all Engines with the FatalLevel set.
    The parameterless logging functions return a QxtLogStream for use similar to qDebug().
    \code
    qxtLog->fatal() << "fatal error message";
    \endcode
*/
QxtLogStream QxtLogger::fatal()
{
    return stream(QxtLogger::FatalLevel);
}

/*! \short Opens a stream to write a message to all Engines with the WriteLevel set.
    The parameterless logging functions return a QxtLogStream for use similar to qDebug().
    \code
    qxtLog->write() << "log message";
    \endcode
*/
QxtLogStream QxtLogger::write()
{
    return stream(QxtLogger::WriteLevel);
}


/*! \short Writes a message to all Engines with the InfoLevel set.
    The 10-parameter logging functions are designed to be used with Qt's Signals and Slots, since moc
    currently only accepts functions with up to 10 parameters.  They can take any value that
    QVariant can take as an argument.
*/
void QxtLogger::info(const QVariant &message, const QVariant &msg1, const QVariant &msg2, const QVariant &msg3, const QVariant &msg4, const QVariant &msg5, const QVariant &msg6, const QVariant &msg7, const QVariant &msg8 , const QVariant &msg9)
{
    QMutexLocker lock(qxt_d().mut_lock);
    QList<QVariant> args;
    args.push_back(message);
    if (!msg1.isNull()) args.push_back(msg1);
    if (!msg2.isNull()) args.push_back(msg2);
    if (!msg3.isNull()) args.push_back(msg3);
    if (!msg4.isNull()) args.push_back(msg4);
    if (!msg5.isNull()) args.push_back(msg5);
    if (!msg6.isNull()) args.push_back(msg6);
    if (!msg7.isNull()) args.push_back(msg7);
    if (!msg8.isNull()) args.push_back(msg8);
    if (!msg9.isNull()) args.push_back(msg9);

    info(args);
}


/*! \short Writes a message to all Engines with the TraceLevel set.
    The 10-parameter logging functions are designed to be used with Qt's Signals and Slots, since moc
    currently only accepts functions with up to 10 parameters.  They can take any value that
    QVariant can take as an argument.
*/
void QxtLogger::trace(const QVariant &message, const QVariant &msg1 , const QVariant &msg2 , const QVariant &msg3 , const QVariant &msg4 , const QVariant &msg5 , const QVariant &msg6 , const QVariant &msg7 , const QVariant &msg8 , const QVariant &msg9)
{
    QMutexLocker lock(qxt_d().mut_lock);
    QList<QVariant> args;
    args.push_back(message);
    if (!msg1.isNull()) args.push_back(msg1);
    if (!msg2.isNull()) args.push_back(msg2);
    if (!msg3.isNull()) args.push_back(msg3);
    if (!msg4.isNull()) args.push_back(msg4);
    if (!msg5.isNull()) args.push_back(msg5);
    if (!msg6.isNull()) args.push_back(msg6);
    if (!msg7.isNull()) args.push_back(msg7);
    if (!msg8.isNull()) args.push_back(msg8);
    if (!msg9.isNull()) args.push_back(msg9);

    trace(args);
}

/*! \short Writes a message to all Engines with the WarningLevel set.
    The 10-parameter logging functions are designed to be used with Qt's Signals and Slots, since moc
    currently only accepts functions with up to 10 parameters.  They can take any value that
    QVariant can take as an argument.
*/
void QxtLogger::warning(const QVariant &message, const QVariant &msg1 , const QVariant &msg2 , const QVariant &msg3 , const QVariant &msg4 , const QVariant &msg5 , const QVariant &msg6 , const QVariant &msg7 , const QVariant &msg8 , const QVariant &msg9)
{
    QMutexLocker lock(qxt_d().mut_lock);
    QList<QVariant> args;
    args.push_back(message);
    if (!msg1.isNull()) args.push_back(msg1);
    if (!msg2.isNull()) args.push_back(msg2);
    if (!msg3.isNull()) args.push_back(msg3);
    if (!msg4.isNull()) args.push_back(msg4);
    if (!msg5.isNull()) args.push_back(msg5);
    if (!msg6.isNull()) args.push_back(msg6);
    if (!msg7.isNull()) args.push_back(msg7);
    if (!msg8.isNull()) args.push_back(msg8);
    if (!msg9.isNull()) args.push_back(msg9);

    warning(args);
}

/*! \short Writes a message to all Engines with the ErrorLevel set.
    The 10-parameter logging functions are designed to be used with Qt's Signals and Slots, since moc
    currently only accepts functions with up to 10 parameters.  They can take any value that
    QVariant can take as an argument.
*/
void QxtLogger::error(const QVariant &message, const QVariant &msg1 , const QVariant &msg2 , const QVariant &msg3 , const QVariant &msg4 , const QVariant &msg5 , const QVariant &msg6 , const QVariant &msg7 , const QVariant &msg8 , const QVariant &msg9)
{
    QMutexLocker lock(qxt_d().mut_lock);
    QList<QVariant> args;
    args.push_back(message);
    if (!msg1.isNull()) args.push_back(msg1);
    if (!msg2.isNull()) args.push_back(msg2);
    if (!msg3.isNull()) args.push_back(msg3);
    if (!msg4.isNull()) args.push_back(msg4);
    if (!msg5.isNull()) args.push_back(msg5);
    if (!msg6.isNull()) args.push_back(msg6);
    if (!msg7.isNull()) args.push_back(msg7);
    if (!msg8.isNull()) args.push_back(msg8);
    if (!msg9.isNull()) args.push_back(msg9);

    error(args);
}

/*! \short Writes a message to all Engines with the DebugLevel set.
    The 10-parameter logging functions are designed to be used with Qt's Signals and Slots, since moc
    currently only accepts functions with up to 10 parameters.  They can take any value that
    QVariant can take as an argument.
*/
void QxtLogger::debug(const QVariant &message, const QVariant &msg1 , const QVariant &msg2 , const QVariant &msg3 , const QVariant &msg4 , const QVariant &msg5 , const QVariant &msg6 , const QVariant &msg7 , const QVariant &msg8 , const QVariant &msg9)
{
    QMutexLocker lock(qxt_d().mut_lock);
    QList<QVariant> args;
    args.push_back(message);
    if (!msg1.isNull()) args.push_back(msg1);
    if (!msg2.isNull()) args.push_back(msg2);
    if (!msg3.isNull()) args.push_back(msg3);
    if (!msg4.isNull()) args.push_back(msg4);
    if (!msg5.isNull()) args.push_back(msg5);
    if (!msg6.isNull()) args.push_back(msg6);
    if (!msg7.isNull()) args.push_back(msg7);
    if (!msg8.isNull()) args.push_back(msg8);
    if (!msg9.isNull()) args.push_back(msg9);

    debug(args);
}

/*! \short Writes a message to all Engines with the WriteLevel set.
    The 10-parameter logging functions are designed to be used with Qt's Signals and Slots, since moc
    currently only accepts functions with up to 10 parameters.  They can take any value that
    QVariant can take as an argument.
*/
void QxtLogger::write(const QVariant &message, const QVariant &msg1 , const QVariant &msg2, const QVariant &msg3 , const QVariant &msg4 , const QVariant &msg5 , const QVariant &msg6 , const QVariant &msg7 , const QVariant &msg8 , const QVariant &msg9)
{
    QMutexLocker lock(qxt_d().mut_lock);
    QList<QVariant> args;
    args.push_back(message);
    if (!msg1.isNull()) args.push_back(msg1);
    if (!msg2.isNull()) args.push_back(msg2);
    if (!msg3.isNull()) args.push_back(msg3);
    if (!msg4.isNull()) args.push_back(msg4);
    if (!msg5.isNull()) args.push_back(msg5);
    if (!msg6.isNull()) args.push_back(msg6);
    if (!msg7.isNull()) args.push_back(msg7);
    if (!msg8.isNull()) args.push_back(msg8);
    if (!msg9.isNull()) args.push_back(msg9);

    write(args);
}

/*! \short Writes a message to all Engines with the CriticalLevel set.
    The 10-parameter logging functions are designed to be used with Qt's Signals and Slots, since moc
    currently only accepts functions with up to 10 parameters.  They can take any value that
    QVariant can take as an argument.
*/
void QxtLogger::critical(const QVariant &message, const QVariant &msg1 , const QVariant &msg2 , const QVariant &msg3 , const QVariant &msg4 , const QVariant &msg5 , const QVariant &msg6 , const QVariant &msg7 , const QVariant &msg8 , const QVariant &msg9)
{
    QMutexLocker lock(qxt_d().mut_lock);
    QList<QVariant> args;
    args.push_back(message);
    if (!msg1.isNull()) args.push_back(msg1);
    if (!msg2.isNull()) args.push_back(msg2);
    if (!msg3.isNull()) args.push_back(msg3);
    if (!msg4.isNull()) args.push_back(msg4);
    if (!msg5.isNull()) args.push_back(msg5);
    if (!msg6.isNull()) args.push_back(msg6);
    if (!msg7.isNull()) args.push_back(msg7);
    if (!msg8.isNull()) args.push_back(msg8);
    if (!msg9.isNull()) args.push_back(msg9);

    critical(args);
}

/*! \short Writes a message to all Engines with the FatalLevel set.
    The 10-parameter logging functions are designed to be used with Qt's Signals and Slots, since moc
    currently only accepts functions with up to 10 parameters.  They can take any value that
    QVariant can take as an argument.
*/
void QxtLogger::fatal(const QVariant &message, const QVariant &msg1 , const QVariant &msg2 , const QVariant &msg3 , const QVariant &msg4 , const QVariant &msg5 , const QVariant &msg6 , const QVariant &msg7 , const QVariant &msg8 , const QVariant &msg9)
{
    QMutexLocker lock(qxt_d().mut_lock);
    QList<QVariant> args;
    args.push_back(message);
    if (!msg1.isNull()) args.push_back(msg1);
    if (!msg2.isNull()) args.push_back(msg2);
    if (!msg3.isNull()) args.push_back(msg3);
    if (!msg4.isNull()) args.push_back(msg4);
    if (!msg5.isNull()) args.push_back(msg5);
    if (!msg6.isNull()) args.push_back(msg6);
    if (!msg7.isNull()) args.push_back(msg7);
    if (!msg8.isNull()) args.push_back(msg8);
    if (!msg9.isNull()) args.push_back(msg9);

    fatal(args);
}

/*! \short Writes a message to all Engines with the InfoLevel set.
    The 1-parameter logging messages can take any number of arguments in the
    form of a QList<QVariant>, or QList<QVariant>.
*/
void QxtLogger::info(const QList<QVariant> &args)
{
    log(QxtLogger::InfoLevel, args);
}

/*! \short Writes a message to all Engines with the TraceLevel set.
    The 1-parameter logging messages can take any number of arguments in the
    form of a QList<QVariant>, or QList<QVariant>.
*/
void QxtLogger::trace(const QList<QVariant> &args)
{
    log(QxtLogger::TraceLevel, args);
}

/*! \short Writes a message to all Engines with the WarningLevel set.
    The 1-parameter logging messages can take any number of arguments in the
    form of a QList<QVariant>, or QList<QVariant>.
*/
void QxtLogger::warning(const QList<QVariant> &args)
{
    log(QxtLogger::WarningLevel, args);
}

/*! \short Writes a message to all Engines with the ErrorLevel set.
    The 1-parameter logging messages can take any number of arguments in the
    form of a QList<QVariant>, or QList<QVariant>.
*/
void QxtLogger::error(const QList<QVariant> &args)
{
    log(QxtLogger::ErrorLevel, args);
}

/*! \short Writes a message to all Engines with the DebugLevel set.
    The 1-parameter logging messages can take any number of arguments in the
    form of a QList<QVariant>, or QList<QVariant>.
*/
void QxtLogger::debug(const QList<QVariant> &args)
{
    log(QxtLogger::DebugLevel, args);
}

/*! \short Writes a message to all Engines with the CriticalLevel set.
    The 1-parameter logging messages can take any number of arguments in the
    form of a QList<QVariant>, or QList<QVariant>.
*/
void QxtLogger::critical(const QList<QVariant> &args)
{
    log(QxtLogger::CriticalLevel, args);
}

/*! \short Writes a message to all Engines with the FatalLevel set.
    The 1-parameter logging messages can take any number of arguments in the
    form of a QList<QVariant>, or QList<QVariant>.
*/
void QxtLogger::fatal(const QList<QVariant> &args)
{
    log(QxtLogger::FatalLevel, args);
}

/*! \short Writes a message to all Engines with the WriteLevel set.
    The 1-parameter logging messages can take any number of arguments in the
    form of a QList<QVariant>, or QList<QVariant>.
*/
void QxtLogger::write(const QList<QVariant> &args)
{
    log(QxtLogger::WriteLevel, args);
}

/*! A Generic Logging Function that takes a LogLevel and a QList<QVariant> of messages

    This function is provided for convenience.
*/
void QxtLogger::log(LogLevel level, const QList<QVariant>& args)
{
    /*
    QMutexLocker lock(qxt_d().mut_lock);
    qxt_d().log(level, msgList);
    */
    QMetaObject::invokeMethod(&qxt_d(), "log", Qt::AutoConnection, Q_ARG(QxtLogger::LogLevel, level), Q_ARG(QList<QVariant>, args));
}

/*******************************************************************************
    Message Handler for qdebug, qerror, qwarning, etc...
    When QxtLogger is enabled as a message handler for Qt, this function
    redirects message calls like qdebug, qwarning, qfatal.
    \see QxtLogger::installAsMessageHandler
    \see QxtLogger::removeAsMessageHandler
*******************************************************************************/
void QxtLoggerMessageHandler(QtMsgType type, const char *msg)
{
    switch (type)
    {
    case QtDebugMsg:
        QxtLogger::getInstance()->debug(msg, "qdebug");
        break;
    case QtWarningMsg:
        QxtLogger::getInstance()->warning(msg, "qwarning");
        break;
    case QtCriticalMsg:
        QxtLogger::getInstance()->critical(msg, "qcritical");
        break;
    case QtFatalMsg:
        QxtLogger::getInstance()->fatal(msg, "qfatal");
        abort();
    }
}

/*! \short Installs QxtLogger as Qt's message handler.
    This will make Qt macros use QxtLogger instead of the default
    mechanism:
        \li qDebug()
        \li qWarning()
        \li qCritical()
        \li qFatal() will call abort() and terminate your application.
*/
void QxtLogger::installAsMessageHandler()
{
    QMutexLocker lock(qxt_d().mut_lock);
    qInstallMsgHandler(QxtLoggerMessageHandler);
}

/*! \short Tells Qt to use it's own message handling again.
*/
void QxtLogger::removeAsMessageHandler()
{
    QMutexLocker lock(qxt_d().mut_lock);
    qInstallMsgHandler(0);
}

/*****************************************************************************
    Constructor
    Private, since QxtLogger is a singleton.
*****************************************************************************/
QxtLogger::QxtLogger()
{
    QXT_INIT_PRIVATE(QxtLogger);
    qRegisterMetaType<QxtLogger::LogLevel>();
    qRegisterMetaType<QxtLogger::LogLevels>();
    addLoggerEngine("DEFAULT", new QxtBasicSTDLoggerEngine);
    setMinimumLevel("DEFAULT", QxtLogger::InfoLevel);
}

/***************************************************************************//**
    Destructor.
    The Destructor for QxtLogger iterates through all the currently installed
    QxtLoggerEngines, calls their killLoggerEngine functions through QxtLoggerEngine::killLoggerEngine(),
    and then deletes them from the map.
*******************************************************************************/
QxtLogger::~QxtLogger()
{
    // implicit destruction only
}

/*! \short Returns a pointer to the instance of the Logger.
    QxtLogger is implemented as a singleton, a single object, that
    manages all of the logging done in an application.  The easiest way
    to use it is by calling the qxtLog macro:

    \code
    #include <QxtLogger>
    ...
    qxtLog->info("I can log things!");
    \endcode

    qxtLog expands to QxtLogger::getInstance, which returns a pointer to the logger.

    QxtLogger manages it's own memory, so please remember the second rule of pointers:
    don't delete it unless you instantiated it yourself.
    \code
    delete qxtLog; // Will horribly crash your app, and possibly your system
    \endcode

*/
QxtLogger *QxtLogger::getInstance()
{
    static QxtLogger objectInstance;
    return &objectInstance;
}

/*! \short Returns a QString of the given LogLevel.
    This function is provided for convenience.
    */
QString QxtLogger::logLevelToString(LogLevel level)
{
    switch (level)
    {
    case TraceLevel:
        return "TraceLevel";
    case DebugLevel:
        return "DebugLevel";
    case InfoLevel:
        return "InfoLevel";
    case WarningLevel:
        return "WarningLevel";
    case ErrorLevel:
        return "ErrorLevel";
    case CriticalLevel:
        return "CriticalLevel";
    case FatalLevel:
        return "FatalLevel";
    case WriteLevel:
        return "WriteLevel";
    case AllLevels:
        return "AllLevels";
    default:
        return "NoLevels";
    }
}

/*! \short Returns a LogLevel for the given string, or QxtLogger::NoLevels if invalid.
    This function is provided for convenience.
*/
QxtLogger::LogLevel QxtLogger::stringToLogLevel(const QString& level)
{
    if (level.toLower() == "alllevels") return QxtLogger::AllLevels;
    else if (level.toLower() == "writelevel") return QxtLogger::TraceLevel;
    else if (level.toLower() == "fatallevel") return QxtLogger::DebugLevel;
    else if (level.toLower() == "criticallevel") return QxtLogger::InfoLevel;
    else if (level.toLower() == "errorlevel") return QxtLogger::WarningLevel;
    else if (level.toLower() == "warnlevel") return QxtLogger::ErrorLevel;
    else if (level.toLower() == "infolevel") return QxtLogger::CriticalLevel;
    else if (level.toLower() == "debuglevel") return QxtLogger::FatalLevel;
    else if (level.toLower() == "tracelevel") return QxtLogger::WriteLevel;
    else return QxtLogger::NoLevels;
}

/*! \short Enables the given LogLevels across all Engines.
    \code
    qxtLog->enableLogLevels(QxtLogger::NoLevels);
    qxtLog->write("I don't do anything!");
    qxtLog->enableLogLevels(QxtLogger::AllLevels);
    qxtLog->write("Hi there!");
    \endcode
    \param levels   A bitmask of LogLevels
*/
void QxtLogger::enableLogLevels(LogLevels levels)
{
    QMutexLocker lock(qxt_d().mut_lock);
    if (qxt_d().map_logEngineMap.empty()) return;

    Q_FOREACH(QxtLoggerEngine *eng, qxt_d().map_logEngineMap)
    {
        if (eng)
        {
            eng->enableLogLevels(levels);
        }
    }
}

/*! \short Returns a reference to a refcounted stream.
    This is still in its early phases and is in dire need of testing and debugging.
    \code
    QxtLogger::stream(QxtLogger::WriteLevel) << "This should write stuff" << 1.5 << QString();
    \endcode
*/
QxtLogStream QxtLogger::stream(LogLevel level)
{
    return QxtLogStream(this, level, QList<QVariant>());
}

/*! \short Enables the given LogLevels on a named Engine.
    This will use the given engine name to tell a loaded QxtLoggerEngine
    what LogLevels it should enable.
    \code
    qxtLog->addLoggerEngine("test", "libTestLogger");
    qxtLog->enableLogLevels("test", QxtLoger::AllLevels);
    qxtLog->write("You can see me through your 'test' logger now!");
    \endcode

    \param engineName The name of a QxtLoggerEngine.
    \param levels   A LogLevel or LogLevels to enable.
    \see addLoggerEngine()
*******************************************************************************/
void QxtLogger::enableLogLevels(const QString &engineName, LogLevels levels)
{
    QMutexLocker lock(qxt_d().mut_lock);
    if (qxt_d().map_logEngineMap.contains(engineName))
    {
        if (qxt_d().map_logEngineMap.value(engineName))
        {
            qxt_d().map_logEngineMap.value(engineName)->enableLogLevels(levels);
        }
    }
}
/*! \short Turns on all log levels for a named engine.
    This is a function provided for convenience, and is equivalent to
    calling:
    \code
    qxtLog->enableLogLevels("test", QxtLogger::AllLevels);
    \endcode
*/
void QxtLogger::enableAllLogLevels(const QString &engineName)
{
    enableLogLevels(engineName, QxtLogger::AllLevels);
}

/*! \short Turns on all log levels for all engines.
    This is a function provided for convenience, and is equivalent to
    calling:
    \code
    qxtLog->enableLogLevels(QxtLogger::AllLevels);
    \endcode
*/
void QxtLogger::enableAllLogLevels()
{
    enableLogLevels(QxtLogger::AllLevels);
}

/*! \short Enables a named engine if it is currently disabled.
    \param engineName the name of a QxtLoggerEngine.
    */
void QxtLogger::enableLoggerEngine(const QString &engineName)
{
    QMutexLocker lock(qxt_d().mut_lock);
    if (qxt_d().map_logEngineMap.contains(engineName))
    {
        if (qxt_d().map_logEngineMap.value(engineName))
        {
            qxt_d().map_logEngineMap.value(engineName)->enableLogging();
            emit loggerEngineEnabled(engineName);
        }
    }
}

/*! \short Unflags the given LogLevels across all Engines.
    Disables the given LogLevel across all QxtLoggersEngines.  Note that some

    \param levels A LogLevel or LogLevels to disable.
*/
void QxtLogger::disableLogLevels(LogLevels levels)
{
    QMutexLocker lock(qxt_d().mut_lock);
    if (qxt_d().map_logEngineMap.empty()) return;
    Q_FOREACH(QxtLoggerEngine *eng, qxt_d().map_logEngineMap)
    {
        if (eng)
        {
            eng->disableLogLevels(levels);
        }
    }
}

/*! \short Disables the named Engine.
    Disables the the named QxtLoggerEngine if it exists.

    \param engineName The name of a log Engine to disable.
*/
void QxtLogger::disableLoggerEngine(const QString &engineName)
{
    QMutexLocker lock(qxt_d().mut_lock);
    if (qxt_d().map_logEngineMap.contains(engineName))
    {
        if (qxt_d().map_logEngineMap.value(engineName))
        {
            qxt_d().map_logEngineMap.value(engineName)->disableLogging();
            emit loggerEngineDisabled(engineName);
        }
    }
}

/*! \short Sets the minimumlog level for all Engines, as well as the levels above it.
    \param level The single LogLevel to set as minimum.
*/
void QxtLogger::setMinimumLevel(LogLevel level)
{
    QMutexLocker lock(qxt_d().mut_lock);
    Q_FOREACH(QxtLoggerEngine *eng, qxt_d().map_logEngineMap)
    {
        if (eng)
        {
            qxt_d().setQxtLoggerEngineMinimumLevel(eng, level);
        }
    }
}

/*! \short Sets the minimumlog level for the named Engine, as well as the levels above it.
    \param engineName The name of a QxtLoggerEngine.
    \param level The single LogLevel to set as minimum.
*/
void QxtLogger::setMinimumLevel(const QString &engineName, LogLevel level)
{
    QMutexLocker lock(qxt_d().mut_lock);
    if (qxt_d().map_logEngineMap.contains(engineName))
    {
        if (qxt_d().map_logEngineMap.value(engineName))
        {
            qxt_d().setQxtLoggerEngineMinimumLevel(qxt_d().map_logEngineMap.value(engineName), level);
        }
    }
}

/*! \short Calls QxtLoggerEngine::initLoggerEngine() for the named Engine.
    Some QxtLoggerEngine plugins might require additional initialization.  Check the documentation
    for your plugin.  Most basic plugins will not require special tasks.
    \param engineName The name of a QxtLoggerEngine.
*/
void QxtLogger::initLoggerEngine(const QString &engineName)
{
    QMutexLocker lock(qxt_d().mut_lock);
    if (qxt_d().map_logEngineMap.contains(engineName))
    {
        if (qxt_d().map_logEngineMap.value(engineName))
        {
            qxt_d().map_logEngineMap.value(engineName)->initLoggerEngine();
        }
    }
}

/*! \short Calls QxtLoggerEngine::killLoggerEngine() for the named Engine.
    Some QxtLoggerEngine plugins might require special cleanup before destruction.
    Check the documentation for your plugin.  Most basic plugins will not require this.
    \param engineName The name of a QxtLoggerEngine.
*/
void QxtLogger::killLoggerEngine(const QString &engineName)
{
    QMutexLocker lock(qxt_d().mut_lock);
    if (qxt_d().map_logEngineMap.contains(engineName))
    {
        if (qxt_d().map_logEngineMap.value(engineName))
        {
            qxt_d().map_logEngineMap.value(engineName)->killLoggerEngine();
        }
    }
}
/*! \short Checks if the named Engine has the given LogLevel enabled.
    \param engineName  The name of a QxtLoggerEngine to query
    \param level           A LogLevel or LogLevels to disable.
    \ret                   Returns true or false.
*/
bool QxtLogger::isLogLevelEnabled(const QString &engineName, LogLevel level) const
{
    QMutexLocker lock(qxt_d().mut_lock);
    if (qxt_d().map_logEngineMap.contains(engineName))
    {
        return qxt_d().map_logEngineMap.value(engineName)->isLogLevelEnabled(level);
    }
    else return false;
}

/*! \short Disables the given LogLevel across the named QxtLoggersEngines.
    \param engineName The name of a QxtLoggerEngine.
    \param level   A LogLevel or LogLevels to disable.
*/
void QxtLogger::disableLogLevels(const QString &engineName, LogLevels levels)
{
    QMutexLocker lock(qxt_d().mut_lock);
    if (qxt_d().map_logEngineMap.contains(engineName))
    {
        if (qxt_d().map_logEngineMap.value(engineName))
        {
            qxt_d().map_logEngineMap.value(engineName)->disableLogLevels(levels);
        }
    }
}

/*! \short Disables all log levels for the named Engine.
    \param engineName The name of an Engine.
*/
void QxtLogger::disableAllLogLevels(const QString &engineName)
{
    disableLogLevels(engineName, QxtLogger::AllLevels);
}

/*! \short Disables all log levels for all named Engines.
*/
void QxtLogger::disableAllLogLevels()
{
    disableLogLevels(QxtLogger::AllLevels);
}

/*! \short Gives QxtLogger an already-instantiated QxtLoggerEngine to use.
    addLoggerEngine inserts a subclass of QxtLoggerEngine for QxtLogger
        to manage.  QxtLogger takes ownership of the engine and will
        manage memory on its own.
    \code
    #include <QxtLogger>
    ...
    class MyLoggerEngine : public QxtLoggerEngine;
    ...
    qxtLog->addLoggerEngine("my engine", new MyLoggerEngine);
    \endcode
    \see QxtLoggerEngine
*/
void QxtLogger::addLoggerEngine(const QString &engineName, QxtLoggerEngine *engine)
{
    QMutexLocker lock(qxt_d().mut_lock);
    if (!qxt_d().map_logEngineMap.contains(engineName) && engine)
    {
        qxt_d().map_logEngineMap.insert(engineName, engine);
        emit loggerEngineAdded(engineName);
    }
}

/*
\short Gives QxtLogger an already-instantiated QLibrary.
    This is an overloaded functionand not the preferred method of adding Engines.
    It is useful to load plugins that are not in the applications Path.
    \code
    QLibrary *lib = new QLibrary("/path/to/plugin");
    qxtLog->addLogger("my engine", lib);
    \endcode

void QxtLogger::addLoggerEngine(const QString &engineName, QLibrary *engineLib)
{
    QMutexLocker lock(qxt_d().mut_lock);
    typedef QxtLoggerEngine* (*LibInstance)();
    LibInstance instance = (LibInstance)engineLib->resolve("getInstance");
    qWarning() << engineLib->errorString();

    if (!instance) return;
    else if (!qxt_d().map_logEngineMap.contains(engineName) && engineLib)
    {
        qxt_d().map_logEngineMap.insert(engineName, instance());
        emit loggerEngineAdded(engineName);
    }
}

\short Loads an Engine from a plugin in the current path.
    \code
    qxtLog->addLogger("my lib", "libQxtSomeKindOfLoggerEngine");
    \endcode
    \param engineName The name to give this QxtLoggerEngine.
    \param libName The name of the library to load.

void QxtLogger::addLoggerEngine(const QString &engineName, const QString &libName)
{
    QLibrary engineLib(libName);

        addLoggerEngine(engineName, &engineLib);
}
*/
/*! \short Remove the named Engine from use.
*/
void QxtLogger::removeLoggerEngine(const QString &engineName)
{
    QMutexLocker lock(qxt_d().mut_lock);
    QxtLoggerEngine* eng = takeLoggerEngine(engineName);
    if (!eng) return;
    eng->killLoggerEngine();
    delete eng;
}

/*! \short Remove the Engine from use.
 */
void QxtLogger::removeLoggerEngine(QxtLoggerEngine *engine)
{
    QMutexLocker lock(qxt_d().mut_lock);
    Q_FOREACH(const QString& i, this->qxt_d().map_logEngineMap.keys(engine))
    {
        takeLoggerEngine(i); // return value ignored
    }
}

/*! \short Take the named Engine.
 */
QxtLoggerEngine *QxtLogger::takeLoggerEngine(const QString &engineName)
{
    QMutexLocker lock(qxt_d().mut_lock);
    QxtLoggerEngine *eng = qxt_d().map_logEngineMap.take(engineName);
    if (!eng) return NULL;
    emit loggerEngineRemoved(engineName);
    return eng;
}

/*! \short Retuns a QStringList containing the names of all loaded Engines being managed by QxtLogger.
    \ret QStringList engine names.
*/
QStringList QxtLogger::allLoggerEngines() const
{
    QMutexLocker lock(qxt_d().mut_lock);
    return qxt_d().map_logEngineMap.keys();
}

/*! \short Retuns a QStringList containing the names of all loaded Engines that are currently enabled.
    \return QStringList engine names.
*/
QStringList QxtLogger::allEnabledLoggerEngines() const
{
    QMutexLocker lock(qxt_d().mut_lock);
    QStringList engineNames = qxt_d().map_logEngineMap.keys();
    QStringList result;
    Q_FOREACH(const QString& name, engineNames)
    {
        if (qxt_d().map_logEngineMap.value(name)->isLoggingEnabled()) result.append(name);
    }
    return result;
}

/*! \short Retuns a QStringList containing the names of all loaded Engines that have currently certain log level enabled.
    \return QStringList engine names.
 */
QStringList QxtLogger::allEnabledLoggerEngines(LogLevel level) const
{
    QMutexLocker lock(qxt_d().mut_lock);
    QStringList engineNames = qxt_d().map_logEngineMap.keys();
    QStringList result;
    Q_FOREACH(const QString& name, engineNames)
    {
        QxtLoggerEngine* engine = qxt_d().map_logEngineMap.value(name);
        if (engine->isLoggingEnabled() && engine->isLogLevelEnabled(level))
        {
            result.append(name);
        }
    }
    return result;
}

/*! \short Retuns a QStringList containing the names of all loaded Engines that are currently disabled.
    \return QStringList engine names.
*/
QStringList QxtLogger::allDisabledLoggerEngines() const
{
    QMutexLocker lock(qxt_d().mut_lock);
    QStringList sl_engineNames = qxt_d().map_logEngineMap.keys();
    QStringList result;
    Q_FOREACH(const QString& name, sl_engineNames)
    {
        if (!qxt_d().map_logEngineMap.value(name)->isLoggingEnabled()) result.append(name);
    }
    return result;
}

/*! \short Checks if the given string names a currently loaded Engine.
    \return True or false.
*/
bool QxtLogger::isLoggerEngine(const QString &engineName) const
{
    QMutexLocker lock(qxt_d().mut_lock);
    return qxt_d().map_logEngineMap.contains(engineName);
}

/*! \short Checks if the named engine is currently enabled.
    \return True or false
*/
bool QxtLogger::isLoggerEngineEnabled(const QString &engineName) const
{
    QMutexLocker lock(qxt_d().mut_lock);
    return (qxt_d().map_logEngineMap.contains(engineName) && qxt_d().map_logEngineMap.value(engineName)->isLoggingEnabled());
}
