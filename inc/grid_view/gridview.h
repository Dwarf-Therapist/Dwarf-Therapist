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

#ifndef GRID_VIEW_H
#define GRID_VIEW_H

#include <QtGui>

class ViewColumnSet;
class ViewManager;

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

	QString name() {return m_name;}
	void set_name(const QString &name) {m_name = name;}
	void set_filename(const QString &filename) {m_filename = filename;}
	void add_set(ViewColumnSet *set);
	void remove_set(QString name);
	void clear();
	QList<ViewColumnSet*> sets() {return m_sets;}
	bool is_active() {return m_active;}
	void set_active(bool active) {m_active = active;}

	static GridView *from_file(const QString &filepath, ViewManager *mgr, QObject *parent = 0);
	public slots:
		void write_settings();

private:
	bool m_active;
	QString m_name;
	QString m_filename;
	QList<ViewColumnSet*> m_sets;
	QMap<QString, ViewColumnSet*> m_set_map;
};

#endif
