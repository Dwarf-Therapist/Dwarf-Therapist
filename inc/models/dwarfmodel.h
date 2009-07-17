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
		DR_IS_AGGREGATE,
		DR_DIRTY,
		DR_LABOR_ID,
		DR_GROUP_NAME,
		DR_ID,
		DR_DUMMY // used as an int counter that increments to force re-draws
	} DATA_ROLES;

	DwarfModel(QObject *parent = 0);
	//virtual ~DwarfModel();
	void set_instance(DFInstance *df) {m_df = df;}
	
	GROUP_BY current_grouping() const {return m_group_by;}
	const QMap<QString, QVector<Dwarf*>> *get_dwarf_groups() const {return &m_grouped_dwarves;}
	Dwarf *get_dwarf_by_id(int id) const {return m_dwarves.value(id, 0);}
		
	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

	QVector<Dwarf*> get_dirty_dwarves();

	public slots:
		void set_group_by(int group_by);
		void load_dwarves();
		void labor_clicked(const QModelIndex &idx);
		void clear_pending();
		void commit_pending();

private:
	DFInstance *m_df;
	QVector<QStringList> m_labor_cols;
	QMap<int, Dwarf*> m_dwarves;
	QMap<QString, QVector<Dwarf*>> m_grouped_dwarves;
	GROUP_BY m_group_by;
	void calculate_pending();

	void build_rows();

signals:
	void new_pending_changes(int);
};
#endif
