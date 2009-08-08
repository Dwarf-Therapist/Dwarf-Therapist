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
#include "viewmanager.h"
#include "statetableview.h"
#include "dwarfmodel.h"
#include "dwarfmodelproxy.h"
#include "gridview.h"
#include "defines.h"
#include "dwarftherapist.h"
#include "viewcolumnset.h"

ViewManager::ViewManager(DwarfModel *dm, DwarfModelProxy *proxy, QWidget *parent)
	: QTabWidget(parent)
	, m_model(dm)
	, m_proxy(proxy)
	, m_add_tab_button(new QToolButton(this))
{
	m_proxy->setSourceModel(m_model);
	setTabsClosable(true);
	setMovable(true);

	read_settings();

	reload_sets();
	reload_views();

	m_add_tab_button->setIcon(QIcon(":img/tab_add.png"));
	m_add_tab_button->setPopupMode(QToolButton::InstantPopup);
	m_add_tab_button->setToolButtonStyle(Qt::ToolButtonIconOnly);
	draw_add_tab_button();
	setCornerWidget(m_add_tab_button, Qt::TopLeftCorner);

	connect(tabBar(), SIGNAL(tabMoved(int, int)), this, SLOT(write_views()));
	connect(tabBar(), SIGNAL(currentChanged(int)), this, SLOT(setCurrentIndex(int)));
	connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(remove_tab_for_gridview(int)));
}

void ViewManager::read_settings() {
	QSettings *s = DT->user_settings();
	m_set_path = s->value("options/paths/sets", "etc/sets").toString();
	m_view_path = s->value("options/paths/views", "etc/views").toString();
}

void ViewManager::draw_add_tab_button() {
	QIcon icn(":img/tab_add.png");
	QMenu *m = new QMenu(this);
	foreach(GridView *v, m_views) {
		QAction *a = m->addAction(icn, "Add " + v->name(), this, SLOT(add_tab_from_action()));
		a->setData(v->name());
	}
	m_add_tab_button->setMenu(m);
}

void ViewManager::sets_changed() {
	read_settings();
	reload_sets();
	reload_views();
	setCurrentIndex(currentIndex());
}

void ViewManager::views_changed() {
	read_settings();
	reload_sets();
	reload_views();
	draw_views();
}

void ViewManager::reload_views() {
	// make sure the required directories are in place
	// TODO make directory locations configurable
	QDir cur = QDir::current();
	if (!cur.exists(m_view_path)) {
		QMessageBox::warning(this, tr("Missing Views Directory"),
			tr("Could not find the 'views' directory at '%1'").arg(m_view_path));
		return;
	}
	// goodbye old views!
	foreach(GridView *v, m_views) {
		v->deleteLater();
	}
	m_views.clear();

	QDir views = QDir(m_view_path).absolutePath();
	QStringList view_files = views.entryList(QDir::Files | QDir::Readable, QDir::Time);
	foreach(QString filename, view_files) {
		if (filename.endsWith(".ini")) {
			TRACE << "found view file" << views.filePath(filename);
			GridView *v = GridView::from_file(views.filePath(filename), this, this);
			if (v)
				m_views << v;
		}
	}
	LOGI << "Loaded" << m_views.size() << "views from disk";
	draw_add_tab_button();
	draw_views();
}

void ViewManager::draw_views() {
	// see if we have a saved tab order...
	int idx = currentIndex();
	while (count()) {
		QWidget *w = widget(0);
		w->deleteLater();
		removeTab(0);
	}
	int x = count();
	QStringList tab_order = DT->user_settings()->value("gui_options/tab_order").toStringList();
	if (tab_order.size() == 0) {
		tab_order << "Labors" << "VPView" << "Military" << "Social";
	}
	if (tab_order.size() > 0) {
		foreach(QString name, tab_order) {
			foreach(GridView *v, m_views) {
				if (v->name() == name) 
					add_tab_for_gridview(v);
			}
		}
	} else {
		// load them up in default order
		foreach(GridView *v, m_views) {
			if (v->is_active())
				add_tab_for_gridview(v);
		}
	}
	if (idx >= 0 && idx <= count() - 1) {
		setCurrentIndex(idx);
	} else {
		setCurrentIndex(0);
	}
}

void ViewManager::write_views() {
	foreach(GridView *v, m_views) {
		v->write_settings();
	}
	QStringList tab_order;
	for (int i = 0; i < count(); ++i) {
		tab_order << tabText(i);
	}
	DT->user_settings()->setValue("gui_options/tab_order", tab_order);
}

