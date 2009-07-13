#include <QStandardItem>
#include "dfinstance.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "skill.h"
#include "statetableview.h"

#include <QtDebug>

DwarfModel::DwarfModel(QObject *parent)
	: QStandardItemModel(parent)
	, m_dwarves(QMap<int, Dwarf*>())
{
	GameDataReader *gdr = GameDataReader::ptr();
	QStringList keys = gdr->get_child_groups("labor_table");
	QList<QStandardItem*> items;
	items << new QStandardItem;
	foreach(QString k, keys) {
		int labor_id = gdr->get_keys("labor_table/" + k)[0].toInt();
		QString labor_name_key = QString("labor_table/%1/%2").arg(k).arg(labor_id);
		QString labor_name = gdr->get_string_for_key(labor_name_key);
		
		QStringList labor;
		labor << QString::number(labor_id) << labor_name;
		m_labor_cols << labor;
		items << new QStandardItem(labor_name);
	}
	appendRow(items);
}

void DwarfModel::load_dwarves(DFInstance *df) {
	foreach(Dwarf *d, m_dwarves) {
		delete d;
	}
	m_dwarves.clear();
	removeRows(1, rowCount()-1);

	foreach(Dwarf *d, df->load_dwarves()) {
		m_dwarves[d->id()] = d;
		
		QStandardItem *i_name = new QStandardItem(d->nice_name());

		QString skill_summary;
		QVector<Skill> *skills = d->get_skills();
		foreach(Skill s, *skills) {
			skill_summary += s.to_string();
			skill_summary += "\n";
		}
		i_name->setToolTip(skill_summary);

		QList<QStandardItem*> items;
		items << i_name;
		foreach(QStringList l, m_labor_cols) {
			int labor_id = l[0].toInt();
			QString labor_name = l[1];
			short rating = d->get_rating_for_skill(labor_id);

			QStandardItem *item = new QStandardItem();
			item->setData(QVariant(rating), DR_RATING);
			item->setData(QVariant(d->is_labor_enabled(labor_id)), DR_ENABLED);
			item->setData(labor_id, DR_LABOR_ID);
			item->setData(d->id(), DR_ID);

			item->setToolTip(QString("<h3>%2</h3><h4>%3</h4>%1").arg(d->nice_name()).arg(labor_name).arg(QString::number(rating)));
			items << item;
		}
		appendRow(items);
	}
}

void DwarfModel::labor_clicked(const QModelIndex &idx) {
	int dwarf_id = idx.data(DR_ID).toInt();
	m_dwarves[dwarf_id]->toggle_labor(idx.data(DR_LABOR_ID).toInt());
	setData(idx, !idx.data(DR_ENABLED).toBool(), DR_ENABLED);
	qDebug() << "toggling" << idx.data(DR_LABOR_ID).toInt() << "for dwarf:" << idx.data(DR_ID).toInt();
}

