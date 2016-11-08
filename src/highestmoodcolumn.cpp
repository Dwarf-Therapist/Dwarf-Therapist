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
#include "dwarfmodel.h"
#include "dwarf.h"
#include "gamedatareader.h"
#include "skill.h"

HighestMoodColumn::HighestMoodColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : SkillColumn(s,set,parent)
{
    m_sortable_types.clear();
    m_type = CT_HIGHEST_MOOD;
}

HighestMoodColumn::HighestMoodColumn(const QString &title, ViewColumnSet *set, QObject *parent)
    : SkillColumn(title,-1,set,parent,CT_HIGHEST_MOOD)
{
    m_sortable_types.clear();
}

HighestMoodColumn::HighestMoodColumn(const HighestMoodColumn &to_copy)
    : SkillColumn(to_copy)
{
    m_sortable_types = to_copy.m_sortable_types;
    m_type = CT_HIGHEST_MOOD;
}


QStandardItem *HighestMoodColumn::build_cell(Dwarf *d) {
    GameDataReader *gdr = GameDataReader::ptr();
    QStandardItem *item = init_cell(d);

    QString pixmap_name(":img/question-frame.png");

    QHash<int,Skill> skills = d->get_moodable_skills();
    if(skills.count() > 1){
        m_sort_val = 1000 + skills.count();
        m_skill_id = -1;
    }else if(skills.count() <= 1){
        Skill s = skills.values().at(0);
        m_skill_id = s.id();
        int img_id = 24;
        if(s.capped_level() != -1){
            img_id = gdr->get_mood_skill_prof(s.id()) + 1; //prof images start at 1, id start at 0
        }
        pixmap_name = ":/profession/prof_" + QString::number(img_id) + ".png";

        int id = s.id() < 0 ? 0 : s.id();
        m_sort_val = 50 + (id * 100);
        if(d->had_mood())
            m_sort_val = 0 - m_sort_val;

        m_sort_val += s.raw_level();
    }

    item->setData(QIcon(pixmap_name), Qt::DecorationRole);
    item->setData(CT_HIGHEST_MOOD, DwarfModel::DR_COL_TYPE);
    item->setData(d->had_mood(),DwarfModel::DR_SPECIAL_FLAG);
    set_export_role(DwarfModel::DR_SPECIAL_FLAG);
    item->setData(m_sort_val, DwarfModel::DR_SORT_VALUE);

    if(skills.count() <= 1){
        build_tooltip(d,false,false);
    }else{
        QStringList skill_desc;
        foreach(Skill s, skills.values()){
            skill_desc.append(build_skill_desc(d,s.id()).replace("<br/>"," "));
        }

        QString str_mood = tr("<br/><br/>One of these skills will be chosen at random when a mood occurs.");

        QString tooltip = QString("<center><h3 style=\"margin:0;\">%1</h3>%2%3%4</center>")
                .arg(m_title)
                .arg(skill_desc.join("<br/>"))
                .arg(str_mood)
                .arg(tooltip_name_footer(d));

        item->setToolTip(tooltip);
    }

    return item;
}

QStandardItem *HighestMoodColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) {
    Q_UNUSED(dwarves);
    QStandardItem *item = init_aggregate(group_name);
    return item;
}

void HighestMoodColumn::write_to_ini(QSettings &s) {
    ViewColumn::write_to_ini(s);
}
