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
#ifndef TRAIT_H
#define TRAIT_H

#include <QtGui>

class Trait : public QObject {
	Q_OBJECT
public:
	Trait(const QSettings &s, QObject *parent = 0)
		: QObject(parent)
	{
		name = s.value("name", "UNKNOWN").toString();
		m_level_string[0]  = s.value("level_0", "UNKNOWN TRAIT 0(" + name + ")").toString();
		m_level_string[10] = s.value("level_1", "UNKNOWN TRAIT 1(" + name + ")").toString();
		m_level_string[25] = s.value("level_2", "UNKNOWN TRAIT 2(" + name + ")").toString();
		m_level_string[61] = s.value("level_3", "UNKNOWN TRAIT 3(" + name + ")").toString();
		m_level_string[76] = s.value("level_4", "UNKNOWN TRAIT 4(" + name + ")").toString();
		m_level_string[91] = s.value("level_5", "UNKNOWN TRAIT 5(" + name + ")").toString();
	}

	QString level_message(const short &val) {
		if (val > 91)
			return m_level_string[91];
		else if (val > 76)
			return m_level_string[76];
		else if (val > 61)
			return m_level_string[61];
		else if (val > 25)
			return m_level_string[25];
		else if (val > 10)
			return m_level_string[10];
		else
			return m_level_string[0];
	}

	QString name;
	int trait_id;
	//! this map will hold the minimum_value -> string (e.g. level 76-90 of Nervousness is "Is always tense and jittery")
	QMap<int, QString> m_level_string;
};

#endif