void ViewManager::reload_sets() {
	QDir cur = QDir::current();
	if (!cur.exists(m_set_path)) {
		QMessageBox::warning(this, tr("Missing Sets Directory"),
			tr("Could not fine the 'sets' directory at '%1'").arg(m_set_path));
		return;
	}
	foreach(ViewColumnSet *set, m_sets) {
		set->deleteLater();
	}
	m_sets.clear();
	QDir sets = QDir(m_set_path).absolutePath();
	QStringList set_files = sets.entryList(QDir::Files | QDir::Readable, QDir::Name);
	foreach(QString filename, set_files) {
		if (filename.endsWith(".ini")) {
			TRACE << "found set file" << sets.filePath(filename);
			ViewColumnSet *set = ViewColumnSet::from_file(sets.filePath(filename), this, this);
			if (set)
				m_sets << set;
		}
	}
	LOGI << "Loaded" << m_sets.size() << "column sets from disk";
}

GridView *ViewManager::get_view(const QString &name) {
	GridView *retval = 0;
	foreach(GridView *view, m_views) {
		if (name == view->name()) {
			retval = view;
			break;
		}
	}
	return retval;
}

ViewColumnSet *ViewManager::get_set(const QString &name) {
	ViewColumnSet *retval = 0;
	foreach(ViewColumnSet *set, m_sets) {
		if (name == set->name()) {
			retval = set;
			break;
		}
	}
	return retval;
}

void ViewManager::setCurrentIndex(int idx) {
	StateTableView *stv = qobject_cast<StateTableView*>(widget(idx));
	foreach(GridView *v, m_views) {
		if (v->name() == tabText(idx)) {
			m_model->set_grid_view(v);
			m_model->build_rows();
			stv->header()->setResizeMode(QHeaderView::Fixed);
			stv->header()->setResizeMode(0, QHeaderView::ResizeToContents);
			//stv->sortByColumn(0, Qt::AscendingOrder);
			break;
		}
	}
	tabBar()->setCurrentIndex(idx);
}

int ViewManager::add_tab_from_action() {
	QAction *a = qobject_cast<QAction*>(QObject::sender());
	if (!a)
		return -1;
	
	QString name = a->data().toString();
	foreach(GridView *v, m_views) {
		if (v->name() == name) {
			int idx = add_tab_for_gridview(v);
			setCurrentIndex(idx);
			return idx;
		}
	}
	return -1;
}

int ViewManager::add_tab_for_gridview(GridView *v) {
	v->set_active(true);
	StateTableView *stv = new StateTableView(this);
	stv->setSortingEnabled(false);
	stv->set_model(m_model, m_proxy);
	m_model->set_grid_view(v);
	m_model->build_rows();
	stv->header()->setResizeMode(QHeaderView::Fixed);
	stv->header()->setResizeMode(0, QHeaderView::ResizeToContents);
	//stv->sortByColumn(0, Qt::AscendingOrder);
	stv->setSortingEnabled(true);
	return addTab(stv, v->name());
}

void ViewManager::remove_tab_for_gridview(int idx) {
	if (count() < 2) {
		QMessageBox::warning(this, tr("Can't Remove Tab"), 
			tr("Cannot remove the last tab!"));
		return;
	}
	foreach(GridView *v, m_views) {
		if (v->name() == tabText(idx)) {
			// find out if there are other dupes of this view still active...
			int active = 0;
			for(int i = 0; i < count(); ++i) {
				if (tabText(i) == v->name()) {
					active++;
				}
			}
			if (active < 2)
				v->set_active(false);
		}
	}
	widget(idx)->deleteLater();
	removeTab(idx);
}

void ViewManager::expand_all() {
	StateTableView *stv = qobject_cast<StateTableView*>(currentWidget());
	stv->expandAll();
}

void ViewManager::collapse_all() {
	StateTableView *stv = qobject_cast<StateTableView*>(currentWidget());
	stv->collapseAll();
}

void ViewManager::jump_to_dwarf(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
	StateTableView *stv = qobject_cast<StateTableView*>(currentWidget());
	stv->jump_to_dwarf(current, previous);
}

void ViewManager::jump_to_profession(QListWidgetItem *current, QListWidgetItem *previous) {
	StateTableView *stv = qobject_cast<StateTableView*>(currentWidget());
	stv->jump_to_profession(current, previous);
}
