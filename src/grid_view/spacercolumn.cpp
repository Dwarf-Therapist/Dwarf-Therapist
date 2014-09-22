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

#include "spacercolumn.h"
#include "columntypes.h"
#include "viewcolumnset.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "defines.h"

SpacerColumn::SpacerColumn(int col_width, int col_idx, ViewColumnSet *set, QObject *parent)
    : ViewColumn("", CT_SPACER, set, parent, col_idx)
    , m_width(col_width)
{}

SpacerColumn::SpacerColumn(QString title, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title, CT_SPACER, set, parent)
    , m_width(DEFAULT_SPACER_WIDTH)
{}

SpacerColumn::SpacerColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn("",CT_SPACER, set, parent)
    , m_width(s.value("width", DEFAULT_SPACER_WIDTH).toInt())
{}

SpacerColumn::SpacerColumn(const SpacerColumn &to_copy)
    : ViewColumn(to_copy)
    , m_width(to_copy.m_width)
{}

QStandardItem *SpacerColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);
    item->setData(d->nice_name(),DwarfModel::DR_GLOBAL); //default
        if(m_width <= 0){
            item->setData(QColor(Qt::transparent), DwarfModel::DR_DEFAULT_BG_COLOR);
            item->setData(QColor(Qt::transparent),Qt::BackgroundColorRole);
        }
    return item;
}

QStandardItem *SpacerColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) {
    Q_UNUSED(dwarves);
    QStandardItem *item = init_aggregate(group_name);
    return item;
}

void SpacerColumn::write_to_ini(QSettings &s) {
    if(m_width > 0){
        ViewColumn::write_to_ini(s);
        if (m_width)
            s.setValue("width", m_width);
    }
}
