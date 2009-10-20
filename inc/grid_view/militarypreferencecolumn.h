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
#ifndef MILITARY_PREFERENCE_COLUMN_H
#define MILITARY_PREFERENCE_COLUMN_H

#include "viewcolumn.h"

class MilitaryPreferenceColumn : public ViewColumn {
public:
    MilitaryPreferenceColumn(const QString &title, const int &labor_id, const int &skill_id, ViewColumnSet *set = 0, QObject *parent = 0);
    MilitaryPreferenceColumn(QSettings &s, ViewColumnSet *set = 0, QObject *parent = 0);
    MilitaryPreferenceColumn(const MilitaryPreferenceColumn &to_copy); // copy ctor
    MilitaryPreferenceColumn* clone() {return new MilitaryPreferenceColumn(*this);}
    QStandardItem *build_cell(Dwarf *d);
    QStandardItem *build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves);

    int labor_id() {return m_labor_id;}
    void set_labor_id(int labor_id) {m_labor_id = labor_id;}
    int skill_id() {return m_skill_id;}
    void set_skill_id(int skill_id) {m_skill_id = skill_id;}

    // override
    void write_to_ini(QSettings &s);

protected:
    int m_labor_id;
    int m_skill_id;
};

#endif
