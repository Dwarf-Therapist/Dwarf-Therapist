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

#include "qxtabstractfileloggerengine.h"

/*!
    \class QxtAbstractFileLoggerEngine QxtAbstractFileLoggerEngine
    \brief The base class of file logger engines.
    \ingroup QxtCore

    \sa QxtLogger
 */

class QxtAbstractFileLoggerEnginePrivate : public QxtPrivate<QxtAbstractFileLoggerEngine>
{
    QXT_DECLARE_PUBLIC(QxtAbstractFileLoggerEngine);

public:
    QString logFile;
    QIODevice::OpenMode mode;
};

/*!
    Constructs a QxtAbstractFileLoggerEngine with file name.
 */
QxtAbstractFileLoggerEngine::QxtAbstractFileLoggerEngine(const QString &fileName, QIODevice::OpenMode mode)
        : QxtAbstractIOLoggerEngine(0)
{
    QXT_INIT_PRIVATE(QxtAbstractFileLoggerEngine);
    qxt_d().mode = mode;
    setLogFileName(fileName);
}

/*!
    Destructs the file logger engine.
 */
QxtAbstractFileLoggerEngine::~QxtAbstractFileLoggerEngine()
{
    killLoggerEngine();
}

/*!
    \reimp
 */
void QxtAbstractFileLoggerEngine::initLoggerEngine()
{
    // Are we already logging to a file?  If so, close it and disable logging.
    killLoggerEngine();

    // If the file exists, check if we can write to it.  If we can, we append!
    // If the file doesn't exits, try to create it.
    // If we can't write to a file, disable this plugin.
    if (qxt_d().logFile.isEmpty()) return;  // if there's no filename, disable the engine until one is given

    setDevice(new QFile(qxt_d().logFile));
    if (!device()->open(qxt_d().mode)
            || !device()->isWritable())
    {
        killLoggerEngine();
        return;
    }

    enableLogging();
}

/*!
    \reimp
 */
void QxtAbstractFileLoggerEngine::killLoggerEngine()
{
    if (device() != 0)
    {
        if (device()->isOpen()) device()->close();
        delete device();
        setDevice(0);
    }
}

/*!
    \reimp
 */
bool QxtAbstractFileLoggerEngine::isInitialized() const
{
    return (device() != 0);
}

/*!
    \reimp
 */
void QxtAbstractFileLoggerEngine::writeFormatted(QxtLogger::LogLevel level, const QList<QVariant> &messages)
{
    switch (level)
    {
    case QxtLogger::ErrorLevel:
        writeToFile("Error", messages);
        break;
    case QxtLogger::WarningLevel:
        writeToFile("Warning", messages);
        break;
    case QxtLogger::CriticalLevel:
        writeToFile("Critical", messages);
        break;
    case QxtLogger::FatalLevel:
        writeToFile("Fatal", messages);
        break;
    case QxtLogger::TraceLevel:
        writeToFile("Trace", messages);
        break;
    case QxtLogger::DebugLevel:
        writeToFile("Debug", messages);
        break;
    case QxtLogger::InfoLevel:
        writeToFile("Info", messages);
        break;
    default:
        writeToFile(QString(), messages);
        break;
    }
}

/*!
    Sets the log file name.
 */
void QxtAbstractFileLoggerEngine::setLogFileName(const QString &fileName)
{
    qxt_d().logFile = fileName;
    initLoggerEngine();
}

/*!
    Returns the log file name.
 */
QString QxtAbstractFileLoggerEngine::logFileName() const
{
    return qxt_d().logFile;
}

/*!
    \fn virtual void QxtAbstractFileLoggerEngine::writeToFile( const QString &level, const QVariantList &messages ) = 0

    Writes to file.

    This function is called by QxtAbstractFileLoggerEngine. Reimplement this function when creating a subclass of QxtAbstractFileLoggerEngine.
 */
