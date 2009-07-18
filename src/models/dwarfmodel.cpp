#include <QtCore>
#include <QtDebug>

#include "dfinstance.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "skill.h"
#include "statetableview.h"

DwarfModel::DwarfModel(QObject *parent)
	: QStandardItemModel(parent)
	, m_df(0)
	, m_group_by(GB_NOTHING)
{
	GameDataReader *gdr = GameDataReader::ptr();
	QStringList keys = gdr->get_child_groups("labor_table");
	
	setHorizontalHeaderItem(0, new QStandardItem);
	int i = 1;
	foreach(QString k, keys) {
		int labor_id = gdr->get_keys("labor_table/" + k)[0].toInt();
		QString labor_name_key = QString("labor_table/%1/%2").arg(k).arg(labor_id);
		QString labor_name = gdr->get_string_for_key(labor_name_key);
		
		QStringList labor;
		labor << QString::number(labor_id) << labor_name;
		m_labor_cols << labor;
		setHorizontalHeaderItem(i++, new QStandardItem(labor_name));
	}
}

void DwarfModel::sort(int column, Qt::SortOrder order) {
	if (column == 0)
		setSortRole(Qt::DisplayRole);
	else
		setSortRole(DR_RATING);
	QStandardItemModel::sort(column, order);
}


void DwarfModel::load_dwarves() {
	// clear id->dwarf map
	foreach(Dwarf *d, m_dwarves) {
		delete d;
	}
	m_dwarves.clear();

	// don't need to go delete the dwarf pointers in here, since the earlier foreach should have
	// deleted them
	m_grouped_dwarves.clear();

	// remove rows except for the header
	removeRows(0, rowCount());

	// populate dwarf maps
	foreach(Dwarf *d, m_df->load_dwarves()) {
		m_dwarves[d->id()] = d;
		switch (m_group_by) {
			case GB_NOTHING:
				m_grouped_dwarves[QString::number(d->id())].append(d);
				break;
			case GB_PROFESSION:
				m_grouped_dwarves[d->profession()].append(d);
				break;
		}
	}
	build_rows();
}

void DwarfModel::build_rows() {
	foreach(QString key, m_grouped_dwarves.uniqueKeys()) {
		QStandardItem *root = 0;
		QList<QStandardItem*> root_row;

		if (m_group_by != GB_NOTHING) {
			// we need a root element to hold group members...
			QString title = QString("%1 (%2)").arg(key).arg(m_grouped_dwarves.value(key).size());
			root = new QStandardItem(title);
			root->setData(true, DR_IS_AGGREGATE);
			root_row << root;
		}

		if (root) { // we have a parent, so we should draw an aggregate row
			foreach(QStringList l, m_labor_cols) {
				int labor_id = l[0].toInt();
				QString labor_name = l[1];

				QStandardItem *item = new QStandardItem();
				item->setText(0);
				item->setData(true, DR_IS_AGGREGATE);
				item->setData(labor_id, DR_LABOR_ID);
				item->setData(key, DR_GROUP_NAME);
				item->setData(false, DR_DIRTY);
				item->setData(0, DR_DUMMY);
				root_row << item;
			}
		}
		
		foreach(Dwarf *d, m_grouped_dwarves.value(key)) {
			QStandardItem *i_name = new QStandardItem(d->nice_name());
			QString skill_summary;
			QVector<Skill> *skills = d->get_skills();
			foreach(Skill s, *skills) {
				skill_summary += s.to_string();
				skill_summary += "\n";
			}
			i_name->setToolTip(skill_summary);
			i_name->setStatusTip(d->nice_name());
			i_name->setData(false, DR_IS_AGGREGATE);
			i_name->setData(d->id(), DR_ID);

			QList<QStandardItem*> items;
			items << i_name;
			foreach(QStringList l, m_labor_cols) {
				int labor_id = l[0].toInt();
				QString labor_name = l[1];
				short rating = d->get_rating_for_skill(labor_id);
				//bool enabled = d->is_labor_enabled(labor_id);

				QStandardItem *item = new QStandardItem();
				
				item->setData(false, DR_IS_AGGREGATE);
				item->setData(rating, DR_RATING); // for sort order
				item->setData(labor_id, DR_LABOR_ID);
				item->setData(false, DR_DIRTY);
				item->setData(d->id(), DR_ID);
				item->setData(0, DR_DUMMY);

				item->setToolTip(QString("<h3>%2</h3><h4>%3</h4>%1").arg(d->nice_name()).arg(labor_name).arg(QString::number(rating)));
				item->setStatusTip(labor_name + " :: " + d->nice_name());
				items << item;
			}
			if (root) {
				root->appendRow(items);
			} else {
				appendRow(items);
			}
			d->m_name_idx = indexFromItem(i_name);
		}
		if (root) {
			appendRow(root_row);
		}
	}
}

