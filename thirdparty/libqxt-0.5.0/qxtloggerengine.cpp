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

#include "qxtloggerengine.h"

/*! \class QxtLoggerEngine QxtLoggerEngine
    \brief The parent class of all extended Engine Plugins.
    \ingroup QxtCore

    \section Overview
    QxtLoggerEngine is what gives the QxtLogger it's power.  While the Logger itself
    manages memory, logic, and log levels, it is the various Engine Plugins that
    do the actual work of logging the data.
*/

class QxtLoggerEnginePrivate : public QxtPrivate<QxtLoggerEngine>
{
    QXT_DECLARE_PUBLIC(QxtLoggerEngine);

public:
    QxtLoggerEnginePrivate();

    QxtLogger::LogLevels    bm_logLevel;
    bool                    b_isLogging;
};

QxtLoggerEnginePrivate::QxtLoggerEnginePrivate()
        : bm_logLevel(QxtLogger::AllLevels), b_isLogging(true)
{
}

/*!
    Constructor
 */
QxtLoggerEngine::QxtLoggerEngine()
{
    QXT_INIT_PRIVATE(QxtLoggerEngine);
}

/*!
    Destructor
 */
QxtLoggerEngine::~QxtLoggerEngine()
{
    QxtLogger::getInstance()->removeLoggerEngine(this);
}

/*!
    \fn virtual void QxtLoggerEngine::initLoggerEngine() = 0

    Initializes the logger engine.

    This function is called by QxtLogger. Reimplement this function when creating a subclass of QxtLoggerEngine.
 */

/*!
    \fn virtual void QxtLoggerEngine::killLoggerEngine() = 0

    Kills the logger engine.

    This function is called by QxtLogger. Reimplement this function when creating a subclass of QxtLoggerEngine.
 */

/*!
    \fn virtual bool QxtLoggerEngine::isInitialized() const = 0

    Returns \c true if the logger engine is initialized.

    This function is called by QxtLogger. Reimplement this function when creating a subclass of QxtLoggerEngine.
 */

/*!
    \fn virtual void QxtLoggerEngine::writeFormatted(QxtLogger::LogLevel level, const QList<QVariant>& messages) = 0

    Writes a formatted message.

    This function is called by QxtLogger. Reimplement this function when creating a subclass of QxtLoggerEngine.
 */

/*!
    Returns \c true if logging is enabled and \c false otherwise.
 */
bool QxtLoggerEngine::isLoggingEnabled() const
{
    return qxt_d().b_isLogging;
}

/*!
    Enables logging.
 */
void QxtLoggerEngine::enableLogging()
{
    setLoggingEnabled();
}

/*!
    Disables logging.
 */
void QxtLoggerEngine::disableLogging()
{
    setLoggingEnabled(false);
}

/*!
    Sets logging enabled or disabled.
 */
void QxtLoggerEngine::setLoggingEnabled(bool enable)
{
    qxt_d().b_isLogging = enable;
}

/*!
    Sets log levels enabled or disables.
 */
void QxtLoggerEngine::setLogLevelsEnabled(QxtLogger::LogLevels levels, bool enable)
{
    if (enable)
    {
        qxt_d().bm_logLevel |= levels;
    }
    else
    {
        qxt_d().bm_logLevel &= ~levels;
    }
}

/*!
    Enables log levels.
 */
void QxtLoggerEngine::enableLogLevels(QxtLogger::LogLevels levels)
{
    setLogLevelsEnabled(levels, true);
}

/*!
    Disables log levels.
 */
void QxtLoggerEngine::disableLogLevels(QxtLogger::LogLevels levels)
{
    setLogLevelsEnabled(levels, false);
}

/*!
    Returns \c true if log level is enabled and \c false otherwise.
 */
bool QxtLoggerEngine::isLogLevelEnabled(QxtLogger::LogLevel level) const
{
    return (qxt_d().bm_logLevel & level);
}
