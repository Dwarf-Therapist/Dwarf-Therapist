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
*/

#ifndef TRUNCATING_FILE_LOGGER_ENGINE_H
#define TRUNCATING_FILE_LOGGER_ENGINE_H
#include <qxtloggerengine>
#include <qxtbasicfileloggerengine.h>
#include <qxtabstractfileloggerengine>
#include <QTextStream>
#include <QFile>
#include <QTime>

class TruncatingFileLoggerEngine : public QxtAbstractFileLoggerEngine
{
public:
	TruncatingFileLoggerEngine(const QString &fileName = QString()) 
		: QxtAbstractFileLoggerEngine(fileName, QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Unbuffered)
	{}

protected:
	void writeToFile(const QString &level, const QVariantList &messages) {
		if (messages.isEmpty()) 
			return;
		QString header = '[' + QTime::currentTime().toString("hh:mm:ss.zzz") + "] [" + level + "] ";
		//QString padding;
		QIODevice* file = device();
		Q_ASSERT(file);
		file->write(header.toUtf8());
		//for (int i = 0; i < header.size(); i++) padding.append(" ");
		//int count = 0;
		Q_FOREACH(const QVariant& out, messages) {
			if (!out.isNull() 
				&& out.toString() != "qdebug"
				&& out.toString() != "qwarning"
				&& out.toString() != "qcritical") {
				//if (count != 0) file->write(padding.toUtf8());
				file->write(out.toString().toUtf8());
				file->write(" ");
			}
			//count++;
		}
		file->write("\n");
	}
};

#endif //TRUNCATING_FILE_LOGGER_ENGINE_H


