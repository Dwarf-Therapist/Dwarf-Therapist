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

#include "dwarfmodelproxy.h"
#include "dwarfmodel.h"

DwarfModelProxy::DwarfModelProxy(QObject *parent)
	:QSortFilterProxyModel(parent)
{}

DwarfModel* DwarfModelProxy::get_dwarf_model() const {
	//QStandardItemModel *tmp = static_cast<QStandardItemModel*>(sourceModel());
	return static_cast<DwarfModel*>(sourceModel());
}

void DwarfModelProxy::cell_activated(const QModelIndex &idx) {
	bool valid = idx.isValid();
	QModelIndex new_idx = mapToSource(idx);
	valid = new_idx.isValid();
	return get_dwarf_model()->cell_activated(new_idx);
}

void DwarfModelProxy::setFilterFixedString(const QString &pattern) {
	m_filter_text = pattern;
	QSortFilterProxyModel::setFilterFixedString(pattern);
}

bool DwarfModelProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
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
	return true;
}

void DwarfModelProxy::sort(int column, Qt::SortOrder order) {
	if (column == 0) {
		DwarfModel *dm = get_dwarf_model();
		if (dm->current_grouping() == DwarfModel::GB_PROFESSION) {
			return;
		}
		setSortRole(Qt::DisplayRole);
	} else {
		if (sortColumn() != column) {
			order = Qt::DescendingOrder;
		}
		setSortRole(DwarfModel::DR_RATING);
	}
	QSortFilterProxyModel::sort(column, order);
}
