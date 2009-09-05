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

#include "qxtbasicstdloggerengine.h"
#include <QTextStream>
#include <QTime>

#define QXT_REQUIRED_LEVELS (QxtLogger::WarningLevel | QxtLogger::ErrorLevel | QxtLogger::CriticalLevel | QxtLogger::FatalLevel)

/*!
    \class QxtBasicSTDLoggerEngine QxtBasicSTDLoggerEngine
    \brief A basic STD logger engine.
    \ingroup QxtCore

    Example basic STD log output:
    \code
    [22:38:33.159] [Error] Unknown error
    [22:51:43.488] [Debug] What's going on?
                           Hi there!
    \endcode

    \sa QxtLogger
 */

class QxtBasicSTDLoggerEnginePrivate : public QxtPrivate<QxtBasicSTDLoggerEngine>
{
    QXT_DECLARE_PUBLIC(QxtBasicSTDLoggerEngine);

public:
    QxtBasicSTDLoggerEnginePrivate();

    QTextStream *errstream, *outstream;
};

QxtBasicSTDLoggerEnginePrivate::QxtBasicSTDLoggerEnginePrivate()
{
    errstream = new QTextStream(stderr);
    outstream = new QTextStream(stdout);
}

/*!
    Constructor.
 */
QxtBasicSTDLoggerEngine::QxtBasicSTDLoggerEngine()
{
    QXT_INIT_PRIVATE(QxtBasicSTDLoggerEngine);
#ifndef QT_NO_DEBUG
    setLogLevelsEnabled(QXT_REQUIRED_LEVELS);
#else
    setLogLevelsEnabled(QXT_REQUIRED_LEVELS | QxtLogger::DebugLevel);
#endif
    enableLogging();
}

/*!
    Destructor.
 */
QxtBasicSTDLoggerEngine::~QxtBasicSTDLoggerEngine()
{
    if (qxt_d().errstream)
    {
        qxt_d().errstream->flush();
        delete qxt_d().errstream;
        qxt_d().errstream = 0;
    }
    if (qxt_d().outstream)
    {
        qxt_d().outstream->flush();
        delete qxt_d().outstream;
        qxt_d().errstream = 0;
    }
}

/*!
    \reimp
 */
void QxtBasicSTDLoggerEngine::initLoggerEngine()
{
    return; // Should work out of the box!
}

/*!
    \reimp
 */
void QxtBasicSTDLoggerEngine::killLoggerEngine()
{
    return; // I do nothing.
}

/*!
    \reimp
 */
bool QxtBasicSTDLoggerEngine::isInitialized() const
{
    return qxt_d().errstream && qxt_d().outstream;
}

/*!
    Returns the stderr stream.
 */
QTextStream* QxtBasicSTDLoggerEngine::stdErrStream() const
{
    return qxt_d().errstream;
}

/*!
    Returns the stdout stream.
 */
QTextStream* QxtBasicSTDLoggerEngine::stdOutStream() const
{
    return qxt_d().outstream;
}

/*!
    \reimp
 */
void QxtBasicSTDLoggerEngine::setLogLevelEnabled(QxtLogger::LogLevels level, bool enable)
{
    QxtLoggerEngine::setLogLevelsEnabled(level | QXT_REQUIRED_LEVELS, enable);
    if (!enable) QxtLoggerEngine::setLogLevelsEnabled(QXT_REQUIRED_LEVELS);
}

/*!
    \reimp
 */
void QxtBasicSTDLoggerEngine::writeFormatted(QxtLogger::LogLevel level, const QList<QVariant> &msgs)
{
    switch (level)
    {
    case QxtLogger::ErrorLevel:
        writeToStdErr("Error", msgs);
        break;
    case QxtLogger::WarningLevel:
        writeToStdOut("Warning", msgs);
        break;
    case QxtLogger::CriticalLevel:
        writeToStdErr("Critical", msgs);
        break;
    case QxtLogger::FatalLevel:
        writeToStdErr("!!FATAL!!", msgs);
        break;
    case QxtLogger::TraceLevel:
        writeToStdOut("Trace", msgs);
        break;
    case QxtLogger::DebugLevel:
        writeToStdErr("DEBUG", msgs);
        break;
    case QxtLogger::InfoLevel:
        writeToStdOut("INFO", msgs);
        break;
    default:
        writeToStdOut("", msgs);
        break;
    }
}

/*!
    A helper function that actually writes to stderr.

    This function is called by QxtBasicSTDLoggerEngine. Reimplement this function when creating a subclass of QxtBasicSTDLoggerEngine.
 */
void QxtBasicSTDLoggerEngine::writeToStdErr(const QString &level, const QList<QVariant> &msgs)
{
    if (msgs.isEmpty()) return;
    QString header = '[' + QTime::currentTime().toString("hh:mm:ss.zzz") + "] [" + level + "] ";
    QString padding;
    QTextStream* errstream = stdErrStream();
    Q_ASSERT(errstream);
    *errstream << header;
    for (int i = 0; i < header.size(); i++) padding.append(" ");
    int count = 0;
    Q_FOREACH(const QVariant& out, msgs)
    {
        if (!out.isNull())
        {
            if (count != 0) *errstream << padding;
            *errstream << out.toString() << '\n';
        }
        count++;
    }
    *errstream << endl;
}

/*!
    A helper function that actually writes to stdout.

    This function is called by QxtBasicSTDLoggerEngine. Reimplement this function when creating a subclass of QxtBasicSTDLoggerEngine.
 */
void QxtBasicSTDLoggerEngine::writeToStdOut(const QString& level, const QList<QVariant> &msgs)
{
    /* Message format...
        [time] [error level] First message.....
                    second message
                    third message
    */
    if (msgs.isEmpty()) return;
    QString header = '[' + QTime::currentTime().toString("hh:mm:ss.zzz") + "] [" + level + "] ";
    QString padding;
    QTextStream* outstream = stdOutStream();
    Q_ASSERT(outstream);
    *outstream << header;
    for (int i = 0; i < header.size(); i++) padding.append(' ');
    int count = 0;
    Q_FOREACH(const QVariant& out, msgs)
    {
        if (!out.isNull())
        {
            if (count != 0) *outstream << padding;
            *outstream << out.toString() << '\n';
        }
        count++;
    }
    *outstream << endl;
}
