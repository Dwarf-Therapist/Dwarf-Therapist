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
#include "viewcolumnsetdock.h"
#include "ui_viewcolumnsetdock.h"
#include "viewmanager.h"
#include "viewcolumnset.h"
#include "viewcolumnsetdialog.h"
#include "defines.h"

ViewColumnSetDock::ViewColumnSetDock(ViewManager *mgr, QWidget *parent, Qt::WindowFlags flags)
	: QDockWidget(parent, flags)
	, m_manager(mgr)
	, ui(new Ui::ViewColumnSetDock)
	, m_tmp_item(0)
{
	ui->setupUi(this);
	setFeatures(QDockWidget::AllDockWidgetFeatures);
	setAllowedAreas(Qt::AllDockWidgetAreas);
	draw_sets();

	connect(ui->list_sets, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(edit_set(QListWidgetItem*)));
	connect(ui->btn_add, SIGNAL(clicked()), this, SLOT(add_new_set()));
}

void ViewColumnSetDock::draw_sets() {
	ui->list_sets->clear();
	foreach(ViewColumnSet *set, m_manager->sets()) {
		new QListWidgetItem(set->name(), ui->list_sets);
	}
}

void ViewColumnSetDock::add_new_set() {
	ViewColumnSet *set = new ViewColumnSet("", m_manager);
	ViewColumnSetDialog *d = new ViewColumnSetDialog(m_manager, set, this);
	int accepted = d->exec();
	if (accepted == QDialog::Accepted) {
		set->update_from_dialog(d);
		emit sets_changed();
		draw_sets();
	}
}

void ViewColumnSetDock::contextMenuEvent(QContextMenuEvent *e) {
	m_tmp_item = ui->list_sets->itemAt(ui->list_sets->viewport()->mapFromGlobal(e->globalPos()));
	if (!m_tmp_item)
		return;
	QMenu m(this);
	m.addAction(tr("Edit..."), this, SLOT(edit_set()));
	m.addAction(tr("Delete..."), this, SLOT(delete_set()));
	m.exec(e->globalPos());
}

void ViewColumnSetDock::edit_set(QListWidgetItem *item) {
	m_tmp_item = item;
	edit_set();
}

void ViewColumnSetDock::edit_set() {
	if (!m_tmp_item)
		return;
	
	ViewColumnSet *set= m_manager->get_set(m_tmp_item->text());
	if (!set)
		return;
	
	ViewColumnSetDialog *d = new ViewColumnSetDialog(m_manager, set, m_manager);
	int accepted = d->exec();
	if (accepted == QDialog::Accepted) {
		set->update_from_dialog(d);
		emit sets_changed();
		draw_sets();
	}
	m_tmp_item = 0;
}
