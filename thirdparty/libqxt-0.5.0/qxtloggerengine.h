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

#ifndef QXTLOGGERENGINE_H
#define QXTLOGGERENGINE_H

#include <QString>
#include <QVariant>
#include <QIODevice>
#include <QFile>
#include "qxtlogger.h"
#include "qxtpimpl.h"

class QxtLoggerEnginePrivate;

class QXT_CORE_EXPORT QxtLoggerEngine
{
    QXT_DECLARE_PRIVATE(QxtLoggerEngine);

public:
    QxtLoggerEngine();
    virtual ~QxtLoggerEngine();

    virtual void    initLoggerEngine() = 0;
    virtual void    killLoggerEngine() = 0;
    virtual bool    isInitialized() const = 0;

    virtual void    writeFormatted(QxtLogger::LogLevel level, const QList<QVariant>& messages) = 0;

    virtual void    setLoggingEnabled(bool enable = true);
    bool            isLoggingEnabled() const;
    void            enableLogging();
    void            disableLogging();

    virtual void    setLogLevelsEnabled(QxtLogger::LogLevels levels, bool enable = true);
    bool            isLogLevelEnabled(QxtLogger::LogLevel level) const;
    void            enableLogLevels(QxtLogger::LogLevels levels);
    void            disableLogLevels(QxtLogger::LogLevels levels);
};

#endif // QXTLOGGERENGINE_H
