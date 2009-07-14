#ifndef DWARFMODEL_H
#define DWARFMODEL_H

#include <Qt>
#include <QStandardItemModel>
class Dwarf;
class DFInstance;

class DwarfModel : public QStandardItemModel {
	Q_OBJECT
public:
	typedef enum {
		GB_NOTHING = 0,
		GB_PROFESSION = 1,
		GB_TOTAL
	} GROUP_BY;
	typedef enum {
		DR_RATING = Qt::ItemDataRole::UserRole + 1,
		DR_SKILL_NAME,
		DR_ENABLED,
		DR_LABOR_ID,
		DR_ID,
		DR_HAS_CHILDREN
	} DATA_ROLES;

	DwarfModel(QObject *parent = 0);
	//virtual ~DwarfModel();

	public slots:
		void load_dwarves(DFInstance *df);
		void labor_clicked(const QModelIndex &idx);

private:
	QVector<QStringList> m_labor_cols;
	QMap<int, Dwarf*> m_dwarves;

signals:
	void toggle_labor(int labor_id, int dwarf_id);
};
#endif
