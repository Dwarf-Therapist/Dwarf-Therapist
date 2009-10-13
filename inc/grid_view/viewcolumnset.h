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
#ifndef VIEW_COLUMN_SET_H
#define VIEW_COLUMN_SET_H

#include <QtGui>

class ViewManager;
class ViewColumn;
class ViewColumnSetDialog;
class GridView;
class Dwarf;

/*!
ViewColumnSet
*/
class ViewColumnSet : public QObject {
	Q_OBJECT
public:
	ViewColumnSet(QString name, QObject *parent = 0);
	ViewColumnSet(const ViewColumnSet &copy); //copy ctor

    void re_parent(QObject *parent);

	QString name() {return m_name;}
	void add_column(ViewColumn *col);
	void clear_columns();
	void set_bg_color(const QColor &color) {m_bg_color = color;}
	QColor bg_color() {return m_bg_color;}
	QList<ViewColumn*> columns() {return m_columns;}
	GridView *view() {return m_view;}
	void remove_column(int offset) {m_columns.removeAt(offset);}
	void remove_column(ViewColumn *vc) {m_columns.removeAll(vc);}
	ViewColumn *column_at(int offset) {return m_columns.at(offset);}

	//! order of columns was changed by a view, so reflect those changes internally
	void reorder_columns(const QStandardItemModel &model);

	//! persist this structure to disk
	void write_to_ini(QSettings &s);
	//! factory method for creating a set based on a QSettings that has been pointed at a set entry
	static ViewColumnSet *read_from_ini(QSettings &s, QObject *parent = 0);

	public slots:
		void set_name(const QString &name);
		void toggle_for_dwarf(Dwarf *d);
		void toggle_for_dwarf(); // from context menu of single labor
		void toggle_for_dwarf_group(); // from context menu of aggregate

private:
	QString m_name;
	GridView *m_view;
	QList<ViewColumn*> m_columns;
	QBrush m_bg_brush; // possibly allow textured backgrounds in the long long ago, err future.
	QColor m_bg_color;
};

#endif
