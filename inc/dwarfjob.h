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
#ifndef DWARF_JOB_H
#define DWARF_JOB_H

#include <QtCore>

class DwarfJob : public QObject {
	Q_OBJECT
public:
	typedef enum {
		DJT_DEFAULT,
		DJT_IDLE,
		DJT_DIG,
        DJT_CUT,
		DJT_REST,
		DJT_DRINK,
		DJT_FOOD,
		DJT_BUILD,
		DJT_HAUL,
		DJT_ADMIN,
        DJT_FIGHT,
        DJT_MOOD,
        DJT_FORGE
	} DWARF_JOB_TYPE;

	static DWARF_JOB_TYPE get_type(const QString &type) {
		QMap<QString, DWARF_JOB_TYPE> m;
		m["idle"] = DJT_IDLE;
		m["dig"] = DJT_DIG;
        m["cut"] = DJT_CUT;
		m["rest"] = DJT_REST;
		m["drink"] = DJT_DRINK;
		m["food"] = DJT_FOOD;
		m["build"] = DJT_BUILD;
		m["haul"] = DJT_HAUL;
		m["admin"] = DJT_ADMIN;
        m["fight"] = DJT_FIGHT;
        m["mood"] = DJT_MOOD;
        m["forge"] = DJT_FORGE;
		return m.value(type.toLower(), DJT_DEFAULT);
	}
	
	DwarfJob(short id, QString description, DWARF_JOB_TYPE type, QObject *parent = 0)
		: QObject(parent)
		, id(id)
		, description(description)
		, type(type)
	{}

	short id;
	QString description;
	DWARF_JOB_TYPE type;
};

#endif
