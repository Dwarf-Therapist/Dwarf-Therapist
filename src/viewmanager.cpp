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
#include "skillcolumn.h"
#include "laborcolumn.h"
#include "happinesscolumn.h"
#include "spacercolumn.h"
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

void ViewManager::views_changed() {
	reload_views();
	draw_views();
}

void ViewManager::reload_views() {
	// goodbye old views!
	foreach(GridView *v, m_views) {
		v->deleteLater();
	}
	m_views.clear();

	// start reading views from main settings
	QSettings *s = DT->user_settings();
	int total_views = s->beginReadArray("gridviews");
	for (int i = 0; i < total_views; ++i) {
		s->setArrayIndex(i);
		QString name = s->value("name", "UNKNOWN").toString();
		bool active = s->value("active", true).toBool();

		GridView *gv = new GridView(name, this);
		gv->set_active(active);
		add_view(gv);

		int total_sets = s->beginReadArray("sets");
		for (int j = 0; j < total_sets; ++j) {
			s->setArrayIndex(j);
			QString name = s->value("name", "UNKNOWN").toString();
			QString color_in_hex = s->value("bg_color", "0xFFFFFF").toString();
			QColor bg_color = from_hex(color_in_hex);

			ViewColumnSet *set = new ViewColumnSet(name, this);
			set->set_bg_color(bg_color);
			gv->add_set(set);

			int total_columns = s->beginReadArray("columns");
			for (int k = 0; k < total_columns; ++k) {
				s->setArrayIndex(k);
				QString tmp_type = s->value("type", "DEFAULT").toString();
				QString col_name = s->value("name", "UNKNOWN " + QString::number(k)).toString();
				COLUMN_TYPE type = get_column_type(tmp_type);
				QString hex_color = s->value("bg_color", color_in_hex).toString();
				QColor bg_color = from_hex(hex_color);

				ViewColumn *vc = 0;
				switch (type) {
					case CT_SKILL:
						{
							int skill_id = s->value("skill_id", -1).toInt();
							//TODO: check that labor and skill are known ids
							SkillColumn *c = new SkillColumn(col_name, skill_id, set, set);
							vc = c;
						}
						break;
					case CT_LABOR:
						{
							int labor_id = s->value("labor_id", -1).toInt();
							int skill_id = s->value("skill_id", -1).toInt();
							//TODO: check that labor and skill are known ids
							LaborColumn *c = new LaborColumn(col_name, labor_id, skill_id, set, set);
							vc = c;
						}
						break;
					case CT_HAPPINESS:
						{
							HappinessColumn *c = new HappinessColumn(col_name, set, set);
							vc = c;
						}
						break;
					case CT_SPACER:
						{
							int width = s->value("width", DEFAULT_SPACER_WIDTH).toInt();
							SpacerColumn *c = new SpacerColumn(col_name, set, set);
							c->set_width(width);
							vc = c;
						}
						break;
					default:
						LOGW << "Column " << col_name << "in set" << set->name() << "has unknown type: " << tmp_type;
						break;
				}
				if (vc) {
					vc->set_override_color(s->value("override_color").toBool());
					if (vc->override_color()) {
						vc->set_bg_color(bg_color);
					}
				}
			}
			s->endArray();
		}
		s->endArray();
	}
	s->endArray();
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
	}
	if (idx >= 0 && idx <= count() - 1) {
		setCurrentIndex(idx);
	} else {
		setCurrentIndex(0);
	}
}

void ViewManager::write_views() {
	QSettings *s = DT->user_settings();
	s->remove("gridviews"); // look at us, taking chances like this!
	s->beginWriteArray("gridviews", m_views.size());
	int i = 0;
	foreach(GridView *gv, m_views) {
		s->setArrayIndex(i++);
		s->setValue("name", gv->name());
		s->setValue("active", gv->is_active());
		s->beginWriteArray("sets", gv->sets().size());
		int j = 0;
		foreach(ViewColumnSet *set, gv->sets()) {
			s->setArrayIndex(j++);
			s->setValue("name", set->name());
			s->setValue("bg_color", to_hex(set->bg_color()));
			s->beginWriteArray("columns", set->columns().size());
			int k = 0;
			foreach(ViewColumn *vc, set->columns()) {
				s->setArrayIndex(k++);
				if (!vc->title().isEmpty())
					s->setValue("name", vc->title());
				else
					s->setValue("name", "UNKNOWN");

				s->setValue("type", get_column_type(vc->type()));
				if (vc->override_color()) {
					s->setValue("override_color", true);
					s->setValue("bg_color", to_hex(vc->bg_color()));
				}
				switch (vc->type()) {
					case CT_SKILL:
						{
							SkillColumn *c = static_cast<SkillColumn*>(vc);
							s->setValue("skill_id", c->skill_id());
						}
						break;
					case CT_LABOR:
						{
							LaborColumn *c = static_cast<LaborColumn*>(vc);
							s->setValue("labor_id", c->labor_id());
							s->setValue("skill_id", c->skill_id());
						}
						break;
					case CT_SPACER:
						{
							SpacerColumn *c = static_cast<SpacerColumn*>(vc);
							if (c->width() > 0)
								s->setValue("width", c->width());
						}
						break;
					default:
						break;
				}
			}
			s->endArray();
		}
		s->endArray();
	}
	s->endArray();
	
	QStringList tab_order;
	for (int i = 0; i < count(); ++i) {
		tab_order << tabText(i);
	}
	DT->user_settings()->setValue("gui_options/tab_order", tab_order);
}

void ViewManager::add_view(GridView *view) {
    view->re_parent(this);
    m_views << view;
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
	write_views();
	views_changed();
}

void ViewManager::replace_view(GridView *old_view, GridView *new_view) {
	m_views.removeAll(old_view);
	m_views.append(new_view);
	write_views();
	views_changed();
}

void ViewManager::setCurrentIndex(int idx) {
	if (idx < 0 || idx > count()-1) {
		LOGW << "tab switch to index" << idx << "requested but there are only" << count() << "tabs";
		return;
	}
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
	stv->restore_expanded_items();	
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
	connect(stv, SIGNAL(dwarf_focus_changed(Dwarf*)), SIGNAL(dwarf_focus_changed(Dwarf*))); // pass-thru
	stv->setSortingEnabled(false);
	stv->set_model(m_model, m_proxy);
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

void ViewManager::set_group_by(int group_by) {
	m_model->set_group_by(group_by);
	redraw_current_tab();
}

void ViewManager::redraw_current_tab() {
	setCurrentIndex(currentIndex());
}