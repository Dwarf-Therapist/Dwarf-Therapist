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

#include "qxtxmlfileloggerengine.h"
#include <QTime>

/*!
    \class QxtXmlFileLoggerEngine QxtXmlFileLoggerEngine
    \brief An XML file logger engine.
    \ingroup QxtCore

    Example XML log output:
    \code
    <?xml version="1.0" encoding="UTF-8"?>
    <log>
        <entry type="Error" time="22:38:33.159159">
            <message>Unknown error</message>
        </entry>
        <entry type="Debug" time="22:51:43.488488">
            <message>What's going on?</message>
            <message>Hi there</message>
        </entry>
    </log>
    \endcode

    \sa QxtLogger
 */

class QxtXmlFileLoggerEnginePrivate : public QxtPrivate<QxtXmlFileLoggerEngine>
{
    QXT_DECLARE_PUBLIC(QxtXmlFileLoggerEngine);

public:
    QxtXmlFileLoggerEnginePrivate();
    QString tab;
};

QxtXmlFileLoggerEnginePrivate::QxtXmlFileLoggerEnginePrivate()
        : tab("    ")
{
}

/*!
    Constructs an XML file logger engine with file name.
*/
QxtXmlFileLoggerEngine::QxtXmlFileLoggerEngine(const QString& fileName)
        : QxtAbstractFileLoggerEngine(fileName, QIODevice::ReadWrite | QIODevice::Unbuffered)
{
    QXT_INIT_PRIVATE(QxtXmlFileLoggerEngine);
}

/*!
    \reimp
 */
void QxtXmlFileLoggerEngine::initLoggerEngine()
{
    QxtAbstractFileLoggerEngine::initLoggerEngine();

    // Mkay, we have an open file.  We need to check that it's all valid.
    // at the end of this block of code, we either can't log, or the carat is ready for writing.
    /*
    <?xml version="1.0" encoding="UTF-8"?>
    <log>
        <entry type="Error" time="sometime">
            <message>What's going on?</message>
            <message?Hi there</message>
        </entry>
    </log>
    */
    QIODevice* file = device();
    Q_ASSERT(file);
    if (file->size() == 0)
    {
        file->write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        file->write("<log>\n");
        file->write("</log>");
    }
    else
    {
        QByteArray data = file->read(64);
        if (!data.startsWith(QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<log>")))
        {
            QFile* ptr_fileTarget = static_cast<QFile*>(file);
            qxtLog->warning(QString(" is not a valid XML log file.").prepend(ptr_fileTarget->fileName()));
            killLoggerEngine();
            return;
        }
    }
}

/*!
    \reimp
 */
void QxtXmlFileLoggerEngine::writeToFile(const QString &level, const QVariantList &messages)
{
    QIODevice* ptr_fileTarget = device();
    Q_ASSERT(ptr_fileTarget);
    ptr_fileTarget->seek(ptr_fileTarget->size() - 6);
    ptr_fileTarget->write(qxt_d().tab.toUtf8());
    ptr_fileTarget->write("<entry type=\"");
    ptr_fileTarget->write(level.toUtf8());
    ptr_fileTarget->write("\" time=\"");
    ptr_fileTarget->write(QTime::currentTime().toString("hh:mm:ss.zzzz").toUtf8());
    ptr_fileTarget->write("\">");
    ptr_fileTarget->write("\n");
    Q_FOREACH(const QVariant& m, messages)
    {
        ptr_fileTarget->write(qxt_d().tab.toUtf8());
        ptr_fileTarget->write(qxt_d().tab.toUtf8());
        ptr_fileTarget->write("<message>");
        ptr_fileTarget->write(toXmlSafeString(m.toString()).toUtf8());
        ptr_fileTarget->write("</message>\n");
    }

    ptr_fileTarget->write(qxt_d().tab.toUtf8());
    ptr_fileTarget->write("</entry>");
    ptr_fileTarget->write("\n");
    ptr_fileTarget->write("</log>");
}

/*!
    Replaces reserved characters with corresponding entities.
 */
QString QxtXmlFileLoggerEngine::toXmlSafeString(const QString &raw)
{
    /* Reserved characters:
    <  &lt;
    &  &amp;
    > &lt;
    ' &apos;
    " &quot;

    Convert ampersands first, then the rest.
    */
    return QByteArray(raw.toUtf8()).replace('&', "&amp;").replace('<', "&lt;").replace('>', "&gt;").replace('\'', "&apos;").replace('"', "&quot;");
}
