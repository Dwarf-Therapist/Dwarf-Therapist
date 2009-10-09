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

#include "idlecolumn.h"
#include "columntypes.h"
#include "viewcolumnset.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "dwarftherapist.h"
#include "defines.h"

IdleColumn::IdleColumn(QString title, ViewColumnSet *set, QObject *parent) 
    : ViewColumn(title, CT_IDLE, set, parent)
{}

QStandardItem *IdleColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);
    short job_id = d->current_job_id();
    if (job_id == -1)
        item->setData(QIcon(":status/img/bullet_red.png"), Qt::DecorationRole);
    else if (job_id == 15)
        item->setData(QIcon(":status/img/cheese.png"), Qt::DecorationRole);
    else if (job_id == 20)
        item->setData(QIcon(":status/img/pillow.png"), Qt::DecorationRole);
    else 
        item->setData(QIcon(":status/img/control_play_blue.png"), Qt::DecorationRole);
    
    item->setData(CT_IDLE, DwarfModel::DR_COL_TYPE);
    item->setData(d->current_job_id(), DwarfModel::DR_SORT_VALUE);
    QString tooltip = QString("<h3>%1</h3>%2 (%3)<h4>%4</h4>")
        .arg(m_title)
        .arg(d->current_job())
        .arg(d->current_job_id())
        .arg(d->nice_name());
    item->setToolTip(tooltip);
    return item;
}

QStandardItem *IdleColumn::build_aggregate(const QString &, const QVector<Dwarf*> &dwarves) {
    QStandardItem *item = new QStandardItem;
    /*
    // find lowest happiness of all dwarfs this set represents, and show that color (so low happiness still pops out in a big group)
    Dwarf::DWARF_HAPPINESS lowest = Dwarf::DH_ECSTATIC;
    QString lowest_dwarf = "Nobody";
    foreach(Dwarf *d, dwarves) {
        Dwarf::DWARF_HAPPINESS tmp = d->get_happiness();
        if (tmp <= lowest) {
            lowest = tmp;
            lowest_dwarf = d->nice_name();
        }
    }
    item->setToolTip(tr("<h3>%1</h3>Lowest Happiness in group: <b>%2: %3</b>")
        .arg(m_title)
        .arg(lowest_dwarf)
        .arg(Dwarf::happiness_name(lowest)));
    item->setData(m_colors[lowest], Qt::BackgroundColorRole);
    item->setData(m_colors[lowest], DwarfModel::DR_DEFAULT_BG_COLOR);
    */
    return item;
}