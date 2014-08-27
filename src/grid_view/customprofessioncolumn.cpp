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

#include "customprofessioncolumn.h"
#include "superlaborcolumn.h"
#include "columntypes.h"
#include "viewcolumnset.h"
#include "gamedatareader.h"
#include "dwarfmodel.h"
#include "truncatingfilelogger.h"
#include "dwarf.h"
#include "dwarftherapist.h"
#include "customprofession.h"

CustomProfessionColumn::CustomProfessionColumn(const QString &title, QString id, ViewColumnSet *set, QObject *parent)
    : SuperLaborColumn(title,id,set,parent)
{
    init();
}

CustomProfessionColumn::CustomProfessionColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : SuperLaborColumn(s,set,parent)
{
    init();
}

CustomProfessionColumn::CustomProfessionColumn(const CustomProfessionColumn &to_copy)
    : SuperLaborColumn(to_copy)
{
    init();
}

void CustomProfessionColumn::init(){
    m_type = CT_CUSTOM_PROFESSION;
    ml = QPointer<MultiLabor>(this->get_base_object());
}

QStandardItem *CustomProfessionColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);
    item->setData(CT_CUSTOM_PROFESSION, DwarfModel::DR_COL_TYPE);

    if(ml.isNull()){
        item->setData("", DwarfModel::DR_CUSTOM_PROF);
        item->setData(-1, DwarfModel::DR_RATING);
        item->setData(-1, DwarfModel::DR_DISPLAY_RATING);
        item->setData(-1,DwarfModel::DR_LABORS);
        item->setToolTip(tr("Unknown custom profession."));
        return item;
    }else{
        //build a custom title
        QString prof_title = "";
        QString cp_name = ml->get_name();
        if(!cp_name.isEmpty()){
            prof_title = "<center>";
            CustomProfession *cp  = qobject_cast<CustomProfession*>(ml);
            if(cp->has_icon())
                prof_title.append(cp->get_embedded_pixmap()).append(" ");
            if(d->profession() == cp_name){
                prof_title.append(tr("<font color=%1>%2</font>").arg(ml->active_labor_color().name()).arg(cp_name));
            }else{
                prof_title.append(tr("%1").arg(cp_name));
            }
            prof_title.append("</center>");
        }
        refresh(d,item,prof_title);
    }

    return item;
}

QStandardItem *CustomProfessionColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves){
    Q_UNUSED(dwarves);
    QStandardItem *item = init_aggregate(group_name);
    return item;
}

MultiLabor* CustomProfessionColumn::get_base_object(){
    return DT->get_custom_profession(m_id);
}
