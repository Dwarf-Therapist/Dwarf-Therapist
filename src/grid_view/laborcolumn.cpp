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

LaborColumn::LaborColumn(QString title, int labor_id, int skill_id, ViewColumnSet *set, QObject *parent) 
	: ViewColumn(title, CT_LABOR, set, parent)
	, m_labor_id(labor_id)
	, m_skill_id(skill_id)
{}

LaborColumn::LaborColumn(QSettings &s, ViewColumnSet *set, QObject *parent) 
	: ViewColumn(s, set, parent)
	, m_labor_id(s.value("labor_id", -1).toInt())
	, m_skill_id(s.value("skill_id", -1).toInt())
{}

LaborColumn::LaborColumn(const LaborColumn &to_copy) 
    : ViewColumn(to_copy)
    , m_labor_id(to_copy.m_labor_id)
    , m_skill_id(to_copy.m_skill_id)
{}

QStandardItem *LaborColumn::build_cell(Dwarf *d) {
	GameDataReader *gdr = GameDataReader::ptr();
	QStandardItem *item = init_cell(d);

	item->setData(CT_LABOR, DwarfModel::DR_COL_TYPE);
	short rating = d->get_rating_by_skill(m_skill_id);
	if (rating < 0 && d->labor_enabled(m_labor_id)) {
		item->setData(float(rating + 0.5f), DwarfModel::DR_SORT_VALUE); // push assigned labors above no exp in sort order
	} else {
		item->setData(rating, DwarfModel::DR_SORT_VALUE);		
	}
	item->setData(rating, DwarfModel::DR_RATING);
	item->setData(m_labor_id, DwarfModel::DR_LABOR_ID);
	item->setData(m_set->name(), DwarfModel::DR_SET_NAME);
	
	QString skill_str;
	if (m_skill_id != -1 && rating > -1) {
		QString adjusted_rating = QString::number(rating);
		if (rating > 15)
			adjusted_rating = QString("15 +%1").arg(rating - 15);
		skill_str = tr("%1 %2<br/>[RAW LEVEL: <b><font color=blue>%3</font></b>]<br/><b>Experience:</b><br/>%4")
			.arg(gdr->get_skill_level_name(rating))
			.arg(gdr->get_skill_name(m_skill_id))
			.arg(adjusted_rating)
			.arg(d->get_skill(m_skill_id).exp_summary());
	} else {
		// either the skill isn't a valid id, or they have 0 experience in it
		skill_str = "0 experience";
	}
	item->setToolTip(QString("<h3>%1</h3>%2<h4>%3</h4>").arg(m_title).arg(skill_str).arg(d->nice_name()));
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