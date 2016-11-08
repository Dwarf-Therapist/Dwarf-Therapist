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

#include "beliefcolumn.h"
#include "columntypes.h"
#include "gamedatareader.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "unitbelief.h"
#include "belief.h"

BeliefColumn::BeliefColumn(const QString &title, int belief_id, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title, CT_BELIEF, set, parent)
    , m_id(belief_id)
{
    if (title.isEmpty()) // Determine title based on type if no title was passed in
        m_title = GameDataReader::ptr()->get_belief_name(m_id);
}

BeliefColumn::BeliefColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s, set, parent)
    , m_id(s.value("belief_id",-1).toInt())
{
}

BeliefColumn::BeliefColumn(const BeliefColumn &to_copy)
    : ViewColumn(to_copy)
    , m_id(to_copy.m_id)
{
}

QStandardItem *BeliefColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);
    item->setData(CT_BELIEF, DwarfModel::DR_COL_TYPE);
    item->setData(0, DwarfModel::DR_SPECIAL_FLAG); //default, special flag stores the alpha for the border

    UnitBelief ub = d->get_unit_belief(m_id);
    Belief *b = GameDataReader::ptr()->get_belief(m_id);

    int raw_value = ub.belief_value();
    int display_value = raw_value + 50; //adjust from -50 to 50 -> 0 to 100 for drawing
    QStringList infos;
    if (b){
        infos << b->level_message(raw_value);
    }else{
        infos << tr("Unknown belief");
    }

    if(!d->belief_is_active(m_id)){
        infos << tr("Not an active belief for this dwarf.");
        display_value = 50; //hide inactive beliefs
    }

    //show a description of all possible conflicts
    int all_trait_conflicts = 1;
    if(b){
        all_trait_conflicts = b->get_trait_conflicts().count();
        if(all_trait_conflicts > 0){
            infos << tr("<br/>This belief can conflict with %1").arg(b->trait_conflict_names());
        }
    }

    infos.removeAll("");

    //get a value of this unit's conflicts / total possible conflicts for the border color
    int conflicts = ub.trait_conflicts().count();
    if(conflicts > 0){
        int alpha = 255 * ((float)conflicts / (float)all_trait_conflicts);
        item->setData(alpha, DwarfModel::DR_SPECIAL_FLAG);
    }

    item->setText(QString::number(raw_value));
    item->setData(display_value, DwarfModel::DR_SORT_VALUE);
    item->setData(display_value, DwarfModel::DR_RATING);
    item->setData(raw_value, DwarfModel::DR_DISPLAY_RATING);
    set_export_role(DwarfModel::DR_RATING);

    QString tooltip = QString("<center><h3>%1</h3><b>Value: %2</b></center><br/>%3<br/>%4")
            .arg(m_title)
            .arg(raw_value)
            .arg(infos.join("<br/>"))
            .arg(tooltip_name_footer(d));
    item->setToolTip(tooltip);

    return item;
}

QStandardItem *BeliefColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves){
    Q_UNUSED(dwarves);
    QStandardItem *item = init_aggregate(group_name);
    return item;
}

