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

#ifndef VIEW_MANAGER_H
#define VIEW_MANAGER_H

#include <QtGui>

class GridView;
class ViewColumnSet;
class StateTableView;
class DwarfModel;
class DwarfModelProxy;

class ViewManager : public QTabWidget {
	Q_OBJECT
public:
	ViewManager(DwarfModel *dm, DwarfModelProxy *proxy, QWidget *parent = 0);
	
	QList<GridView*> views() {return m_views;}
	QList<ViewColumnSet*> sets() {return m_sets;}

	public slots:
		void setCurrentIndex(int);
		void reload_views();
		void write_views();

		void reload_sets();
		//void write_sets();
		//void write_set(ViewColumnSet *set);

		GridView *get_view(const QString &name);
		ViewColumnSet *get_set(const QString &name);

		// dialogs
		//void edit_view(QListWidgetItem*); // from main window's list of views
		void edit_set(QListWidgetItem*); // from main window's list of sets
		//void edit_set(); // from context menu
		

		// passthru
		void expand_all();
		void collapse_all();

private:
	QList<GridView*> m_views;
	QList<ViewColumnSet*> m_sets;
	DwarfModel *m_model;
	DwarfModelProxy *m_proxy;

	private slots:
		//! used when adding tabs via the tool button
		int add_tab_from_action();
		//! used from everywhere else
		int add_tab_for_gridview(GridView *v);
		void remove_tab_for_gridview(int index);
};

#endif
