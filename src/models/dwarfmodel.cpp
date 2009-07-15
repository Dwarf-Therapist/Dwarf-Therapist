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
	QList<QStandardItem*> items;
	items << new QStandardItem;
	int i = 1;
	setHorizontalHeaderItem(0, new QStandardItem("woot"));
	foreach(QString k, keys) {
		int labor_id = gdr->get_keys("labor_table/" + k)[0].toInt();
		QString labor_name_key = QString("labor_table/%1/%2").arg(k).arg(labor_id);
		QString labor_name = gdr->get_string_for_key(labor_name_key);
		
		QStringList labor;
		labor << QString::number(labor_id) << labor_name;
		m_labor_cols << labor;
		items << new QStandardItem(labor_name);
		setHorizontalHeaderItem(i++, new QStandardItem(labor_name));
	}
	//appendRow(items);
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
	removeRows(1, rowCount()-1);

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
			root_row << root;
		}

		if (root) { // we have a parent, so we should draw an aggregate row
			foreach(QStringList l, m_labor_cols) {
				int labor_id = l[0].toInt();
				QString labor_name = l[1];

				QStandardItem *item = new QStandardItem();
				item->setText(0);
				item->setData(0, DR_ENABLED);
				item->setData(labor_id, DR_LABOR_ID);
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

			QList<QStandardItem*> items;
			items << i_name;
			foreach(QStringList l, m_labor_cols) {
				int labor_id = l[0].toInt();
				QString labor_name = l[1];
				short rating = d->get_rating_for_skill(labor_id);
				bool enabled = d->is_labor_enabled(labor_id);

				QStandardItem *item = new QStandardItem();
				item->setData(rating, DR_RATING);
				item->setData(enabled, DR_ENABLED);
				item->setData(labor_id, DR_LABOR_ID);
				item->setData(d->id(), DR_ID);

				item->setToolTip(QString("<h3>%2</h3><h4>%3</h4>%1").arg(d->nice_name()).arg(labor_name).arg(QString::number(rating)));
				item->setStatusTip(labor_name + " :: " + d->nice_name());
				items << item;

				if (m_group_by != GB_NOTHING && enabled) {
					// update our aggregate column
					int i = items.size() - 1;
					int cnt = root_row[i]->data(DR_ENABLED).toInt();
					root_row[i]->setData(cnt + 1, DR_ENABLED);
				}
			}
			if (root) {
				root->appendRow(items);
			} else {
				appendRow(items);
			}
		}
		if (root) {
			appendRow(root_row);
		}
	}
}

void DwarfModel::labor_clicked(const QModelIndex &idx) {
	if (idx.row() == 0 && idx.column() == 0) {
		setSortRole(Qt::DisplayRole);
		sort(0, Qt::AscendingOrder);
		return;
	}
	
	if (idx.row() == 0 && idx.column() > 0) {
		if (m_group_by == GB_NOTHING || (m_group_by != GB_NOTHING && !idx.parent().isValid())) {
			// this is a header, so sort it
			setSortRole(DR_RATING);
			sort(idx.column(), Qt::DescendingOrder);
		}
	}
	return;
	
	QStandardItem *item = itemFromIndex(idx);


	int dwarf_id = idx.data(DR_ID).toInt();
	if (!dwarf_id) {
		return;
	}
	m_dwarves[dwarf_id]->toggle_labor(idx.data(DR_LABOR_ID).toInt());
	setData(idx, !idx.data(DR_ENABLED).toBool(), DR_ENABLED);
	qDebug() << "toggling" << idx.data(DR_LABOR_ID).toInt() << "for dwarf:" << idx.data(DR_ID).toInt();
}

void DwarfModel::set_group_by(int group_by) {
	qDebug() << "group_by" << group_by;
	m_group_by = static_cast<GROUP_BY>(group_by);
	if (m_df)
		load_dwarves();
}
