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

#include "traitcolumn.h"
#include "columntypes.h"
#include "viewcolumnset.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "trait.h"
#include "gamedatareader.h"
#include "dwarfstats.h"
#include "dwarftherapist.h"

TraitColumn::TraitColumn(const QString &title, const short &trait_id, ViewColumnSet *set, QObject *parent) 
    : ViewColumn(title, CT_TRAIT, set, parent)
    , m_trait_id(trait_id)
    , m_trait(0)
{
    m_trait = GameDataReader::ptr()->get_trait(trait_id);
}

TraitColumn::TraitColumn(QSettings &s, ViewColumnSet *set, QObject *parent) 
    : ViewColumn(s, set, parent)
    , m_trait_id(s.value("trait_id", -1).toInt())
    , m_trait(0)
{
    m_trait = GameDataReader::ptr()->get_trait(m_trait_id);
}

TraitColumn::TraitColumn(const TraitColumn &to_copy)
    : ViewColumn(to_copy)
    , m_trait_id(to_copy.m_trait_id)
    , m_trait(to_copy.m_trait)
{
}

QStandardItem *TraitColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);
    item->setData(CT_TRAIT, DwarfModel::DR_COL_TYPE);

    short score = d->trait(m_trait_id);
    QString msg = "???";
    if (m_trait)
        msg = tr("%1 (Raw: %2)").arg(m_trait->level_message(score)).arg(score);

    if (d->trait_is_active(m_trait_id)==false)
        msg += tr("<br/><br/>Not an active trait for this dwarf.");

    QString temp = m_trait->skill_conflicts_msgs(score);
    if(!temp.isEmpty())
        msg += tr("<br/><br/>%1").arg(temp);
    temp = m_trait->special_messages(score);
    if(!temp.isEmpty())
        msg += tr("<br/><br/>%1").arg(temp);

    int rating = score;
    QString warning = "";
    if(GameDataReader::ptr()->get_trait(m_trait_id)->inverted){
        warning = tr("<br/><h5 style=\"margin:0;\"><font color=red>*This trait's score should be valued inversely!</font></h5>");
        rating = 100 - score;
    }

    item->setText(QString::number(score));
    item->setData(rating, DwarfModel::DR_SORT_VALUE);
    item->setData(rating, DwarfModel::DR_RATING);
    item->setData(rating, DwarfModel::DR_DISPLAY_RATING);
    item->setData(score, DwarfModel::DR_SPECIAL_FLAG);
    set_export_role(DwarfModel::DR_SPECIAL_FLAG);
    
    QString tooltip = QString("<center><h3>%1</h3></center>%2<br>%3%4")
            .arg(m_title)
            .arg(msg)
            .arg(warning)
            .arg(tooltip_name_footer(d));
    item->setToolTip(tooltip);

    return item;
}

QStandardItem *TraitColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) {    
    Q_UNUSED(dwarves);
    QStandardItem *item = init_aggregate(group_name);
    return item;
}