void DwarfModel::labor_clicked(const QModelIndex &idx) {
	if (idx.column() == 0)
		return; // don't mess with the names
	bool is_aggregate = idx.data(DR_IS_AGGREGATE).toBool();
	int labor_id = idx.data(DR_LABOR_ID).toInt();
	int dwarf_id = idx.data(DR_ID).toInt();
	if (is_aggregate) {
		QModelIndex first_col = idx.sibling(idx.row(), 0);
		// first find out how many are enabled...
		int enabled_count = 0;
		QString group_name = idx.data(DwarfModel::DR_GROUP_NAME).toString();
		int children = rowCount(first_col);

		foreach(Dwarf *d, m_grouped_dwarves.value(group_name)) {
			if (d->is_labor_enabled(labor_id))
				enabled_count++;
		}

		// if none or some are enabled, enable all of them
		bool enabled = (enabled_count < children);
		foreach(Dwarf *d, m_grouped_dwarves.value(group_name)) {
			d->set_labor(labor_id, enabled);
		}

		// tell the view what we touched...
		setData(idx, idx.data(DR_DUMMY).toInt()+1, DR_DUMMY); // redraw the aggregate...
		for(int i = 0; i < rowCount(first_col); ++i) {
			QModelIndex tmp_index = index(i, idx.column(), first_col);
			setData(tmp_index, tmp_index.data(DR_DUMMY).toInt()+1, DR_DUMMY);
		}
	} else {
		QModelIndex aggregate_col = index(idx.parent().row(), idx.column());
		if (aggregate_col.isValid())
			setData(aggregate_col, aggregate_col.data(DR_DUMMY).toInt()+1, DR_DUMMY); // redraw the aggregate...
		setData(idx, idx.data(DR_DUMMY).toInt()+1, DR_DUMMY); // redraw the aggregate...
		m_dwarves[dwarf_id]->toggle_labor(labor_id);
	}
	calculate_pending();
	//qDebug() << "toggling" << labor_id << "for dwarf:" << dwarf_id;
}

void DwarfModel::set_group_by(int group_by) {
	qDebug() << "group_by" << group_by;
	m_group_by = static_cast<GROUP_BY>(group_by);
	if (m_df)
		load_dwarves();
}

void DwarfModel::calculate_pending() {
	int changes = 0;
	foreach(Dwarf *d, m_dwarves) {
		changes += d->pending_changes();
	}
	emit new_pending_changes(changes);
}

void DwarfModel::clear_pending() {
	foreach(Dwarf *d, m_dwarves) {
		if (d->pending_changes()) {
			d->clear_pending();
		}
	}
	emit reset();
	emit new_pending_changes(0);
}

void DwarfModel::commit_pending() {
	return;
}

QVector<Dwarf*> DwarfModel::get_dirty_dwarves() {
	QVector<Dwarf*> dwarves;
	foreach(Dwarf *d, m_dwarves) {
		if (d->pending_changes())
			dwarves.append(d);
	}
	return dwarves;
}