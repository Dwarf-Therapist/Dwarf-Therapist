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

HighestMoodColumn::HighestMoodColumn(const QString &title, ViewColumnSet *set, QObject *parent)
    : SkillColumn(title,-1,set,parent,CT_HIGHEST_MOOD)
{
}

HighestMoodColumn::HighestMoodColumn(const HighestMoodColumn &to_copy)
    : SkillColumn(to_copy)
{
}


QStandardItem *HighestMoodColumn::build_cell(Dwarf *d) {
    GameDataReader *gdr = GameDataReader::ptr();
    QStandardItem *item = init_cell(d);

    Skill *s = d->highest_moodable();
    m_skill_id = s->id();

    int img_id = 24;
    if(s->rating() != -1)
        img_id = gdr->get_pref_from_skill(s->id()) + 1; //prof images start at 1, id start at 0

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

    set_tooltip(d,item, "", false, sort_val);
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

void HighestMoodColumn::write_to_ini(QSettings &s) {
    ViewColumn::write_to_ini(s);
}
