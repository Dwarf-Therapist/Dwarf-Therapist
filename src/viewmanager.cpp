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
#include "viewcolumn.h"
#include "utils.h"

ViewManager::ViewManager(DwarfModel *dm, DwarfModelProxy *proxy, QWidget *parent)
	: QTabWidget(parent)
	, m_model(dm)
	, m_proxy(proxy)
	, m_add_tab_button(new QToolButton(this))
{
	m_proxy->setSourceModel(m_model);
	setTabsClosable(true);
	setMovable(true);

	reload_views();

	m_add_tab_button->setIcon(QIcon(":img/tab_add.png"));
	m_add_tab_button->setPopupMode(QToolButton::InstantPopup);
	m_add_tab_button->setToolButtonStyle(Qt::ToolButtonIconOnly);
	draw_add_tab_button();
	setCornerWidget(m_add_tab_button, Qt::TopLeftCorner);

	connect(tabBar(), SIGNAL(tabMoved(int, int)), this, SLOT(write_views()));
	connect(tabBar(), SIGNAL(currentChanged(int)), this, SLOT(setCurrentIndex(int)));
	connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(remove_tab_for_gridview(int)));
	connect(m_model, SIGNAL(need_redraw()), this, SLOT(redraw_current_tab()));

	draw_views();
}

void ViewManager::draw_add_tab_button() {
	QIcon icn(":img/tab_add.png");
	QMenu *m = new QMenu(this);
	foreach(GridView *v, m_views) {
		QAction *a = m->addAction(icn, "Add " + v->name(), this, SLOT(add_tab_from_action()));
		a->setData((qulonglong)v);
	}
	m_add_tab_button->setMenu(m);
}

void ViewManager::reload_views() {
	// start reading views from main settings
	QSettings *s = DT->user_settings();

    // first make sure the user has some configured views (they won't on first run)
	int total_views = s->beginReadArray("gridviews");
	s->endArray();
    if (!s->childGroups().contains("gridviews") || total_views == 0) {
        QTemporaryFile temp_f;
        if (temp_f.open()) {
            QString filename = temp_f.fileName();
            QResource res(":config/default_gridviews");
			if (res.isCompressed()) {
				temp_f.write(qUncompress(res.data(), res.size()));
			} else {
				temp_f.write(QByteArray((const char*)res.data(), res.size()));
			}
            temp_f.flush();
        }
        s = new QSettings(temp_f.fileName(), QSettings::IniFormat);
    }
    m_views.clear();
	total_views = s->beginReadArray("gridviews");
    for (int i = 0; i < total_views; ++i) {
	    s->setArrayIndex(i);
	    m_views << GridView::read_from_ini(*s, this);
    }
    s->endArray();
	
	LOGI << "Loaded" << m_views.size() << "views from disk";
	draw_add_tab_button();
}

void ViewManager::draw_views() {
	// see if we have a saved tab order...
	QTime start = QTime::currentTime();
	int idx = currentIndex();
	while (count()) {
		QWidget *w = widget(0);
		w->deleteLater();
		removeTab(0);
	}
	QStringList tab_order = DT->user_settings()->value("gui_options/tab_order").toStringList();
	if (tab_order.size() == 0) {
		tab_order << "Labors" << "Military" << "Social";
	}
	if (tab_order.size() > 0) {
		foreach(QString name, tab_order) {
			foreach(GridView *v, m_views) {
				if (v->name() == name) 
					add_tab_for_gridview(v);
			}
		}
	}
	if (idx >= 0 && idx <= count() - 1) {
		setCurrentIndex(idx);
	} else {
		setCurrentIndex(0);
	}
	QTime stop = QTime::currentTime();
	LOGD << QString("redrew views in %L1ms").arg(start.msecsTo(stop));
}

void ViewManager::write_tab_order() {
	QStringList tab_order;
	for (int i = 0; i < count(); ++i) {
		tab_order << tabText(i);
	}
	DT->user_settings()->setValue("gui_options/tab_order", tab_order);
}

void ViewManager::write_views() {
	QSettings *s = DT->user_settings();
	s->remove("gridviews"); // look at us, taking chances like this!
	s->beginWriteArray("gridviews", m_views.size());
	int i = 0;
	foreach(GridView *gv, m_views) {
		s->setArrayIndex(i++);
		gv->write_to_ini(*s);
	}
	s->endArray();
	
	write_tab_order();
}

