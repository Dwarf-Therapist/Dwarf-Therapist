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

SkillColumn::SkillColumn(const QString &title, const int &skill_id, ViewColumnSet *set, QObject *parent) 
	: ViewColumn(title, CT_SKILL, set, parent)
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
	GameDataReader *gdr = GameDataReader::ptr();
	QStandardItem *item = init_cell(d);

	item->setData(CT_SKILL, DwarfModel::DR_COL_TYPE);
    item->setData(m_skill_id,DwarfModel::DR_LABOR_ID);
	short rating = d->skill_rating(m_skill_id);
	item->setData(rating, DwarfModel::DR_RATING);
	item->setData(rating, DwarfModel::DR_SORT_VALUE);

    QString role_str="";
    if (DT->user_settings()->value("options/show_roles_in_skills", true).toBool()) {
        float role_rating=0;
        if(m_skill_id != -1){
            //if we find a role with an exact match to the column name, use only this role
            QVector<Role*> found_roles;
//            Role* r = GameDataReader::ptr()->get_role(this->title());
//            if(r)
//                found_roles << r;
//            else
                found_roles = GameDataReader::ptr()->get_skill_roles().value(m_skill_id);
            if(found_roles.count() > 0){

                float sortVal = 0;
                role_rating = 0;

                //just list roles and %
                role_str = tr("<h4>Related Role:</h4><ul style=\"margin-left:-30px; padding-left:0px;\">");
                foreach(Role *r, found_roles){
                    role_rating = d->get_role_rating(r->name);
                    role_str += tr("<li>%1 (%2%)</li>").arg(r->name).arg(QString::number(role_rating,'f',2));
                    sortVal += role_rating;
                    if(d->get_skill(m_skill_id)->actual_exp() > 0)
                        sortVal += 1000;
                }
                role_str += "</ul>";
                sortVal /= found_roles.count();
                if(DT->user_settings()->value("options/sort_roles_in_skills", true).toBool())
                    item->setData(sortVal,DwarfModel::DR_SORT_VALUE);
            }
        }
    }

    QString str_mood = "";
    if(m_skill_id == d->highest_moodable()->id()){
        str_mood = tr("<br/><br/>This is the highest moodable skill.");
        if(d->had_mood())
            str_mood = tr("<br/><br/>Had a mood with this skill and crafted '%1'.").arg(d->artifact_name());
    }

    QString skill_str;
    if (m_skill_id != -1 && rating > -1) {
        QString adjusted_rating = QString::number(rating);
        if (rating > 15)
            adjusted_rating = QString("15 +%1").arg(rating - 15);
        skill_str = tr("<b>%1</b> %2 %3<br/>[RAW LEVEL: <b><font color=blue>%4</font></b>]<br/><b>Experience:</b><br/>%5")
                .arg(d->get_skill(m_skill_id)->rust_rating())
                .arg(gdr->get_skill_level_name(rating))
                .arg(gdr->get_skill_name(m_skill_id))
                .arg(adjusted_rating)
                .arg(d->get_skill(m_skill_id)->exp_summary());
    } else {
        // either the skill isn't a valid id, or they have 0 experience in it
		skill_str = "0 experience";
	}
    QString tooltip = QString("<h3>%1</h3>%2%3%4<h4>%5</h4>")
            .arg(m_title)
            .arg(skill_str)
            .arg(role_str)
            .arg(str_mood)
            .arg(d->nice_name());

    item->setToolTip(tooltip);

	return item;
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
