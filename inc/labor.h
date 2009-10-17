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
#ifndef LABOR_H
#define LABOR_H

#include <QtGui>

class Labor : public QObject {
	Q_OBJECT
public:
    Labor(QSettings &s, QObject *parent = 0)
        : QObject(parent)
        , name(s.value("name", "UNKNOWN LABOR").toString())
        , labor_id(s.value("id", -1).toInt())
        , skill_id(s.value("skill", -1).toInt())
        , is_weapon(s.value("is_weapon", false).toBool())
    {
        int excludes = s.beginReadArray("excludes");
        for (int i = 0; i < excludes; ++i) {
            s.setArrayIndex(i);
            int labor = s.value("labor_id", -1).toInt();
            if (labor != -1)
                m_excluded_labors << labor;
        }
        s.endArray();
    }
        
    const QList<int> &get_excluded_labors() {
        return m_excluded_labors;
    }

	QString name;
	int labor_id;
	int skill_id;
    QList<int> m_excluded_labors; // list of other labors that this one is exclusive with
    bool is_weapon; // all weapon labors are mutually exclusive of each other
};

#endif

