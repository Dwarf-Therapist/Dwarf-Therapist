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
{
	m_proxy->setSourceModel(m_model);
	setTabsClosable(true);
	setMovable(true);

	reload_sets();
	reload_views();

	QIcon icn(":img/tab_add.png");
	QMenu *m = new QMenu(this);
	foreach(GridView *v, m_views) {
		QAction *a = m->addAction(icn, "Add " + v->name(), this, SLOT(add_tab_from_action()));
		a->setData(v->name());
	}

	QToolButton *btn = new QToolButton(this);
	btn->setIcon(QIcon(":img/tab_add.png"));
	btn->setPopupMode(QToolButton::InstantPopup);
	btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
	btn->setMenu(m);
	setCornerWidget(btn, Qt::TopLeftCorner);

	connect(tabBar(), SIGNAL(tabMoved(int, int)), this, SLOT(write_views()));
	connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(remove_tab_for_gridview(int)));
}

void ViewManager::reload_views() {
	// make sure the required directories are in place
	// TODO make directory locations configurable
	QDir cur = QDir::current();
	if (!cur.exists("etc/views")) {
		QMessageBox::warning(this, tr("Missing Directory"),
			tr("Could not fine the 'views' directory under 'etc'"));
		return;
	}
	if (!cur.exists("etc/sets")) {
		QMessageBox::warning(this, tr("Missing Directory"),
			tr("Could not fine the 'sets' directory under 'etc'"));
		return;
	}

	// goodbye old views!
	foreach(GridView *v, m_views) {
		v->deleteLater();
	}
	m_views.clear();

	QDir views = QDir(QDir::currentPath() + "/etc/views");
	QDir sets = QDir(QDir::currentPath() + "/etc/sets");
	
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

	// see if we have a saved tab order...
	QStringList tab_order = DT->user_settings()->value("gui_options/tab_order").toStringList();
	if (tab_order.size() > 0) {
		foreach(QString name, tab_order) {
			foreach(GridView *v, m_views) {
				if (v->name() == name) {
					add_tab_for_gridview(v);
				}
			}
		}
	} else {
		// load them up in default order
		foreach(GridView *v, m_views) {
			if (v->is_active())
				add_tab_for_gridview(v);
		}
	}
	connect(tabBar(), SIGNAL(currentChanged(int)), this, SLOT(setCurrentIndex(int)));
	//connect(cornerWidget(), SIGNAL(pressed()), m_views[0]->sets()[4], SLOT(show_builder_dialog()));
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
	if (!cur.exists("etc/sets")) {
		QMessageBox::warning(this, tr("Missing Directory"),
			tr("Could not fine the 'sets' directory under 'etc'"));
		return;
	}
	foreach(ViewColumnSet *set, m_sets) {
		set->deleteLater();
	}
	m_sets.clear();
	QDir sets = QDir(QDir::currentPath() + "/etc/sets");
	QStringList set_files = sets.entryList(QDir::Files | QDir::Readable, QDir::Name);
	foreach(QString filename, set_files) {
		if (filename.endsWith(".ini")) {
			TRACE << "found set file" << sets.filePath(filename);
			ViewColumnSet *set = ViewColumnSet::from_file(sets.filePath(filename), this);
			if (set)
				m_sets << set;
		}
	}
	LOGI << "Loaded" << m_sets.size() << "column sets from disk";
}

ViewColumnSet *ViewManager::get_set_by_name(const QString &name) {
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
		if (v->name() == tabBar()->tabText(idx)) {
			m_model->set_grid_view(v);
			m_model->build_rows();
			stv->header()->setResizeMode(QHeaderView::Fixed);
			stv->header()->setResizeMode(0, QHeaderView::ResizeToContents);
			stv->sortByColumn(0, Qt::AscendingOrder);
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
	stv->set_model(m_model, m_proxy);
	m_model->set_grid_view(v);
	m_model->build_rows();
	stv->header()->setResizeMode(QHeaderView::Fixed);
	stv->header()->setResizeMode(0, QHeaderView::ResizeToContents);
	stv->sortByColumn(0, Qt::AscendingOrder);
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