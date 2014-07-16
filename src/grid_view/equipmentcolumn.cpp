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

#include  "equipmentcolumn.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "viewcolumnset.h"
#include "columntypes.h"
#include "dwarftherapist.h"
#include "defaultfonts.h"

EquipmentColumn::EquipmentColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    :ItemTypeColumn(s,set,parent)
{
}

EquipmentColumn::EquipmentColumn(const QString &title, ViewColumnSet *set, QObject *parent)
    :ItemTypeColumn(title,NONE,set,parent,CT_EQUIPMENT)
{
}

EquipmentColumn::EquipmentColumn(const EquipmentColumn &to_copy)
    :ItemTypeColumn(to_copy)
{
}

QStandardItem *EquipmentColumn::build_cell(Dwarf *d){
    QStandardItem *item = init_cell(d);

    QColor rating_color = QColor(69,148,21);
    float rating =  d->get_uniform_rating();
    float coverage = d->get_coverage_rating();

    if(coverage < 100){ //prioritize coverage
        rating = coverage;
        rating_color = Item::color_uncovered();
    }else{
        if(rating < 100) //missing uniform items
            rating_color = Item::color_missing();
    }

    float sort_val = rating - d->get_inventory_wear();
    item->setData(d->get_inventory_wear(),DwarfModel::DR_SPECIAL_FLAG);
//    item->setBackground(QBrush(rating_color));
    item->setData(rating_color,Qt::BackgroundColorRole);
    item->setData(CT_EQUIPMENT, DwarfModel::DR_COL_TYPE);
    item->setData(rating, DwarfModel::DR_RATING); //other drawing 0-100
    item->setData(sort_val, DwarfModel::DR_SORT_VALUE);
    set_export_role(DwarfModel::DR_RATING);

    QString tooltip = QString("<center><h3>%1</h3></center>%2%3")
            .arg(m_title)
            .arg(build_tooltip_desc(d))
            .arg(tooltip_name_footer(d));

    item->setToolTip(tooltip);
    return item;
}

QStandardItem *EquipmentColumn::build_aggregate(const QString &group_name, const QVector<Dwarf *> &dwarves){
    Q_UNUSED(dwarves);
    QStandardItem *item = init_aggregate(group_name);
    return item;
}
