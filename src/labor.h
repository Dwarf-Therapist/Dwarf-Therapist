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

#include <QObject>
#include "role.h"

class Labor : public QObject {
    Q_OBJECT
public:
    Labor(QSettings &s, QObject *parent = 0);

    const QList<int> &get_excluded_labors() {
        return m_excluded_labors;
    }

    static bool hauling_compare(Labor *l1, Labor *l2)
    {
        if(l2->is_hauling < l1->is_hauling)
            return false;
        else if(l1->is_hauling < l2->is_hauling)
            return true;
        else
            return l1->name < l2->name;
    }

    static bool skilled_compare(Labor *l1, Labor *l2)
    {
        if(l1->is_skilled < l2->is_skilled)
            return false;
        else if(l2->is_skilled < l1->is_skilled)
            return true;
        else
            return l1->name < l2->name;
    }


    QString name;
    int labor_id;
    int skill_id;
    QList<int> m_excluded_labors; // list of other labors that this one is exclusive with
    bool requires_equipment; // when first assigned the dwarf should go find
                             // needed equipment (default is false)
    bool is_hauling; //mark hauling labors for optimization purposes
    bool is_skilled; //indicate if a labor requires some skill

};
#endif
