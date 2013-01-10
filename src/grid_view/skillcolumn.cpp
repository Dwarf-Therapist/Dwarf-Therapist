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

#include "skillcolumn.h"
#include "gamedatareader.h"
#include "viewcolumnset.h"
#include "columntypes.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "dwarfstats.h"
#include "dwarftherapist.h"
#include "skill.h"
#include "trait.h"

SkillColumn::SkillColumn(const QString &title, const int &skill_id, ViewColumnSet *set, QObject *parent, COLUMN_TYPE cType)
    : ViewColumn(title, cType, set, parent)
	, m_skill_id(skill_id)
{
}

SkillColumn::SkillColumn(QSettings &s, ViewColumnSet *set, QObject *parent) 
    : ViewColumn(s, set, parent)
    , m_skill_id(s.value("skill_id", -1).toInt())
{
}

SkillColumn::SkillColumn(const SkillColumn &to_copy) 
    : ViewColumn(to_copy)
    , m_skill_id(to_copy.m_skill_id)
{
}

QStandardItem *SkillColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);
    m_sort_val = 0;
    bool sorting_by_role = DT->user_settings()->value("options/sort_roles_in_skills", true).toBool();
    short rating = d->skill_rating(m_skill_id);
    //short raw_rating = d->skill_rating(m_skill_id,true);

    if(d->get_skill(m_skill_id)->actual_exp() > 0)
        m_sort_val += 1000;

    item->setData(CT_SKILL, DwarfModel::DR_COL_TYPE);
    item->setData(rating, DwarfModel::DR_RATING);
    item->setData(m_skill_id, DwarfModel::DR_LABOR_ID);
    item->setData(m_set->name(), DwarfModel::DR_SET_NAME);

    set_sorting(d,item,rating,sorting_by_role);
    set_tooltip(d,item,"show_roles_in_labor",sorting_by_role);

    return item;

}

void SkillColumn::set_sorting(Dwarf *d, QStandardItem *item, int rating, bool sorting_by_role){
    float adj = 0;
    if(!sorting_by_role){
        //include the skill learning rate in the sorting. equally weigh the skill level and rate
        adj = ((float)rating / 20) * 100; //can't use raw rating unless we know the limit for the skill (ie. max dwarf's raw level)

        if(DT->get_DFInstance()->show_skill_rates()){
            adj += (((float)d->get_skill(m_skill_id)->skill_rate() / 500.0f) * 100);
            //m_sort_val += d->get_skill(m_skill_id)->skill_rate();
            adj /= 2.0f;
        }
        m_sort_val += adj;
    }
    item->setData(m_sort_val, DwarfModel::DR_SORT_VALUE);
}

void SkillColumn::set_tooltip(Dwarf *d, QStandardItem *item, QString option_name, bool sorting_by_role){
    GameDataReader *gdr = GameDataReader::ptr();

    //build the role section and adjust the sort value if necessary
    QString role_str="";
    if(!option_name.isEmpty() && DT->user_settings()->value(QString("options/%1").arg(option_name), true).toBool()) {
        float new_sort_val = m_sort_val;
        float role_rating=0;
        if(m_skill_id != -1){
            QVector<Role*> found_roles = gdr->get_skill_roles().value(m_skill_id);
            if(found_roles.count() > 0){
                role_rating = 0;
                //just list roles and %
                role_str = tr("<h4>Related Roles:</h4><ul style=\"margin-left:-20px; padding-left:0px;\">");
                foreach(Role *r, found_roles){
                    role_rating = d->get_role_rating(r->name);
                    role_str += tr("<li>%1 (%2%)</li>").arg(r->name).arg(QString::number(role_rating,'f',2));
                    new_sort_val += role_rating;
                }
                role_str += "</ul>";
                new_sort_val /= found_roles.count();
                if(sorting_by_role){
                    m_sort_val = new_sort_val;
                    item->setData(m_sort_val,DwarfModel::DR_SORT_VALUE);
                }
            }
        }
    }

    //skill bonus
    QString str_skill_rate = "";
    if(DT->get_DFInstance()->show_skill_rates()){
        int raw_bonus = d->get_skill(m_skill_id)->skill_rate();
        int bonus = raw_bonus - 100;
        str_skill_rate = QString("<br><b>XP Bonus: </b>%1% [RAW: %2%]")
                .arg(QString::number(bonus,'f',0))
                .arg(QString::number(raw_bonus,'f',0));
    }

    QString str_mood = "";
    if(m_skill_id == d->highest_moodable()->id()){
        str_mood = tr("<br/><br/>This is the highest moodable skill.");
        if(d->had_mood())
            str_mood = tr("<br/><br/>Had a mood with this skill and crafted '%1'.").arg(d->artifact_name());
    }

    //skill xp, level, name, mood
    QString skill_str = "";
    short rating = d->skill_rating(m_skill_id);
    short raw_rating = d->skill_rating(m_skill_id, true);
    if ((m_skill_id != -1 && rating > -1) || d->had_mood()) {
        if(m_skill_id == -1){
            skill_str = tr("%1 %2")
                    .arg(gdr->get_skill_name(m_skill_id,true))
                    .arg(str_mood);
        }else{
            skill_str = tr("%1 %2<br/><br/><b>[RAW LEVEL:</b> %3]<br/><b>Experience: </b>%4%5%6")
                    .arg(gdr->get_skill_level_name(rating))
                    .arg(gdr->get_skill_name(m_skill_id,true))
                    .arg(QString::number(raw_rating))
                    .arg(d->get_skill(m_skill_id)->exp_summary())
                    .arg(str_skill_rate)
                    .arg(str_mood);
        }
    } else {
        // either the skill isn't a valid id, or they have 0 experience in it
        skill_str = "<b>0 Experience</b>";
        skill_str += str_skill_rate;
    }

    QStringList conflicting_traits;
    QString conflicts_str = "";
    QHashIterator<int, Trait*> i(gdr->get_traits());
    while(i.hasNext()){
        i.next();
        QString con = i.value()->conflict_message(m_skill_id, d->trait(i.value()->trait_id));
        if(!con.isEmpty())
            conflicting_traits.append(con);
    }
    if(conflicting_traits.length() > 0){
        conflicts_str = tr("<h4><b>%1</b></h4>")
                .arg(conflicting_traits.join(", "));
    }

    QString tooltip = QString("<h3>%1</h3>%2%3%4<h4>%5</h4>")
            .arg(m_title)
            .arg(skill_str)
            .arg(role_str)
            .arg(conflicts_str)
            .arg(d->nice_name());

    item->setToolTip(tooltip);
}

QStandardItem *SkillColumn::build_aggregate(const QString &, const QVector<Dwarf*> &) {
	QStandardItem *item = new QStandardItem;
	QColor bg;
	if (m_override_set_colors)
		bg = m_bg_color;
	else
		bg = m_set->bg_color();
	item->setData(bg, Qt::BackgroundColorRole);
	item->setData(bg, DwarfModel::DR_DEFAULT_BG_COLOR);
	return item;
}

void SkillColumn::write_to_ini(QSettings &s) {
    ViewColumn::write_to_ini(s);
    s.setValue("skill_id", m_skill_id);
}
