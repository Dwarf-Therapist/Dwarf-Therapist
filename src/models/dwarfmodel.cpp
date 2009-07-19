#include <QtCore>
#include <QtDebug>

#include "dfinstance.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "skill.h"
#include "labor.h"
#include "statetableview.h"

DwarfModel::DwarfModel(QObject *parent)
	: QStandardItemModel(parent)
	, m_df(0)
	, m_group_by(GB_NOTHING)
{
	GameDataReader *gdr = GameDataReader::ptr();
	//QStringList keys = gdr->get_child_groups("labors");
	setHorizontalHeaderItem(0, new QStandardItem);
	QMap<int, Labor*> labors = gdr->get_ordered_labors();
	foreach(Labor *l, labors) {
		setHorizontalHeaderItem(l->list_order + 1, new QStandardItem(l->name));
	}
	
	/*int i = 0;
	foreach(QString k, keys) {
		int labor_id = gdr->get_int_for_key(QString("labors/%1/id").arg(i));
		QString labor_name = gdr->get_string_for_key(QString("labors/%1/name").arg(i));

		QStringList labor;
		labor << QString::number(labor_id) << labor_name;
		m_labor_cols << labor;
		setHorizontalHeaderItem(i + 1, new QStandardItem(labor_name));
		i++;
	}*/
}

void DwarfModel::sort(int column, Qt::SortOrder order) {
	if (column == 0)
		setSortRole(Qt::DisplayRole);
	else
		setSortRole(DR_RATING);
	QStandardItemModel::sort(column, order);
}

void DwarfModel::filter_changed(const QString &needle) {
}

void DwarfModel::section_clicked(int col, Qt::MouseButton btn) {
	if (btn == Qt::LeftButton) {
		if (col == m_selected_col) {
			// turn it off
			m_selected_col = -1;
		} else {
			m_selected_col = col;
		}
		emit dataChanged(index(0, col), index(rowCount()-1, col));

	} else if (btn == Qt::RightButton) {
		// sort
		if (col == 0)
			sort(col, Qt::AscendingOrder);
		else
			sort(col, Qt::DescendingOrder);
	}
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
			case GB_LEGENDARY:
				int legendary_skills = 0;
				foreach(Skill s, *d->get_skills()) {
					if (s.rating() >= 15)
						legendary_skills++;
				}
				if (legendary_skills)
					m_grouped_dwarves["Legends"].append(d);
				else
					m_grouped_dwarves["Losers"].append(d);
				break;
		}
	}
	build_rows();
}

void DwarfModel::build_rows() {
	GameDataReader *gdr = GameDataReader::ptr();
	QMap<int, Labor*> labors = gdr->get_ordered_labors();

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
			foreach(Labor *l, labors) {
				QStandardItem *item = new QStandardItem();
				//item->setText(0);
				item->setData(true, DR_IS_AGGREGATE);
				item->setData(l->labor_id, DR_LABOR_ID);
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
			qSort(*skills);
			for (int i = skills->size() - 1; i >= 0; --i) {
				skill_summary += skills->at(i).to_string() + "\n";
			}
			i_name->setToolTip(skill_summary);
			i_name->setStatusTip(d->nice_name());
			i_name->setData(false, DR_IS_AGGREGATE);
			i_name->setData(d->id(), DR_ID);

			QList<QStandardItem*> items;
			items << i_name;
			foreach(Labor *l, labors) {
				short rating = d->get_rating_for_skill(l->labor_id);
				//bool enabled = d->is_labor_enabled(labor_id);

				QStandardItem *item = new QStandardItem();
				
				item->setData(false, DR_IS_AGGREGATE);
				item->setData(rating, DR_RATING); // for sort order
				item->setData(l->labor_id, DR_LABOR_ID);
				item->setData(false, DR_DIRTY);
				item->setData(d->id(), DR_ID);
				item->setData(0, DR_DUMMY);
				QString tooltip = "<h3>" + l->name + "</h3>";
				tooltip += gdr->get_skill_level_name(rating) + " " + gdr->get_skill_name(l->skill_id) + " (" + QString::number(rating) + ")";
				tooltip += "\n<h4>" + d->nice_name() + "</h4>";
				item->setToolTip(tooltip);
				item->setStatusTip(l->name + " :: " + d->nice_name());
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
	foreach(Dwarf *d, m_dwarves) {
		if (d->pending_changes()) {
			d->commit_pending();
		}
	}
	emit reset();
	emit new_pending_changes(0);
}

QVector<Dwarf*> DwarfModel::get_dirty_dwarves() {
	QVector<Dwarf*> dwarves;
	foreach(Dwarf *d, m_dwarves) {
		if (d->pending_changes())
			dwarves.append(d);
	}
	return dwarves;
}