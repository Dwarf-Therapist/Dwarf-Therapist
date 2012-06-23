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
#include "laborcolumn.h"
#include "gamedatareader.h"
#include "columntypes.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "viewcolumnset.h"
#include "dwarftherapist.h"
#include "mainwindow.h"
#include "dfinstance.h"
#include "viewmanager.h"

LaborColumn::LaborColumn(QString title, int labor_id, int skill_id, ViewColumnSet *set, QObject *parent) 
	: ViewColumn(title, CT_LABOR, set, parent)
	, m_labor_id(labor_id)
	, m_skill_id(skill_id)
{
    connect(DT,SIGNAL(labor_counts_updated()), this, SLOT(update_count()));
}

LaborColumn::LaborColumn(QSettings &s, ViewColumnSet *set, QObject *parent) 
	: ViewColumn(s, set, parent)
	, m_labor_id(s.value("labor_id", -1).toInt())
	, m_skill_id(s.value("skill_id", -1).toInt())
{
    connect(DT,SIGNAL(labor_counts_updated()), this, SLOT(update_count()));
}

LaborColumn::LaborColumn(const LaborColumn &to_copy) 
    : ViewColumn(to_copy)
    , m_labor_id(to_copy.m_labor_id)
    , m_skill_id(to_copy.m_skill_id)
{
    connect(DT,SIGNAL(labor_counts_updated()), this, SLOT(update_count()));
}

QStandardItem *LaborColumn::build_cell(Dwarf *d) {
	GameDataReader *gdr = GameDataReader::ptr();
	QStandardItem *item = init_cell(d);

	item->setData(CT_LABOR, DwarfModel::DR_COL_TYPE);
    short rating = d->skill_rating(m_skill_id);
	if (rating < 0 && d->labor_enabled(m_labor_id)) {
		item->setData(float(rating + 0.5f), DwarfModel::DR_SORT_VALUE); // push assigned labors above no exp in sort order
	} else {
		item->setData(rating, DwarfModel::DR_SORT_VALUE);		
	}
	item->setData(rating, DwarfModel::DR_RATING);
	item->setData(m_labor_id, DwarfModel::DR_LABOR_ID);
	item->setData(m_set->name(), DwarfModel::DR_SET_NAME);
	
    QString role_str="";
    if (DT->user_settings()->value("options/show_roles_in_labor", true).toBool()) {

        float role_rating=0;
        if(m_skill_id != -1){
            QVector<Role*> found_roles = GameDataReader::ptr()->get_skill_roles().value(m_skill_id);
            if(found_roles.count() > 0){
                float sortVal = 0;
                role_rating = 0;

                //just list roles and %
                role_str = tr("<h4>Related Roles:</h4><ul style=\"margin-left:-30px; padding-left:0px;\">");
                foreach(Role *r, found_roles){
                    role_rating = d->get_role_rating(r->name);
                    role_str += tr("<li>%1 (%2%)</li>").arg(r->name).arg(QString::number(role_rating,'f',2));
                    sortVal += role_rating;
                    if(d->labor_enabled(m_labor_id))
                        sortVal += 1000;
                }
                role_str += "</ul>";
                sortVal /= found_roles.count();
                item->setData(sortVal,DwarfModel::DR_SORT_VALUE);
            }
        }
    }

    QString skill_str;
    if (m_skill_id != -1 && rating > -1) {
        QString adjusted_rating = QString::number(rating);
        if (rating > 15)
            adjusted_rating = QString("15 +%1").arg(rating - 15);

        skill_str = tr("<b>%1</b> %2 %3<br/>[RAW LEVEL: <b>%4</b>]<br/><b>Experience:</b><br/>%5")
                .arg(d->get_skill(m_skill_id).rust_rating())
                .arg(gdr->get_skill_level_name(rating))
                .arg(gdr->get_skill_name(m_skill_id))
                .arg(adjusted_rating)
                .arg(d->get_skill(m_skill_id).exp_summary());
    } else {
        // either the skill isn't a valid id, or they have 0 experience in it
		skill_str = "0 experience";
	}
    item->setToolTip(QString("<h3>%1</h3>%2%3<h4>%4</h4>").arg(m_title).arg(skill_str).arg(role_str).arg(d->nice_name()));
	return item;
}

QStandardItem *LaborColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &) {
	QStandardItem *item = new QStandardItem;
	item->setStatusTip(m_title + " :: " + group_name);
	QColor bg;
	if (m_override_set_colors) {
		bg = m_bg_color;
	} else {
		bg = set()->bg_color();
	}
	item->setData(CT_LABOR, DwarfModel::DR_COL_TYPE);
	item->setData(bg, Qt::BackgroundColorRole);
	item->setData(bg, DwarfModel::DR_DEFAULT_BG_COLOR);
	item->setData(true, DwarfModel::DR_IS_AGGREGATE);
	item->setData(m_labor_id, DwarfModel::DR_LABOR_ID);
	item->setData(group_name, DwarfModel::DR_GROUP_NAME);
	item->setData(0, DwarfModel::DR_RATING);
	item->setData(m_set->name(), DwarfModel::DR_SET_NAME);    
	return item;
}

void LaborColumn::write_to_ini(QSettings &s) {
	ViewColumn::write_to_ini(s); 
	s.setValue("skill_id", m_skill_id); 
	s.setValue("labor_id", m_labor_id);
}

void LaborColumn::update_count(){
    m_count = DT->get_main_window()->get_DFInstance()->get_labor_count(m_labor_id);
}
