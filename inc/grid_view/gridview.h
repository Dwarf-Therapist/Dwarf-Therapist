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
class GridViewDialog;

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
	GridView(const GridView &to_be_copied); // copy ctor

    void re_parent(QObject *parent);

	const QString name() const {return m_name;}
	void set_name(const QString &name) {m_name = name;}
	void add_set(ViewColumnSet *set);
	void remove_set(QString name);
	void remove_set(ViewColumnSet *set) {m_sets.removeAll(set);}
	void clear();
	const QList<ViewColumnSet*> sets() const {return m_sets;}
	bool is_active() {return m_active;}
	void set_active(bool active) {m_active = active;}
	ViewColumnSet *get_set(const QString &name);
	ViewColumnSet *get_set(int offset) {return m_sets.at(offset);}
	
	void write_to_ini(QSettings &settings);

	//! Factory function to create a gridview from a QSettings that has already been pointed at a gridview entry
	static GridView *read_from_ini(QSettings &settings, QObject *parent = 0);

private:
	bool m_active;
	QString m_name;
	QList<ViewColumnSet*> m_sets;

signals:
    void updated(const GridView*);
};

#endif
