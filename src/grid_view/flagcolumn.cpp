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

#include "flagcolumn.h"
#include "gamedatareader.h"
#include "columntypes.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "viewcolumnset.h"
#include "dwarftherapist.h"

FlagColumn::FlagColumn(QString title, int bit_pos, bool bit_value, ViewColumnSet *set, QObject *parent)
        : ViewColumn(title, CT_FLAGS, set, parent)
        , m_bit_pos(bit_pos)
        , m_bit_value(bit_value)
{    
}

FlagColumn::FlagColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
        : ViewColumn(s, set, parent)
        , m_bit_pos(s.value("bit_pos", -1).toInt())
        , m_bit_value(s.value("bit_value", 0).toBool())
{    
}

FlagColumn::FlagColumn(const FlagColumn &to_copy)
    : ViewColumn(to_copy)
    , m_bit_pos(to_copy.m_bit_pos)
    , m_bit_value(to_copy.m_bit_value)
{    
}

QStandardItem *FlagColumn::build_cell(Dwarf *d) {
        QStandardItem *item = init_cell(d);

        item->setData(CT_FLAGS, DwarfModel::DR_COL_TYPE);        
        short rating = 0;
        if(d->get_flag_value(m_bit_pos))
            rating = 1;
        item->setData(rating, DwarfModel::DR_SORT_VALUE);
        item->setData(m_bit_pos, DwarfModel::DR_LABOR_ID);
        item->setData(m_set->name(), DwarfModel::DR_SET_NAME);
        item->setBackground(QBrush(m_bg_color));
        //check to fix butchering pets. currently this will cause the butchered parts to still be recognized as a pet
        //and they'll put them into a burial recepticle, but won't use them as a food source
        if(d->is_pet() && m_bit_pos == 49){
            item->setToolTip(tr("<b>Please turn off pet availability first.</b><br/><br/>Sorry, pets cannot be butchered due to technical limitations!"));
            item->setData(QBrush(QColor(187,34,34,200)),DwarfModel::DR_DEFAULT_BG_COLOR);
        }
        return item;
}

QStandardItem *FlagColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &) {
        QStandardItem *item = new QStandardItem;
        item->setStatusTip(m_title + " :: " + group_name);
        QColor bg;
        if (m_override_set_colors) {
                bg = m_bg_color;
        } else {
                bg = set()->bg_color();
        }
        item->setData(CT_FLAGS, DwarfModel::DR_COL_TYPE);
        item->setData(m_bit_pos, DwarfModel::DR_LABOR_ID);
        item->setData(bg, Qt::BackgroundColorRole);
        return item;
}

void FlagColumn::write_to_ini(QSettings &s) {
        ViewColumn::write_to_ini(s);
        s.setValue("bit_value", m_bit_value);
        s.setValue("bit_pos", m_bit_pos);
}
