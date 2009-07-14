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
		DR_RATING = Qt::UserRole + 1,
		DR_SKILL_NAME,
		DR_ENABLED,
		DR_LABOR_ID,
		DR_ID,
		DR_EXPANDED
	} DATA_ROLES;

	DwarfModel(QObject *parent = 0);
	//virtual ~DwarfModel();
	void set_instance(DFInstance *df) {m_df = df;}
	
	GROUP_BY current_grouping() const {return m_group_by;}
	public slots:
		void set_group_by(int group_by);
		void load_dwarves();
		void labor_clicked(const QModelIndex &idx);

private:
	DFInstance *m_df;
	QVector<QStringList> m_labor_cols;
	QMap<int, Dwarf*> m_dwarves;
	QMap<QString, QVector<Dwarf*>> m_grouped_dwarves;
	GROUP_BY m_group_by;

	void build_rows();

	

signals:
	void toggle_labor(int labor_id, int dwarf_id);
};
#endif
