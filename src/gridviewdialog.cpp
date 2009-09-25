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

#include "gridviewdialog.h"
#include "ui_gridviewdialog.h"
#include "viewmanager.h"
#include "gridview.h"
#include "viewcolumnset.h"
#include "viewcolumn.h"
#include "defines.h"

GridViewDialog::GridViewDialog(ViewManager *mgr, GridView *view, QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::GridViewDialog)
	, m_view(view)
	, m_manager(mgr)
	, m_is_editing(false)
	, m_model(new ViewManagerItemModel(this))
{
	ui->setupUi(this);
	ui->tree->setModel(m_model);
	
	if (!m_view->name().isEmpty()) { // looks like this is an edit...
		ui->le_name->setText(view->name());
		draw_sets();
		ui->buttonBox->setEnabled(!view->name().isEmpty());
		m_is_editing = true;
		m_original_name = view->name();
	}
	ui->tree->installEventFilter(this);

	connect(ui->tree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_set_context_menu(const QPoint &)));
	connect(ui->le_name, SIGNAL(textChanged(const QString &)), SLOT(check_name(const QString &)));
}

QString GridViewDialog::name() {
	 return ui->le_name->text();
}

QStringList GridViewDialog::sets() {
	QStringList retval;
	return retval;
	/*
	for(int i = 0; i < ui->list_sets->count(); ++i) {
		retval << ui->list_sets->item(i)->text();
	}
	return retval;
	*/
}

void GridViewDialog::draw_sets() {
	m_model->clear();
	foreach(ViewColumnSet *set, m_view->sets()) {
		QStandardItem *set_item = new QStandardItem(set->name());
		set_item->setBackground(set->bg_color());
		m_model->appendRow(set_item);

		foreach(ViewColumn *vc, set->columns()) {
			QStandardItem *col_item = new QStandardItem(vc->title());
			set_item->appendRow(col_item);
			if (vc->override_color())
				col_item->setBackground(vc->bg_color());
			else
				col_item->setBackground(set->bg_color());
		}
	}
}
/*
bool GridViewDialog::eventFilter(QObject *, QEvent *e) {
	if (e->type() == QEvent::ChildRemoved) {
		order_changed(); // we're just here to help the signals along since this shit is broken in Qt...
	}
	return false; // don't actually interrupt anything
}
*/

void GridViewDialog::order_changed() {
	LOGD << "ORDER CHANGED!";
}

void GridViewDialog::check_name(const QString &name) {
	ui->buttonBox->setDisabled(name.isEmpty());
}

void GridViewDialog::add_set() {
	/*
	QString set_name = ui->cb_sets->currentText();
	ViewColumnSet *set = m_manager->get_set(set_name);
	m_view->add_set(set);
	draw_sets();
	*/
}

void GridViewDialog::remove_set_from_action() {
	/*
	if (!m_temp_item)
		return;
	if (!m_temp_item->childCount()) // leaf
		ui->tree->model->in

		QTreeWidgetItem *removed = ui->tree->takeTopLevelItem(idx.row());
		delete removed;
	}
	*/
}

void GridViewDialog::draw_set_context_menu(const QPoint &p) {
	/*
	QMenu m(this);
	QTreeWidgetItem *item = ui->tree->itemAt(p);
	if (!item)
		return;
	QAction *a = m.addAction(tr("Remove..."), this, SLOT(remove_set_from_action()));
	m_temp_item = item;
	m.exec(ui->tree->viewport()->mapToGlobal(p));
	*/
}

void GridViewDialog::accept() {
	/*
	if (ui->le_name->text().isEmpty()) {
		QMessageBox::warning(this, tr("Empty Name"), tr("Cannot save a view with no name!"));
		return;
	} else if (m_manager->get_view(ui->le_name->text())) {
		// this name exists
		if (!m_is_editing || (m_is_editing && m_original_name != ui->le_name->text())) {
			QMessageBox m(QMessageBox::Question, tr("Overwrite View?"),
				tr("There is already a view named <b>%1</b><h3>Do you want to overwrite it?</h3>").arg(ui->le_name->text()),
				QMessageBox::Yes | QMessageBox::No, 0);
			if (m.exec() == QMessageBox::Yes) {
				return QDialog::accept();
			} else {
				return;
			}
		}
	}
	*/
	//return QDialog::accept();
	return QDialog::reject();
}