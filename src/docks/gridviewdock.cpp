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
#include "gridviewdock.h"
#include "ui_gridviewdock.h"
#include "viewmanager.h"
#include "gridview.h"
#include "gridviewdialog.h"
#include "defines.h"

GridViewDock::GridViewDock(ViewManager *mgr, QWidget *parent, Qt::WindowFlags flags)
	: QDockWidget(parent, flags)
	, m_manager(mgr)
	, ui(new Ui::GridViewDock)
	, m_tmp_item(0)
{
	ui->setupUi(this);
	setFeatures(QDockWidget::AllDockWidgetFeatures);
	setAllowedAreas(Qt::AllDockWidgetAreas);
	draw_views();

	connect(ui->list_views, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(edit_view(QListWidgetItem*)));
	connect(ui->btn_add, SIGNAL(clicked()), this, SLOT(add_new_view()));
}

void GridViewDock::draw_views() {
	ui->list_views->clear();
	QStringList view_names;
	foreach(GridView *v, m_manager->views()) {
		view_names << v->name();
	}
	qSort(view_names);
	foreach(QString name, view_names) {
		foreach(GridView *v, m_manager->views()) {
			if (v->name() == name) {
				new QListWidgetItem(v->name(), ui->list_views);
			}
		}
	}
}

void GridViewDock::contextMenuEvent(QContextMenuEvent *e) {
	m_tmp_item = ui->list_views->itemAt(ui->list_views->viewport()->mapFromGlobal(e->globalPos()));
	if (!m_tmp_item)
		return;
	QMenu m(this);
	m.addAction(tr("Edit..."), this, SLOT(edit_view()));
	m.addAction(tr("Copy..."), this, SLOT(copy_view()));
	m.addAction(tr("Delete..."), this, SLOT(delete_view()));
	m.exec(e->globalPos());
}

void GridViewDock::add_new_view() {
	GridView *view = new GridView("", m_manager, m_manager);
	GridViewDialog *d = new GridViewDialog(m_manager, view, this);
	int accepted = d->exec();
	if (accepted == QDialog::Accepted) {
		GridView *new_view = d->pending_view();
		new_view->set_active(false);
		m_manager->add_view(view);
		m_manager->write_views();
		emit views_changed();
		draw_views();
	}
}

void GridViewDock::edit_view(QListWidgetItem *item) {
	m_tmp_item = item;
	edit_view();
}

void GridViewDock::edit_view() {
	if (!m_tmp_item)
		return;
	
	GridView *view = m_manager->get_view(m_tmp_item->text());
	if (!view)
		return;
	GridViewDialog *d = new GridViewDialog(m_manager, view, this);
	int accepted = d->exec();
	if (accepted == QDialog::Accepted) {
		m_manager->replace_view(view, d->pending_view());
		emit views_changed();
		draw_views();
	}
	m_tmp_item = 0;
}

void GridViewDock::copy_view() {
	if (!m_tmp_item)
		return;

	GridView *view = m_manager->get_view(m_tmp_item->text());
	if (!view)
		return;

	GridView *copy = new GridView(*view);
	copy->set_name(view->name() + "(COPY)");
	m_manager->add_view(copy);
	m_manager->write_views();
	emit views_changed();
	draw_views();
	m_tmp_item = 0;
}

void GridViewDock::delete_view() {
	if (!m_tmp_item)
		return;
	GridView *view = m_manager->get_view(m_tmp_item->text());
	if (!view)
		return;

	int answer = QMessageBox::question(
		0, tr("Delete View"),
		tr("Deleting '%1' will permanently delete it from disk. <h1>Really delete '%1'?</h1><h2>There is no undo!</h2>").arg(view->name()),
		QMessageBox::Yes | QMessageBox::No);
	if (answer == QMessageBox::Yes) {
		LOGI << "permanently deleting set" << view->name();
		m_manager->remove_view(view);
		emit views_changed();
		draw_views();
		view->deleteLater();
	}
	m_tmp_item = 0;
}
