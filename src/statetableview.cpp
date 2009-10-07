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
#include <QtGui>
#include "qmath.h"

#include "mainwindow.h"
#include "statetableview.h"
#include "dwarfmodel.h"
#include "dwarfmodelproxy.h"
#include "uberdelegate.h"
#include "rotatedheader.h"
#include "dwarf.h"
#include "defines.h"
#include "columntypes.h"
#include "gridview.h"
#include "viewcolumnset.h"
#include "laborcolumn.h"
#include "happinesscolumn.h"
#include "dwarftherapist.h"
#include "customprofession.h"
#include "viewmanager.h"

StateTableView::StateTableView(QWidget *parent)
	: QTreeView(parent)
	, m_model(0)
	, m_proxy(0)
	, m_delegate(new UberDelegate(this))
	, m_header(new RotatedHeader(Qt::Horizontal, this))
	, m_expanded_rows(QList<int>())
{
	read_settings();

	setMouseTracking(true);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setUniformRowHeights(true);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setIndentation(8);
	setFocusPolicy(Qt::NoFocus); // keep the dotted border off of things

	setItemDelegate(m_delegate);
	setHeader(m_header);

	// Set StaticContents to enable minimal repaints on resizes.
	viewport()->setAttribute(Qt::WA_StaticContents);

	connect(DT, SIGNAL(settings_changed()), this, SLOT(read_settings()));
	connect(this, SIGNAL(expanded(const QModelIndex &)), SLOT(index_expanded(const QModelIndex &)));
	connect(this, SIGNAL(collapsed(const QModelIndex &)), SLOT(index_collapsed(const QModelIndex &)));
    connect(this, SIGNAL(clicked(const QModelIndex &)), SLOT(clicked(const QModelIndex &)));
}

StateTableView::~StateTableView()
{}

void StateTableView::read_settings() {
	QSettings *s = DT->user_settings();
	
	//font
	QFont fnt = s->value("options/grid/font", QFont("Segoe UI", 8)).value<QFont>();
	setFont(fnt);
	
	//cell size
	m_grid_size = s->value("options/grid/cell_size", DEFAULT_CELL_SIZE).toInt();
	int pad = s->value("options/grid/cell_padding", 0).toInt();
	setIconSize(QSize(m_grid_size - 2 - pad * 2, m_grid_size - 2 - pad * 2));
	
	set_single_click_labor_changes(s->value("options/single_click_labor_changes", true).toBool());
	m_auto_expand_groups = s->value("options/auto_expand_groups", false).toBool();
}

void StateTableView::set_model(DwarfModel *model, DwarfModelProxy *proxy) {
	QTreeView::setModel(proxy);
	m_model = model;
	m_proxy = proxy;

	m_delegate->set_model(model);
	m_delegate->set_proxy(proxy);

	connect(m_header, SIGNAL(section_right_clicked(int)), m_model, SLOT(section_right_clicked(int)));
    connect(m_header, SIGNAL(sort_alpha_ascending()), m_proxy, SLOT(sort_alpha_ascending()));
    connect(m_header, SIGNAL(sort_alpha_descending()), m_proxy, SLOT(sort_alpha_descending()));
    connect(m_header, SIGNAL(sort_game_order()), m_proxy, SLOT(sort_game_order()));

	connect(this, SIGNAL(activated(const QModelIndex&)), proxy, SLOT(cell_activated(const QModelIndex&)));
	connect(m_model, SIGNAL(preferred_header_size(int, int)), m_header, SLOT(resizeSection(int, int)));
	connect(m_model, SIGNAL(set_index_as_spacer(int)), m_header, SLOT(set_index_as_spacer(int)));
	connect(m_model, SIGNAL(clear_spacers()), m_header, SLOT(clear_spacers()));
	set_single_click_labor_changes(DT->user_settings()->value("options/single_click_labor_changes", true).toBool());
}

void StateTableView::new_custom_profession() {
	QModelIndex idx = currentIndex();
	if (idx.isValid()) {
		int id = idx.data(DwarfModel::DR_ID).toInt();
		Dwarf *d = m_model->get_dwarf_by_id(id);
		if (d)
			emit new_custom_profession(d);
	}
}

void StateTableView::filter_dwarves(QString text) {
	m_proxy->setFilterFixedString(text);
	m_proxy->setFilterKeyColumn(0);
	m_proxy->setFilterRole(Qt::DisplayRole);
}

