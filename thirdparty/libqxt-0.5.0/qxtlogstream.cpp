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

#include "qxtlogstream.h"
#include "qxtlogstream_p.h"
#include "qxtlogger.h"

/*!
    \class QxtLogStream QxtLogStream
    \brief A logging stream.
    \ingroup QxtCore

    \sa QxtLogger
 */

QxtLogStreamPrivate::QxtLogStreamPrivate(QxtLogger *owner, QxtLogger::LogLevel level, const QList<QVariant> &data) : owner(owner), level(level), refcount(1), data(data)
{
    // Nothing to see here.
}

QxtLogStreamPrivate::~QxtLogStreamPrivate()
{
    owner->log(level, data);
}

/*!
    Constructor.
 */
QxtLogStream::QxtLogStream(QxtLogger *owner, QxtLogger::LogLevel level, const QList<QVariant> &data) : d(new QxtLogStreamPrivate(owner, level, data))
{
    // Nothing here either.
}

/*!
    Copy constructor.
 */
QxtLogStream::QxtLogStream(const QxtLogStream &other)
{
    d = other.d;
    d->refcount++;
}

/*!
    Destructor.
 */
QxtLogStream::~QxtLogStream()
{
    d->refcount--;
    if (d->refcount == 0) delete d;
}

/*!
    Stream operator.
 */
QxtLogStream& QxtLogStream::operator<< (const QVariant &value)
{
    d->data.append(value);
    return *this;
}
