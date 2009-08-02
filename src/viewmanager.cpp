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

ViewManager::ViewManager(DwarfModel *dm, DwarfModelProxy *proxy, QWidget *parent)
	: QTabWidget(parent)
	, m_model(dm)
	, m_proxy(proxy)
{
	m_proxy->setSourceModel(m_model);
 
	QPushButton *btn = new QPushButton("Add View", this);
	setCornerWidget(btn);
	
	reload_views();
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
			LOGD << "found view file" << views.filePath(filename);
			GridView *v = GridView::from_file(views.filePath(filename), sets, this);
			if (v)
				m_views << v;
			if (v->is_active())
				add_tab_for_gridview(v);
		}
	}
	connect(tabBar(), SIGNAL(currentChanged(int)), this, SLOT(setCurrentIndex(int)));
	m_model->set_grid_view(m_views[0]);
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
}

int ViewManager::add_tab_for_gridview(GridView *v) {
	StateTableView *stv = new StateTableView(this);
	stv->set_model(m_model, m_proxy);
	m_model->set_grid_view(v);
	m_model->build_rows();
	stv->header()->setResizeMode(QHeaderView::Fixed);
	stv->header()->setResizeMode(0, QHeaderView::ResizeToContents);
	stv->sortByColumn(0, Qt::AscendingOrder);
	return addTab(stv, v->name());
}

void ViewManager::expand_all() {
	StateTableView *stv = qobject_cast<StateTableView*>(currentWidget());
	stv->expandAll();
}

void ViewManager::collapse_all() {
	StateTableView *stv = qobject_cast<StateTableView*>(currentWidget());
	stv->collapseAll();
}
