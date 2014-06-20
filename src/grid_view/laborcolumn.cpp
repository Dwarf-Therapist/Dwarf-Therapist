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
#include "dfinstance.h"
#include "viewmanager.h"
#include "skill.h"
#include "labor.h"

LaborColumn::LaborColumn(QString title, int labor_id, int skill_id, ViewColumnSet *set, QObject *parent) 
    : SkillColumn(title, -1, set, parent, CT_LABOR)
	, m_labor_id(labor_id)
	, m_skill_id(skill_id)
{
    connect(DT,SIGNAL(labor_counts_updated()), this, SLOT(update_count()));
    m_current_sort = ViewManager::get_default_col_sort(CT_LABOR);
}

LaborColumn::LaborColumn(QSettings &s, ViewColumnSet *set, QObject *parent) 
    : SkillColumn(s, set, parent)
	, m_labor_id(s.value("labor_id", -1).toInt())
	, m_skill_id(s.value("skill_id", -1).toInt())
{
    connect(DT,SIGNAL(labor_counts_updated()), this, SLOT(update_count()));
    m_current_sort = ViewManager::get_default_col_sort(CT_LABOR);
}

LaborColumn::LaborColumn(const LaborColumn &to_copy) 
    : SkillColumn(to_copy)
    , m_labor_id(to_copy.m_labor_id)
    , m_skill_id(to_copy.m_skill_id)
{
    connect(DT,SIGNAL(labor_counts_updated()), this, SLOT(update_count()));
    m_sortable_types = to_copy.m_sortable_types;
}

QStandardItem *LaborColumn::build_cell(Dwarf *d) {
	QStandardItem *item = init_cell(d);    
    m_sort_val = 0;    

    if(d->labor_enabled(m_labor_id))
        item->setData(1000, DwarfModel::DR_BASE_SORT);

    item->setData(CT_LABOR, DwarfModel::DR_COL_TYPE);
    item->setData(d->skill_level(m_skill_id,false,true), DwarfModel::DR_RATING); //interpolated level
    item->setData(d->skill_level(m_skill_id), DwarfModel::DR_DISPLAY_RATING); //level rounded down
	item->setData(m_labor_id, DwarfModel::DR_LABOR_ID);
	item->setData(m_set->name(), DwarfModel::DR_SET_NAME);
    set_export_role(DwarfModel::DR_RATING);

    refresh_sort(d, m_current_sort);
    build_tooltip(d, DT->user_settings()->value(QString("options/show_roles_in_labor"), true).toBool());

	return item;
}

QStandardItem *LaborColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) {
    Q_UNUSED(dwarves);
    QStandardItem *item = init_aggregate(group_name);
    item->setData(CT_LABOR, DwarfModel::DR_COL_TYPE);
	item->setData(m_labor_id, DwarfModel::DR_LABOR_ID);	
	return item;
}

void LaborColumn::write_to_ini(QSettings &s) {
	ViewColumn::write_to_ini(s); 
	s.setValue("skill_id", m_skill_id); 
	s.setValue("labor_id", m_labor_id);
}

void LaborColumn::update_count(){    
    m_count = DT->get_DFInstance()->get_labor_count(m_labor_id);
}
