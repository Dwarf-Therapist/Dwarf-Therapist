#ifndef GRID_VIEW_H
#define GRID_VIEW_H

#include <QtGui>

class GridView;
class ViewColumnSet;
class Dwarf;

/*!
ViewColumn
TODO: maybe subclass these for different accessors
I can think of a need for:
* Labor cols
* status cols (happiness, wounds, etc...)
* radio cols (choose which weapon skill, or armor type)
* simple counters (num kills, num kids, total gold, total items)
*/
class ViewColumn : public QObject {
	Q_OBJECT
public:
	ViewColumn(QString title, ViewColumnSet *set = 0, QObject *parent = 0);

	QString title() {return m_title;}
	void set_title(QString title) {m_title = title;}
	int labor_id() {return m_labor_id;}
	void set_labor_id(int labor_id) {m_labor_id = labor_id;}
	int skill_id() {return m_skill_id;}
	void set_skill_id(int labor_id) {m_labor_id = labor_id;}
	bool override_color() {return m_override_set_colors;}
	void set_override_color(bool yesno) {m_override_set_colors = yesno;}
	ViewColumnSet *set() {return m_set;}

	QStandardItem *build_cell(Dwarf *d); // create a suitable item based on a dwarf

protected:
	QString m_title;
	QColor m_bg_color;
	bool m_override_set_colors;
	ViewColumnSet *m_set;
	int m_labor_id;
	int m_skill_id;
};

class LaborColumn : public ViewColumn {
public:
	LaborColumn(QString title, int labor_id, int skill_id, ViewColumnSet *set = 0, QObject *parent = 0);
};


/*!
ViewColumnSet
*/
class ViewColumnSet : public QObject {
	Q_OBJECT
public:
	ViewColumnSet(QString name, QObject *parent = 0);

	QString name() {return m_name;}
	void add_column(ViewColumn *col) {
		m_columns.insert(m_columns.uniqueKeys().size(), col);
	}
	void clear_columns() {
		foreach(ViewColumn *col, m_columns) {
			col->deleteLater();
		}
		m_columns.clear();
	}
	void set_bg_color(const QColor &color) {
		m_bg_color = color;
	}
	QColor bg_color() {return m_bg_color;}

	QList<ViewColumn*> columns() {return m_columns.values();}

	GridView *view() {return m_view;}

private:
	QString m_name;
	GridView *m_view;
	QMap<int, ViewColumn*> m_columns;
	QBrush m_bg_brush; // possibly allow textured backgrounds in the long long ago, err future.
	QColor m_bg_color;
};

/*!
The idea: GridViews have many ViewColumnSets, which in turn have many 
ViewColumns. 

The ViewColumn has a data accessor to get at the underlying data for
a given row. 

ViewColumns make up a ViewColumnSet, which has a background color and
maintains the order of its member columns.

ViewColumnSets make up a GridView, which can be named and saved for 
use in other forts. This should allow maximum configurability for how
users may want their labor columns grouped, named, and colored.
*/

/*!
GridView
*/
class GridView : public QObject {
	Q_OBJECT
public:
	GridView(QString name, QObject *parent = 0);

	QString name() {
		return m_name;
	}
	void add_set(ViewColumnSet *set) {
		m_sets.insert(set->name(), set);
	}
	void remove_set(QString name) {
		m_sets.remove(name);
	};
	void clear() {
		foreach(ViewColumnSet *set, m_sets) {
			set->deleteLater();
		}
		m_sets.clear();
	}

	QList<ViewColumnSet*> sets() {return m_sets.values();}

private:
	QString m_name;
	QMap<QString, ViewColumnSet*> m_sets;
};

#endif