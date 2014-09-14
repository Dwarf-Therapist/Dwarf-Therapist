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

#include "unitkillscolumn.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "viewcolumnset.h"
#include "dwarfstats.h"
#include "histfigure.h"

UnitKillsColumn::UnitKillsColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s, set, parent)
{
}

UnitKillsColumn::UnitKillsColumn(const QString &title, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title, CT_KILLS, set, parent)
{
}

UnitKillsColumn::UnitKillsColumn(const UnitKillsColumn &to_copy)
    : ViewColumn(to_copy)

{
}

QStandardItem *UnitKillsColumn::build_cell(Dwarf *d){
    QStandardItem *item = init_cell(d);
    QString kill_summary = "";
    int kills = 0;
    float rating = 50.0f; //50 is not drawn
    HistFigure *h = d->hist_figure();
    if(h){
        kills = h->total_kills();
        kill_summary = h->formatted_summary(true,true);
    }
    if(kills > 0){
        int max_kills = DwarfStats::get_max_unit_kills();
        rating = ((float)kills / (float)max_kills * 100.0f / 2.0f) + 51.0f; //scale from 50+1 to 100        
    }

    item->setData(CT_KILLS, DwarfModel::DR_COL_TYPE);
    item->setData(kills, DwarfModel::DR_DISPLAY_RATING); //numeric drawing, single digits
    item->setData(rating, DwarfModel::DR_RATING); //other drawing 0-100
    item->setData(kills, DwarfModel::DR_SORT_VALUE);
    set_export_role(DwarfModel::DR_SORT_VALUE);

    QString tooltip = QString("<center><h3 style=\"margin:0;\">%1</h3></center>%2%3")
            .arg(m_title)
            .arg(kill_summary)
            .arg(tooltip_name_footer(d));

    item->setToolTip(tooltip);
    return item;
}


QStandardItem *UnitKillsColumn::build_aggregate(const QString &group_name, const QVector<Dwarf *> &dwarves){
    Q_UNUSED(dwarves);
    QStandardItem *item = init_aggregate(group_name);
    return item;
}
