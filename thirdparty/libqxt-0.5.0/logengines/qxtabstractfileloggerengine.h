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

#ifndef QXTABSTRACTFILELOGGERENGINE_H
#define QXTABSTRACTFILELOGGERENGINE_H

#include "qxtabstractiologgerengine.h"
#include "qxtglobal.h"
#include "qxtpimpl.h"

class QxtAbstractFileLoggerEnginePrivate;

class QXT_CORE_EXPORT QxtAbstractFileLoggerEngine : public QxtAbstractIOLoggerEngine
{
    QXT_DECLARE_PRIVATE(QxtAbstractFileLoggerEngine);

public:
    QxtAbstractFileLoggerEngine(const QString &fileName, QIODevice::OpenMode mode);
    ~QxtAbstractFileLoggerEngine();

    virtual void    initLoggerEngine();
    virtual void    killLoggerEngine();
    virtual bool    isInitialized() const;

    virtual void    writeFormatted(QxtLogger::LogLevel level, const QList<QVariant> &messages);

    void    setLogFileName(const QString &fileName);
    QString logFileName() const;

protected:
    virtual void writeToFile(const QString &level, const QVariantList &messages) = 0;
};

#endif // QXTABSTRACTFILELOGGERENGINE_H
