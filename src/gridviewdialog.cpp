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
#include "statetableview.h"
#include "gamedatareader.h"
#include "labor.h"

GridViewDialog::GridViewDialog(ViewManager *mgr, GridView *view, QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::GridViewDialog)
	, m_view(view)
	, m_manager(mgr)
	, m_is_editing(false)
	, m_set_model(new QStandardItemModel)
	, m_col_model(new QStandardItemModel)
{
	ui->setupUi(this);
	ui->list_sets->setModel(m_set_model);
	ui->list_columns->setModel(m_col_model);

	connect(ui->list_sets->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
		SLOT(set_selection_changed(const QItemSelection&, const QItemSelection&)));

	if (!m_view->name().isEmpty()) { // looks like this is an edit...
		ui->le_name->setText(view->name());
		draw_sets();
		ui->buttonBox->setEnabled(!view->name().isEmpty());
		m_is_editing = true;
		m_original_name = view->name();
	}
	//ui->tree->installEventFilter(this);

	/* TODO: show a STV with a preview using this gridview...
	StateTableView *stv = new StateTableView(this);
	QStandardItemModel *m = new QStandardItemModel(this);
	stv->setModel(m);
	ui->vbox->addWidget(stv, 10);
	*/
	connect(ui->list_sets, SIGNAL(activated(const QModelIndex &)), SLOT(edit_set(const QModelIndex &)));
	connect(ui->list_sets, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_set_context_menu(const QPoint &)));
	connect(ui->list_columns, SIGNAL(activated(const QModelIndex &)), SLOT(edit_column(const QModelIndex &)));
	connect(ui->list_columns, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_column_context_menu(const QPoint &)));
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
	m_set_model->clear();
	m_col_model->clear();
	foreach(ViewColumnSet *set, m_view->sets()) {
		QStandardItem *set_item = new QStandardItem(set->name());
		set_item->setBackground(set->bg_color());
		set_item->setDropEnabled(false);
		set_item->setData(set->name(), GPDT_TITLE);
		set_item->setData(set->bg_color(), GPDT_BG_COLOR);
		m_set_model->appendRow(set_item);
	}
	ui->list_sets->selectionModel()->select(m_set_model->index(0,0), QItemSelectionModel::SelectCurrent);
}

void GridViewDialog::set_selection_changed(const QItemSelection &selected, const QItemSelection &) {
	if (selected.indexes().size() != 1)
		return;
	QStandardItem *set_item = m_set_model->itemFromIndex(selected.indexes().at(0));
	foreach(ViewColumnSet *set, m_view->sets()) {
		if (set->name() == set_item->data(GPDT_TITLE).toString()) {
			m_col_model->clear();
			foreach(ViewColumn *vc, set->columns()) {
				QStandardItem *item = new QStandardItem(vc->title());
				item->setBackground(vc->override_color() ? vc->bg_color() : set_item->data(GPDT_BG_COLOR).value<QColor>());
				item->setDropEnabled(false);
				m_col_model->appendRow(item);
			}
		}
	}
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

void GridViewDialog::edit_set() {
	if (m_temp_set < 0)
		return;
	edit_set(m_set_model->index(m_temp_set, 0));
}

void GridViewDialog::edit_set(const QModelIndex &idx) {
	QStandardItem *item = m_set_model->itemFromIndex(idx);

	QDialog *d = new QDialog(this);
	QVBoxLayout *vbox = new QVBoxLayout(d);
	d->setLayout(vbox);

	QFormLayout *form = new QFormLayout(d);
	QLineEdit *le_name = new QLineEdit(item->text(), d);
	QtColorPicker *cp = new QtColorPicker(d);
	cp->setStandardColors();
	cp->setCurrentColor(item->background().color());

	form->addRow(tr("Name of set"), le_name);
	form->addRow(tr("Background color"), cp);

	QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, d);
	vbox->addLayout(form, 10);
	vbox->addWidget(buttons);

	connect(buttons, SIGNAL(accepted()), d, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), d, SLOT(reject()));

	if (d->exec()) {
		item->setBackground(cp->currentColor());
		item->setData(cp->currentColor(), GPDT_BG_COLOR);
		item->setText(le_name->text());
		set_selection_changed(ui->list_sets->selectionModel()->selection(), QItemSelection());
	}
	d->deleteLater();
}

void GridViewDialog::remove_set() {
	if (m_temp_set < 0)
		return;
	m_set_model->removeRow(m_temp_set);
	m_temp_set = -1;
}

