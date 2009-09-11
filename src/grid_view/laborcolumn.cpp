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

QStandardItem *LaborColumn::build_cell(Dwarf *d) {
	GameDataReader *gdr = GameDataReader::ptr();
	QStandardItem *item = init_cell(d);

	item->setData(CT_LABOR, DwarfModel::DR_COL_TYPE);
	short rating = d->get_rating_by_skill(m_skill_id);
	item->setData(rating, DwarfModel::DR_RATING); // for sort order
	item->setData(m_labor_id, DwarfModel::DR_LABOR_ID);
	item->setData(m_set->name(), DwarfModel::DR_SET_NAME);
	
	QString skill_str;
	if (m_skill_id != -1)
		skill_str = QString("%1 %2 (%3) %4exp")
			.arg(gdr->get_skill_level_name(rating))
			.arg(gdr->get_skill_name(m_skill_id))
			.arg(rating)
			.arg(d->get_skill(m_skill_id).exp());
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
	item->setData(0, DwarfModel::DR_DUMMY);
	item->setData(m_set->name(), DwarfModel::DR_SET_NAME);
	return item;
}