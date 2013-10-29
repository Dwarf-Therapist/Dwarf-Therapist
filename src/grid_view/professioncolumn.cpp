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

#include "professioncolumn.h"
#include "columntypes.h"
#include "viewcolumnset.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "dwarftherapist.h"
#include "defines.h"
#include "gamedatareader.h"

ProfessionColumn::ProfessionColumn(const QString &title, ViewColumnSet *set,
                       QObject *parent)
    : ViewColumn(title, CT_PROFESSION, set, parent)
{
}

ProfessionColumn::ProfessionColumn(const ProfessionColumn &to_copy)
    : ViewColumn(to_copy)
{
}


QStandardItem *ProfessionColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);        
    item->setData(QIcon(d->profession_icon()), Qt::DecorationRole);
    item->setData(CT_PROFESSION, DwarfModel::DR_COL_TYPE);
    item->setData(d->raw_profession(), DwarfModel::DR_SORT_VALUE);    

    QColor bg = QColor(175,175,175);
    if(DT->user_settings()->value("options/grid/shade_cells",true)==false)
        bg = QColor(255,255,255);
    item->setData(bg,Qt::BackgroundColorRole);

    QString tooltip = tr("<center><h3>%1</h3><h4>%2 (%3)</h4></center><h5><i>%4</i></h5><h4>%5</h4>")
            .arg(m_title)
            .arg(d->profession())
            .arg(d->raw_profession())
            .arg(tr("Right click to edit this profession's icon."))
            .arg(d->nice_name());

    item->setToolTip(tooltip);
    return item;
}

QStandardItem *ProfessionColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) {
    Q_UNUSED(dwarves);
    QStandardItem *item = init_aggregate(group_name);
    return item;
}

