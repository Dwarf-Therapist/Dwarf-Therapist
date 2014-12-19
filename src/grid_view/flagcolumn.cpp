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
#include "defines.h"
#include "gamedatareader.h"
#include "columntypes.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "viewcolumnset.h"
#include "dwarftherapist.h"
#include "caste.h"
#include "viewcolumncolors.h"

FlagColumn::FlagColumn(QString title, int bit_pos, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title, CT_FLAGS, set, parent)
    , m_bit_pos(bit_pos)
{
    init_states();
    refresh_color_map();
}

FlagColumn::FlagColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s, set, parent)
    , m_bit_pos(s.value("bit_pos", -1).toInt())
{
    init_states();
    refresh_color_map();
}

FlagColumn::FlagColumn(const FlagColumn &to_copy)
    : ViewColumn(to_copy)
    , m_bit_pos(to_copy.m_bit_pos)
{
}

void FlagColumn::init_states(){
    ViewColumn::init_states();
    if(m_bit_pos != FLAG_CAGED){
        m_available_states << STATE_PENDING;
    }
}

QStandardItem *FlagColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);

    item->setData(CT_FLAGS, DwarfModel::DR_COL_TYPE);
    item->setData(0,DwarfModel::DR_STATE); //default

    QString info_msg = "";
    QString info_col_name = "";

    short rating = 0;
    if(d->get_flag_value(m_bit_pos))
        rating = 1;
    //check to fix butchering pets. currently this will cause the butchered parts to still be recognized as a pet
    //and they'll put them into a burial recepticle, but won't use them as a food source
    ViewColumn::CELL_STATE state  = STATE_TOGGLE;
    if(m_bit_pos == FLAG_BUTCHER){
        if(d->is_pet()){
            info_msg = tr("<b>Pets cannot be slaughtered!</b>");
            state = STATE_DISABLED;
        }else if(!d->get_caste() || !d->get_caste()->flags().has_flag(BUTCHERABLE)){
            info_msg = tr("<b>This caste cannot be slaughtered!</b>");
            state = STATE_DISABLED;
        }else if(rating == 1){
            info_msg = tr("<b>This creature has been marked for slaughter.</b>");
            state = STATE_PENDING;
        }else{
            state = STATE_TOGGLE;
        }
    }else if(m_bit_pos == FLAG_GELD){
        if(d->get_gender() != Dwarf::SEX_M){
            info_msg = tr("<b>Only males can be gelded!</b>");
            state = STATE_DISABLED;
        }else if(d->has_health_issue(42,0)){
            info_msg = tr("<b>This creature has already been gelded!</b>");
            state = STATE_ACTIVE;
        }else if(rating == 1){
            info_msg = tr("<b>This creature has been marked for gelding.</b>");
            state = STATE_PENDING;
        }else if(!d->get_caste()->is_geldable()){ //check last as it's the most expensive
            info_msg = tr("<b>This caste is not geldable!</b>");
            state = STATE_DISABLED;
        }else{
            state = STATE_TOGGLE;
        }
    }

    item->setData(state,DwarfModel::DR_STATE);
    info_col_name = get_state_color(state).name();//item->data(Qt::BackgroundColorRole).value<QColor>().name();

    item->setData(rating, DwarfModel::DR_SORT_VALUE);
    item->setData(m_bit_pos, DwarfModel::DR_LABOR_ID);
    item->setData(m_set->name(), DwarfModel::DR_SET_NAME);

    QString tooltip = QString("<center><h3>%1</h3>%2</center>%3")
            .arg(m_title)
            .arg(QString("<font color=%1>%2</font>").arg(info_col_name).arg(info_msg))
            .arg(tooltip_name_footer(d));
    item->setToolTip(tooltip);

    return item;
}

QStandardItem *FlagColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) {
    Q_UNUSED(dwarves);
    QStandardItem *item = init_aggregate(group_name);
    item->setData(CT_FLAGS, DwarfModel::DR_COL_TYPE);
    item->setData(m_bit_pos, DwarfModel::DR_LABOR_ID);
    return item;
}

void FlagColumn::write_to_ini(QSettings &s) {
    ViewColumn::write_to_ini(s);
    s.setValue("bit_pos", m_bit_pos);
}
