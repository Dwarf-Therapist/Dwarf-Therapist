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
#ifndef DWARF_MODEL_H
#define DWARF_MODEL_H

#include <QtGui>
class Dwarf;
class DFInstance;
class DwarfModel;
class GridView;

class DwarfModel : public QStandardItemModel {
	Q_OBJECT
public:
	typedef enum {
		GB_NOTHING = 0,
		GB_PROFESSION,
		GB_LEGENDARY,
		GB_SEX,
		GB_HAPPINESS,
		GB_TOTAL
	} GROUP_BY;
	typedef enum {
		DR_RATING = Qt::UserRole + 1,
		DR_SORT_VALUE,
		DR_IS_AGGREGATE,
		DR_LABOR_ID,
		DR_GROUP_NAME,
		DR_ID,
		DR_DEFAULT_BG_COLOR,
		DR_DUMMY, // used as an int counter that increments to force re-draws
		DR_COL_TYPE,
		DR_SET_NAME
	} DATA_ROLES;

	DwarfModel(QObject *parent = 0);
	virtual ~DwarfModel();
	void set_instance(DFInstance *df) {m_df = df;}
	void set_grid_view(GridView *v) {m_gridview = v;}
	void clear_all(); // reset everything to normal

	GROUP_BY current_grouping() const {return m_group_by;}
	const QMap<QString, QVector<Dwarf*> > *get_dwarf_groups() const {return &m_grouped_dwarves;}
	Dwarf *get_dwarf_by_id(int id) const {return m_dwarves.value(id, 0);}
		
	QVector<Dwarf*> get_dirty_dwarves();
	QList<Dwarf*> get_dwarves() {return m_dwarves.values();}
	void calculate_pending();
	int selected_col() const {return m_selected_col;}
	void filter_changed(const QString &);

	public slots:
		void build_row(const QString &key);
		void build_rows();
		void set_group_by(int group_by);
		void load_dwarves();
		void cell_activated(const QModelIndex &idx); // a grid cell was clicked/doubleclicked or enter was pressed on it
		void clear_pending();
		void commit_pending();
		void section_right_clicked(int idx);

private:
	DFInstance *m_df;
	QMap<int, Dwarf*> m_dwarves;
	QMap<QString, QVector<Dwarf*> > m_grouped_dwarves;
	GROUP_BY m_group_by;
	int m_selected_col;
	GridView *m_gridview;
	
signals:
	void new_pending_changes(int);
	void preferred_header_size(int section, int width);
	void set_index_as_spacer(int);
	void clear_spacers();
	void need_redraw();
};
#endif