void GridViewDialog::draw_set_context_menu(const QPoint &p) {
	QMenu m(this);
	QModelIndex idx = ui->list_sets->indexAt(p);
	if (!idx.isValid())
		return;
	m.addAction(QIcon(":img/application_edit.png"), tr("Edit..."), this, SLOT(edit_set()));
	m.addAction(QIcon(":img/delete.png"), tr("Remove"), this, SLOT(remove_set()));
	m_temp_set = idx.row();
	m.exec(ui->list_sets->viewport()->mapToGlobal(p));
}


void GridViewDialog::edit_column() {
	if (m_temp_col < 0)
		return;
	edit_column(m_col_model->index(m_temp_col, 0));
}

void GridViewDialog::edit_column(const QModelIndex &idx) {
	QStandardItem *item = m_col_model->itemFromIndex(idx);
	/*

	QDialog *d = new QDialog(this);
	QVBoxLayout *vbox = new QVBoxLayout(d);
	d->setLayout(vbox);

	QFormLayout *form = new QFormLayout(d);
	QLineEdit *le_name = new QLineEdit(item->text(), d);
	QtColorPicker *cp = new QtColorPicker(d);
	cp->setStandardColors();
	cp->setCurrentColor(item->background().color());

	form->addRow(tr("Name of set"), le_name);
	form->addRow(tr("Background color"), cp);

	QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, d);
	vbox->addLayout(form, 10);
	vbox->addWidget(buttons);

	connect(buttons, SIGNAL(accepted()), d, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), d, SLOT(reject()));

	if (d->exec()) {
		item->setBackground(cp->currentColor());
		item->setData(cp->currentColor(), GPDT_BG_COLOR);
		item->setText(le_name->text());
		set_selection_changed(ui->list_sets->selectionModel()->selection(), QItemSelection());
	}
	d->deleteLater();
	*/
}

void GridViewDialog::remove_column() {
	if (m_temp_col < 0)
		return;
	m_col_model->removeRow(m_temp_col);
	m_temp_col = -1;
}

void GridViewDialog::draw_column_context_menu(const QPoint &p) {
	QMenu *m = new QMenu(this);
	QModelIndex idx = ui->list_columns->indexAt(p);
	if (idx.isValid()) { // context on a column
		m->addAction(QIcon(":img/application_edit.png"), tr("Edit..."), this, SLOT(edit_column()));
		m->addAction(QIcon(":img/delete.png"), tr("Remove"), this, SLOT(remove_column()));
		m_temp_col = idx.row();
	} else { // in whitespace
		GameDataReader *gdr = GameDataReader::ptr();
		QMenu *m_special = m->addMenu(tr("Add Special"));
		QAction *a = m_special->addAction("Spacer", this, SLOT(add_spacer_column()));
		a->setToolTip(tr("Adds a non-selectable spacer to this set. You can set a custom width and color on spacer columns."));
		a = m_special->addAction("Happiness", this, SLOT(add_happiness_column()));
		a->setToolTip(tr("Adds a single column that shows a color-coded happiness indicator for "
			"each dwarf. You can customize the colors used in the options menu."));

		QMenu *m_labor = m->addMenu(tr("Add Labor Column"));
		m_labor->setTearOffEnabled(true);
		m_labor->setToolTip(tr("Labor columns function as toggle switches for individual labors on a dwarf."));
		foreach(Labor *l, gdr->get_ordered_labors()) {
			QAction *a = m_labor->addAction(l->name, this, SLOT(add_labor_column()));
			a->setData(l->labor_id);
			a->setToolTip(tr("Add a column for labor %1 (ID%2)").arg(l->name).arg(l->labor_id));
		}

		QMenu *m_skill = m->addMenu(tr("Add Skill Column"));
		m_skill->setTearOffEnabled(true);
		m_skill->setToolTip(tr("Skill columns function as a read-only display of a dwarf's skill in a particular area."
			" Note that you can add skill columns for labors but they won't work as toggles."));
		QHash<int, QString> skills = gdr->get_skills();
		foreach(int skill_id, skills.uniqueKeys()) {
			QAction *a = m_skill->addAction(skills.value(skill_id), this, SLOT(add_skill_column()));
			a->setData(skill_id);
			a->setToolTip(tr("Add a column for skill %1 (ID%2)").arg(skills.value(skill_id)).arg(skill_id));
		}
	}
	m->exec(ui->list_columns->viewport()->mapToGlobal(p));
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