void ViewManager::add_view(GridView *view) {
    view->re_parent(this);
    m_views << view;
	write_views();
	draw_add_tab_button();
	add_tab_for_gridview(view);
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

GridView *ViewManager::get_active_view() {
	GridView *retval = 0;
	foreach(GridView *view, m_views) {
		if (view->name() == tabText(currentIndex())) {
			retval = view;
			break;
		}
	}
	return retval;
}

void ViewManager::remove_view(GridView *view) {
	m_views.removeAll(view);
	for (int i = 0; i < count(); ++i) {
		if (tabText(i) == view->name())
			removeTab(i);
	}
	write_views();
    draw_add_tab_button();
	view->deleteLater();
}

void ViewManager::replace_view(GridView *old_view, GridView *new_view) {
	bool update_current_index = false; // if the current tab was updated, we need to redraw it
	for (int i = 0; i < count(); ++i) {
		if (tabText(i) == old_view->name()) {
			setTabText(i, new_view->name()); // update tab titles that were showing the old view
            if (i == currentIndex()) // going to have to redraw the active tab as its view was just updated
			    update_current_index = true;
		}
	}
	m_views.removeAll(old_view);
	m_views.append(new_view);
	write_views();
    draw_add_tab_button();

	if (update_current_index) {
        setCurrentIndex(currentIndex());
	}
}

StateTableView *ViewManager::get_stv(int idx) {
    if (idx == -1)
        idx = currentIndex();
    QWidget *w = widget(idx);
    if (w)
        return static_cast<StateTableView*>(w);
    return 0;
}

void ViewManager::setCurrentIndex(int idx) {
	if (idx < 0 || idx > count()-1) {
		//LOGW << "tab switch to index" << idx << "requested but there are only" << count() << "tabs";
		return;
	}
	StateTableView *stv = get_stv(idx);
	foreach(GridView *v, m_views) {
		if (v->name() == tabText(idx)) {
			m_model->set_grid_view(v);
			m_model->build_rows();
			stv->header()->setResizeMode(QHeaderView::Fixed);
			stv->header()->setResizeMode(0, QHeaderView::ResizeToContents);
            //stv->header()->setResizeMode(m_model->columnCount() - 1, QHeaderView::ResizeToContents);
			stv->sortByColumn(0, Qt::AscendingOrder);
            QList<Dwarf*> tmp_list;
            foreach(Dwarf *d, m_selected_dwarfs) {
                tmp_list << d;
            }
            foreach(Dwarf *d, tmp_list) {
                stv->select_dwarf(d);
            }
			break;
		}
	}
	tabBar()->setCurrentIndex(idx);
	stv->restore_expanded_items();	
	write_tab_order();
}

void ViewManager::dwarf_selection_changed(const QItemSelection &selected, const QItemSelection &deselected) {
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
    QItemSelectionModel *selection = qobject_cast<QItemSelectionModel*>(QObject::sender());
    m_selected_dwarfs.clear();
    foreach(QModelIndex idx, selection->selectedRows(0)) {
        int dwarf_id = idx.data(DwarfModel::DR_ID).toInt();
        Dwarf *d = DT->get_dwarf_by_id(dwarf_id);
        if (d)
            m_selected_dwarfs << d;
    }
}

int ViewManager::add_tab_from_action() {
	QAction *a = qobject_cast<QAction*>(QObject::sender());
	if (!a)
		return -1;
	
	GridView *v = (GridView*)(a->data().toULongLong());
	int idx = add_tab_for_gridview(v);
	setCurrentIndex(idx);
	return idx;
}

int ViewManager::add_tab_for_gridview(GridView *v) {
	v->set_active(true);
	StateTableView *stv = new StateTableView(this);
	stv->setSortingEnabled(false);
	stv->set_model(m_model, m_proxy);
	stv->setSortingEnabled(true);
    connect(stv, SIGNAL(dwarf_focus_changed(Dwarf*)), SIGNAL(dwarf_focus_changed(Dwarf*))); // pass-thru
    connect(stv->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
        SLOT(dwarf_selection_changed(const QItemSelection &, const QItemSelection &)));
    int new_idx = addTab(stv, v->name());
	write_tab_order();
	return new_idx;
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
	write_tab_order();
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

void ViewManager::set_group_by(int group_by) {
	m_model->set_group_by(group_by);
	redraw_current_tab();
}

void ViewManager::redraw_current_tab() {
	setCurrentIndex(currentIndex());
}
