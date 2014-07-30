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
#ifndef SUPERLABORCOLUMN_H
#define SUPERLABORCOLUMN_H

#include "viewcolumn.h"
#include "skillcolumn.h"
#include "global_enums.h"
#include "multilabor.h"

class ViewColumn;
class Dwarf;
class Attribute;
class SuperLabor;

class SuperLaborColumn : public SkillColumn {
    Q_OBJECT
public:

    SuperLaborColumn(const QString &title, QString id, ViewColumnSet *set = 0, QObject *parent = 0);
    SuperLaborColumn(QSettings &s, ViewColumnSet *set = 0, QObject *parent = 0);
    SuperLaborColumn(const SuperLaborColumn &to_copy); // copy ctor
    SuperLaborColumn* clone() {return new SuperLaborColumn(*this);}
    QStandardItem *build_cell(Dwarf *d);
    QStandardItem *build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves);

    //override
    void write_to_ini(QSettings &s);    
protected:
    QString m_id;
    QPointer<MultiLabor> ml;
    void refresh(Dwarf *d, QStandardItem *item = 0, QString title = "");
    float get_base_sort(Dwarf *d);
    float get_role_rating(Dwarf *d);
    float get_skill_rating(int id, Dwarf *d);
    float get_skill_rate_rating(int id, Dwarf *d);

    virtual MultiLabor* get_base_object();
    void init();

protected slots:    
    virtual void customizations_changed();
    virtual float get_rating(int id, MultiLabor::ML_RATING_TYPE);
};
#endif // SUPERLABORCOLUMN_H