void StateTableView::jump_to_dwarf(QTreeWidgetItem* current, QTreeWidgetItem*) {
	if (!current)
		return;
	int dwarf_id = current->data(0, Qt::UserRole).toInt();
	
	Dwarf *d = m_model->get_dwarf_by_id(dwarf_id);
	if (d && d->m_name_idx.isValid()) {
		QModelIndex proxy_idx = m_proxy->mapFromSource(d->m_name_idx);
		if (proxy_idx.isValid()) {
			scrollTo(proxy_idx);
			selectionModel()->select(proxy_idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
		}
	}
}

void StateTableView::jump_to_profession(QListWidgetItem* current, QListWidgetItem*) {
	if (!current)
		return;
	QString prof_name = current->text();
	QModelIndexList matches = m_proxy->match(m_proxy->index(0,0), Qt::DisplayRole, prof_name);
	if (matches.size() > 0) {
		QModelIndex group_header = matches.at(0);
		scrollTo(group_header);
		expand(group_header);
		selectionModel()->select(group_header, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
	}
}

void StateTableView::contextMenuEvent(QContextMenuEvent *event) {
	QModelIndex idx = indexAt(event->pos());
	if (!idx.isValid())
		return;
	
	if (idx.column() == 0 && !idx.data(DwarfModel::DR_IS_AGGREGATE).toBool()) {
		// we're on top of a dwarf's name
		QMenu m(this); // this will be the popup menu
		int id = idx.data(DwarfModel::DR_ID).toInt();
		Dwarf *d = m_model->get_dwarf_by_id(id);
		m.addActions(d->get_actions());
		m.addSeparator();
		m.addAction(tr("Set Nickname..."), this, SLOT(set_nickname()));
		m.addSeparator();

		QMenu sub(&m);
		sub.setTitle(tr("Custom Professions"));
		QAction *a = sub.addAction(tr("New custom profession from this dwarf..."), this, SLOT(custom_profession_from_dwarf()));
		a->setData(id);
		sub.addAction(tr("Reset to default profession"), this, SLOT(reset_custom_profession()));
		sub.addSeparator();

		foreach(CustomProfession *cp, DT->get_custom_professions()) {
			sub.addAction(cp->get_name(), this, SLOT(apply_custom_profession()));
		}
		m.addMenu(&sub);
		m.exec(viewport()->mapToGlobal(event->pos()));
	} else if (idx.data(DwarfModel::DR_COL_TYPE).toInt() == CT_LABOR) {
		// labor column
		QMenu m(this); // this will be the popup menu
		QString set_name = idx.data(DwarfModel::DR_SET_NAME).toString();
		ViewColumnSet *set = DT->get_main_window()->get_view_manager()->get_active_view()->get_set(set_name);
		if (idx.data(DwarfModel::DR_IS_AGGREGATE).toBool()) { //aggregate labor
			QModelIndex first_col = idx.sibling(idx.row(), 0);
			QString group_name = idx.data(DwarfModel::DR_GROUP_NAME).toString();
			QAction *a = m.addAction(tr("Toggle %1 for %2").arg(set_name).arg(group_name));
			a->setData(group_name);
			connect(a, SIGNAL(triggered()), set, SLOT(toggle_for_dwarf_group()));
		} else { // single dwarf labor
			// find the dwarf...
			int dwarf_id = idx.data(DwarfModel::DR_ID).toInt();
			Dwarf *d = m_model->get_dwarf_by_id(dwarf_id);
			QAction *a = m.addAction(tr("Toggle %1 for %2").arg(set_name).arg(d->nice_name()));
			a->setData(dwarf_id);
			connect(a, SIGNAL(triggered()), set, SLOT(toggle_for_dwarf()));
		}
		m.exec(viewport()->mapToGlobal(event->pos()));
	}
}

void StateTableView::set_nickname() {
	const QItemSelection sel = selectionModel()->selection();
	QModelIndexList first_col;
	foreach(QModelIndex i, sel.indexes()) {
		if (i.column() == 0 && !i.data(DwarfModel::DR_IS_AGGREGATE).toBool())
			first_col << i;
	}

	if (first_col.size() != 1) {
		QMessageBox::warning(this, tr("Too many!"), tr("Slow down, killer. One at a time."));
		return;
	}
	
	int id = first_col[0].data(DwarfModel::DR_ID).toInt();
	Dwarf *d = m_model->get_dwarf_by_id(id);
	if (d) {
		QString new_nick = QInputDialog::getText(this, tr("New Nickname"), tr("Nickname"), QLineEdit::Normal, d->nickname());
		if (new_nick.length() > 28) {
			QMessageBox::warning(this, tr("Nickname too long"), tr("Nicknames must be under 28 characters long."));
			return;
		}
		d->set_nickname(new_nick);
		m_model->setData(first_col[0], d->nice_name(), Qt::DisplayRole);
	}
	m_model->calculate_pending();
}

void StateTableView::custom_profession_from_dwarf() {
	QAction *a = qobject_cast<QAction*>(QObject::sender());
	int id = a->data().toInt();
	Dwarf *d = m_model->get_dwarf_by_id(id);

	DT->custom_profession_from_dwarf(d);
}

void StateTableView::apply_custom_profession() {
	QAction *a = qobject_cast<QAction*>(QObject::sender());
	CustomProfession *cp = DT->get_custom_profession(a->text());
	if (!cp)
		return;

	const QItemSelection sel = selectionModel()->selection();
	foreach(const QModelIndex idx, sel.indexes()) {
		if (idx.column() == 0 && !idx.data(DwarfModel::DR_IS_AGGREGATE).toBool()) {
			Dwarf *d = m_model->get_dwarf_by_id(idx.data(DwarfModel::DR_ID).toInt());
			if (d)
				d->apply_custom_profession(cp);
		}
	}
	m_model->calculate_pending();
}

void StateTableView::reset_custom_profession() {
	const QItemSelection sel = selectionModel()->selection();
	foreach(const QModelIndex idx, sel.indexes()) {
		if (idx.column() == 0 && !idx.data(DwarfModel::DR_IS_AGGREGATE).toBool()) {
			Dwarf *d = m_model->get_dwarf_by_id(idx.data(DwarfModel::DR_ID).toInt());
			if (d)
				d->reset_custom_profession();
		}
	}
	m_model->calculate_pending();
}

void StateTableView::currentChanged(const QModelIndex &cur, const QModelIndex &) {
	// current item changed, so find out what dwarf the current item is for...
	if (!m_proxy) // special case tables (like skill legend, don't have a proxy)
		return;
	int id = m_proxy->data(cur, DwarfModel::DR_ID).toInt();
	Dwarf *d = m_model->get_dwarf_by_id(id);
	if (d) {
		//LOGD << "focus changed to" << d->nice_name();
		emit dwarf_focus_changed(d);
	}
}

void StateTableView::select_dwarf(Dwarf *d) {
    for(int top = 0; top < m_proxy->rowCount(); ++top) {
        QModelIndex idx = m_proxy->index(top, 0);
        if (idx.data(DwarfModel::DR_ID).toInt() == d->id()) {
            this->selectionModel()->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
            break;
        } else if (m_proxy->rowCount(idx)) { // has children
            for (int sub = 0; sub < m_proxy->rowCount(idx); ++sub) {
                QModelIndex sub_idx = m_proxy->index(sub, 0, idx);
                if (sub_idx.data(DwarfModel::DR_ID).toInt() == d->id()) {
                    this->selectionModel()->select(sub_idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
                }
            }
        }
    }
}
/************************************************************************/
/* Hey look, our own mouse handling (/rolleyes)                         */
/************************************************************************/
void StateTableView::mousePressEvent(QMouseEvent *event) {
    m_last_button = event->button();
    QTreeView::mousePressEvent(event);
}
void StateTableView::mouseMoveEvent(QMouseEvent *event) {
    QModelIndex idx = indexAt(event->pos());
    if (idx.isValid())
        m_header->column_hover(idx.column());
    QTreeView::mouseMoveEvent(event);
}

void StateTableView::mouseReleaseEvent(QMouseEvent *event) {
    m_last_button = event->button();
    QTreeView::mouseReleaseEvent(event);
}

void StateTableView::clicked(const QModelIndex &idx) {
    if (m_last_button == Qt::LeftButton && // only activate on left-clicks
        m_single_click_labor_changes && // only activated on single-click if the user set it that way
        m_proxy && // we only do this for views that have proxies (dwarf views)
        idx.column() != 0) // don't single-click activate the name column
        m_proxy->cell_activated(idx);
}

/************************************************************************/
/* Handlers for expand/collapse persistence                             */
/************************************************************************/
void StateTableView::expandAll() {
	m_expanded_rows.clear();
	for(int i = 0; i < m_proxy->rowCount(); ++i) {
		m_expanded_rows << i;
	}
	QTreeView::expandAll();
}

void StateTableView::collapseAll() {
	m_expanded_rows.clear();
	QTreeView::collapseAll();
}

void StateTableView::index_expanded(const QModelIndex &idx) {
	m_expanded_rows << idx.row();
}

void StateTableView::index_collapsed(const QModelIndex &idx) {
	int i = m_expanded_rows.indexOf(idx.row());
	if (i != -1)
		m_expanded_rows.removeAt(i);
}

void StateTableView::restore_expanded_items() {
	if (m_auto_expand_groups) {
		expandAll();
		return;
	}
	disconnect(this, SIGNAL(expanded(const QModelIndex &)), 0, 0);
	foreach(int row, m_expanded_rows) {
		expand(m_proxy->index(row, 0));
	}
	connect(this, SIGNAL(expanded(const QModelIndex &)), SLOT(index_expanded(const QModelIndex &)));
}