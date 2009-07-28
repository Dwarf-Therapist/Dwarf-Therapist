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
#include <QtCore>
#include <QtDebug>

#include "dfinstance.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "skill.h"
#include "labor.h"
#include "statetableview.h"
#include "defines.h"


DwarfModelProxy::DwarfModelProxy(QObject *parent)
	:QSortFilterProxyModel(parent)
{}

DwarfModel* DwarfModelProxy::get_dwarf_model() const {
	//QStandardItemModel *tmp = static_cast<QStandardItemModel*>(sourceModel());
	return static_cast<DwarfModel*>(sourceModel());
}

void DwarfModelProxy::labor_clicked(const QModelIndex &idx) {
	bool valid = idx.isValid();
	QModelIndex new_idx = mapToSource(idx);
	valid = new_idx.isValid();
	return get_dwarf_model()->labor_clicked(new_idx);
}

void DwarfModelProxy::setFilterFixedString(const QString &pattern) {
	m_filter_text = pattern;
	QSortFilterProxyModel::setFilterFixedString(pattern);
}

bool DwarfModelProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
	//return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
	const DwarfModel *m = get_dwarf_model();
	if (m->current_grouping() == DwarfModel::GB_NOTHING) {
		QModelIndex idx = m->index(source_row, 0, source_parent);
		QString data = m->data(idx, filterRole()).toString();
		return data.contains(m_filter_text, Qt::CaseInsensitive);	
	} else {
		QModelIndex tmp_idx = m->index(source_row, 0, source_parent);
		QStandardItem *item = m->itemFromIndex(tmp_idx);
		if (m->data(tmp_idx, DwarfModel::DR_IS_AGGREGATE).toBool()) {
			int matches = 0;
			for(int i = 0; i < item->rowCount(); ++i) {
				if (filterAcceptsRow(i, tmp_idx)) // a child matches
					matches++;
			}
			return matches > 0;
		} else {
			QModelIndex idx = m->index(source_row, 0, source_parent);
			QString data = m->data(idx, filterRole()).toString();
			return data.contains(m_filter_text, Qt::CaseInsensitive);	
		}
	}
	return true;
}

bool DwarfModelProxy::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const {
	/*
	if (!mShowLineAndFile && source_column == 1 || source_column == 2) {
		return false;
	} else {
		return QSortFilterProxyModel::filterAcceptsColumn(source_column, source_parent);
	}
	*/
	return true;
}

void DwarfModelProxy::sort(int column, Qt::SortOrder order) {
	if (column == 0) {
		setSortRole(Qt::DisplayRole);
	} else {
		if (sortColumn() != column && column > 0) {
			order = Qt::DescendingOrder;
		}
		setSortRole(DwarfModel::DR_RATING);
	}
	QSortFilterProxyModel::sort(column, order);
}

/////////////////////////////////////////

DwarfModel::DwarfModel(QObject *parent)
	: QStandardItemModel(parent)
	, m_df(0)
	, m_group_by(GB_NOTHING)
{
	GameDataReader *gdr = GameDataReader::ptr();
	setHorizontalHeaderItem(0, new QStandardItem);
	QMap<int, Labor*> labors = gdr->get_ordered_labors();
	foreach(Labor *l, labors) {
		setHorizontalHeaderItem(l->list_order + 1, new QStandardItem(l->name));
	}
}

void DwarfModel::section_right_clicked(int col) {
	if (col == m_selected_col) {
		// turn it off
		m_selected_col = -1;
	} else {
		m_selected_col = col;
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
				{
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
			case GB_SEX:
				if (d->is_male())
					m_grouped_dwarves["Males"].append(d);
				else
					m_grouped_dwarves["Females"].append(d);
				break;
			case GB_HAPPINESS:
				m_grouped_dwarves[QString::number(d->get_happiness())].append(d);
				break;
		}
	}
	build_rows();
}

void DwarfModel::build_rows() {
	GameDataReader *gdr = GameDataReader::ptr();
	QMap<int, Labor*> labors = gdr->get_ordered_labors();

	QIcon icn_f(":img/female.png");
	QIcon icn_m(":img/male.png");

	foreach(QString key, m_grouped_dwarves.uniqueKeys()) {
		QStandardItem *root = 0;
		QList<QStandardItem*> root_row;

		if (m_group_by != GB_NOTHING) {
			// we need a root element to hold group members...
			QString title = QString("%1 (%2)").arg(key).arg(m_grouped_dwarves.value(key).size());
			root = new QStandardItem(title);
			root->setData(true, DR_IS_AGGREGATE);
			root->setData(0, DR_RATING);
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
				item->setData(0, DR_RATING);
				item->setData(0, DR_DUMMY);
				root_row << item;
			}
		}
		
		foreach(Dwarf *d, m_grouped_dwarves.value(key)) {
			QStandardItem *i_name = new QStandardItem(d->nice_name());
			QString skill_summary = "<h4>Happiness Level: " + QString::number(d->get_happiness()) + "</h4><h4>Skills</h4><ul>";
			QVector<Skill> *skills = d->get_skills();
			qSort(*skills);
			for (int i = skills->size() - 1; i >= 0; --i) {
				skill_summary += "<li>" + skills->at(i).to_string() + "</li>";
			}
			skill_summary += "</ul>";
			i_name->setToolTip(skill_summary);
			i_name->setStatusTip(d->nice_name());
			i_name->setData(false, DR_IS_AGGREGATE);
			i_name->setData(0, DR_RATING);
			i_name->setData(d->id(), DR_ID);
			
			/* SEX ICON
			if (d->is_male()) {
				i_name->setIcon(icn_m);
			} else {
				i_name->setIcon(icn_f);
			}
			*/
			
			switch (d->get_happiness()) {
				case Dwarf::DH_MISERABLE:
					i_name->setIcon(QIcon(":img/exclamation.png"));
					break;
				case Dwarf::DH_UNHAPPY:
					i_name->setIcon(QIcon(":img/emoticon_unhappy.png"));
					break;
				case Dwarf::DH_FINE:
					i_name->setIcon(QIcon(":img/emoticon_smile.png"));
					break;
				case Dwarf::DH_CONTENT:
					i_name->setIcon(QIcon(":img/emoticon_smile.png"));
					break;
				case Dwarf::DH_HAPPY:
					i_name->setIcon(QIcon(":img/emoticon_grin.png"));
					break;
				case Dwarf::DH_ECSTATIC:
					i_name->setIcon(QIcon(":img/emoticon_happy.png"));
					break;
			}

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
				if (l->skill_id != -1)
					tooltip += gdr->get_skill_level_name(rating) + " " + gdr->get_skill_name(l->skill_id) + " (" + QString::number(rating) + ")";
				tooltip += "\n<h4>" + d->nice_name() + "</h4>";
				item->setToolTip(tooltip);
				item->setStatusTip(l->name + " :: " + d->nice_name());
				items << item;

				if (root) {
					short highest_rating = root_row[items.size()-1]->data(DR_RATING).toInt();
					if (rating > highest_rating) {
						root_row[items.length() -1]->setData(rating, DR_RATING); // aggregate should hold the highest sub skill value
						//root->setData(rating, DR_RATING); // so the parents can be sorted by rating before the labors
					}
				}
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
	QStandardItem *item = itemFromIndex(idx);
	bool valid = idx.isValid();
	Q_ASSERT(item);

	bool is_aggregate = item->data(DR_IS_AGGREGATE).toBool();
	int labor_id = item->data(DR_LABOR_ID).toInt();
	int dwarf_id = item->data(DR_ID).toInt();
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
	LOGD << "group_by now set to" << group_by;
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
	load_dwarves();
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