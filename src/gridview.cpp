#include "gridview.h"
#include "dwarfmodel.h"
#include "dwarf.h"

ViewColumn::ViewColumn(QString title, ViewColumnSet *set, QObject *parent)
	: QObject(parent)
	, m_title(title)
	, m_set(set)
	, m_override_set_colors(false)
{
	if (set)
		set->add_column(this);
}

QStandardItem *ViewColumn::build_cell(Dwarf *d) {
	GameDataReader *gdr = GameDataReader::ptr();
	QStandardItem *item = new QStandardItem;

	short rating = d->get_rating_for_skill(m_labor_id);
	item->setData(set()->bg_color(), Qt::BackgroundColorRole);
	item->setData(set()->bg_color(), DwarfModel::DR_DEFAULT_BG_COLOR);
	item->setData(false, DwarfModel::DR_IS_AGGREGATE);
	item->setData(rating, DwarfModel::DR_RATING); // for sort order
	item->setData(m_labor_id, DwarfModel::DR_LABOR_ID);
	item->setData(false, DwarfModel::DR_DIRTY);
	item->setData(d->id(), DwarfModel::DR_ID);
	item->setData(0, DwarfModel::DR_DUMMY);
	QString tooltip = "<h3>" + m_title + "</h3>";
	if (m_skill_id != -1)
		tooltip += gdr->get_skill_level_name(rating) + " " + gdr->get_skill_name(m_skill_id) + " (" + QString::number(rating) + ")";
	tooltip += "\n<h4>" + d->nice_name() + "</h4>";
	item->setToolTip(tooltip);
	item->setStatusTip(m_title + " :: " + d->nice_name());
	
	return item;
}

LaborColumn::LaborColumn(QString title, int labor_id, int skill_id, ViewColumnSet *set, QObject *parent) 
	: ViewColumn(title, set, parent)
	, m_labor_id(labor_id)
	, m_skill_id(skill_id)
{}

ViewColumnSet::ViewColumnSet(QString name, QObject *parent)
	: QObject(parent)
	, m_name(name)
{}

GridView::GridView(QString name, QObject *parent)
	: QObject(parent)
	, m_name(name)
{}