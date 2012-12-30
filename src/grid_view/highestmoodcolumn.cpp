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

#include "highestmoodcolumn.h"
#include "columntypes.h"
#include "viewcolumnset.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "dwarftherapist.h"
#include "defines.h"
#include "gamedatareader.h"
#include "skill.h"

HighestMoodColumn::HighestMoodColumn(const QString &title, ViewColumnSet *set,
                       QObject *parent)
    : ViewColumn(title, CT_HIGHEST_MOOD, set, parent)
{
}

HighestMoodColumn::HighestMoodColumn(const HighestMoodColumn &to_copy)
    : ViewColumn(to_copy)
{
}


QStandardItem *HighestMoodColumn::build_cell(Dwarf *d) {
    GameDataReader *gdr = GameDataReader::ptr();
    QStandardItem *item = init_cell(d);

    Skill *s = d->highest_moodable();
    QString skill_name = tr("Random Craftsdwarf");

    int img_id = 24;
    if(s->rating() != -1){
        img_id = gdr->get_pref_from_skill(s->id()) + 1; //prof images start at 1, id start at 0
        skill_name = s->name();
    }

    QString pixmap_name = ":/profession/img/profession icons/prof_" + QString::number(img_id) + ".png";
    item->setData(QIcon(pixmap_name), Qt::DecorationRole);

    item->setData(CT_HIGHEST_MOOD, DwarfModel::DR_COL_TYPE);
    item->setData(d->had_mood(),DwarfModel::DR_RATING);
    int sort_val = 50 + (s->id() * 100);

    if(d->had_mood())
        sort_val = 0 - sort_val;

    sort_val += s->rating();
    item->setData(sort_val, DwarfModel::DR_SORT_VALUE);

    QColor bg = QColor(175,175,175);
    if(DT->user_settings()->value("options/grid/shade_cells",true)==false)
        bg = QColor(255,255,255);
    item->setData(bg,Qt::BackgroundColorRole);


    QString str_mood = "";
    str_mood = tr("<br/><br/>%1 is the highest moodable skill.").arg(capitalize(s->name()));
    if(d->had_mood())
        str_mood = tr("<br/><br/>Has already had a mood as a %1 and crafted '%2'.").arg(capitalize(s->name())).arg(d->artifact_name());


    QString skill_str;
    if(s->id() != -1 && s->rating() > -1) {
        QString adjusted_rating = QString::number(s->rating());
        if (s->rating() > 15)
            adjusted_rating = QString("15 +%1").arg(s->rating() - 15);
        skill_str = tr("<b>%1</b> %2 %3<br/>[RAW LEVEL: <b><font color=blue>%4</font></b>]<br/><b>Experience:</b><br/>%5")
                .arg(s->rust_rating())
                .arg(gdr->get_skill_level_name(s->rating()))
                .arg(gdr->get_skill_name(s->id()))
                .arg(adjusted_rating)
                .arg(s->exp_summary());
    } else {
        // either the skill isn't a valid id, or they have 0 experience in it
        skill_str = "0 experience";
    }
    QString tooltip = QString("<h3>%1</h3>%2%3<h4>%4</h4>")
            .arg(m_title)
            .arg(skill_str)
            .arg(str_mood)
            .arg(d->nice_name());

    item->setToolTip(tooltip);

    s = 0;

    return item;
}

QStandardItem *HighestMoodColumn::build_aggregate(const QString &group_name,
                                           const QVector<Dwarf*> &dwarves) {
    Q_UNUSED(group_name);
    Q_UNUSED(dwarves);
    QStandardItem *item = new QStandardItem;
    item->setData(m_bg_color, DwarfModel::DR_DEFAULT_BG_COLOR);
    return item;
}
