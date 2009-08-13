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

#include "viewcolumnsetdialog.h"
#include "ui_viewcolumnsetdialog.h"
#include "ui_columneditdialog.h"
#include "viewmanager.h"
#include "viewcolumnset.h"
#include "viewcolumn.h"
#include "defines.h"
#include "gamedatareader.h"
#include "labor.h"
#include "spacercolumn.h"
#include "happinesscolumn.h"
#include "laborcolumn.h"
#include "skillcolumn.h"
#include "utils.h"

ViewColumnSetDialog::ViewColumnSetDialog(ViewManager *mgr, ViewColumnSet *set, QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::ViewColumnSetDialog)
	, m_set(set)
	, m_manager(mgr)
	, m_pending_bg_color(Qt::white)
	, m_is_editing(false)
	, m_original_name("")
{
	m_pending_columns.clear();
	ui->setupUi(this);
	ui->cp_bg_color->setCurrentColor(m_pending_bg_color);
	if (set) {
		ui->le_name->setText(set->name());
		ui->cp_bg_color->setCurrentColor(set->bg_color());
		m_pending_bg_color = set->bg_color();
		foreach(ViewColumn *vc, set->columns()) {
			m_pending_columns << vc;
		}
		ui->buttonBox->setEnabled(!set->name().isEmpty());
		m_is_editing = true;
		m_original_name = set->name();
	}
	ui->cp_bg_color->setStandardColors();
	ui->list_columns->installEventFilter(this);
	ui->cb_col_type->addItem("Special");
	ui->cb_col_type->addItem("Labor");
	ui->cb_col_type->addItem("Skill");

	connect(ui->le_name, SIGNAL(textChanged(const QString &)), this, SLOT(check_name(const QString &)));
	connect(ui->cp_bg_color, SIGNAL(colorChanged(const QColor &)), this, SLOT(update_color(const QColor &)));
	connect(ui->cb_col_type, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(type_chosen(const QString &)));
	connect(ui->btn_add_col, SIGNAL(clicked()), this, SLOT(add_column_from_gui()));
	connect(ui->list_columns, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_column_context_menu(const QPoint &)));
	connect(ui->list_columns, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(edit_column(QListWidgetItem*)));

	draw_columns();
}

QString ViewColumnSetDialog::name() {
	return ui->le_name->text();
}

QColor ViewColumnSetDialog::bg_color() {
	return m_pending_bg_color;
}

void ViewColumnSetDialog::check_name(const QString &name) {
	ui->buttonBox->setDisabled(name.isEmpty());
}

void ViewColumnSetDialog::update_color(const QColor &new_color) {
	m_pending_bg_color = new_color;
	draw_columns();
}

void ViewColumnSetDialog::draw_columns() {
	ui->list_columns->clear();
	foreach(ViewColumn *vc, m_pending_columns) {
		QString title = QString("%1 %2").arg(get_column_type(vc->type())).arg(vc->title());
		QListWidgetItem *item = new QListWidgetItem(title, ui->list_columns);
		item->setData(Qt::UserRole, vc->title());
		item->setData(Qt::UserRole + 1, vc->type());
		if (vc->override_color()) {
			item->setBackground(QBrush(vc->bg_color()));
			item->setForeground(QBrush(compliment(vc->bg_color())));
		} else {
			item->setBackground(QBrush(m_pending_bg_color));
			item->setForeground(QBrush(compliment(m_pending_bg_color)));
		}
	}
}

void ViewColumnSetDialog::type_chosen(const QString &type_name) {
	GameDataReader *gdr = GameDataReader::ptr();

	QStandardItemModel *m = new QStandardItemModel();
	ui->cb_column->clear();
	ui->cb_column->setEnabled(true);
	ui->btn_add_col->setEnabled(true);
	if (type_name == "Special") {
		QStandardItem *spacer = new QStandardItem("Spacer");
		spacer->setData(CT_SPACER, Qt::UserRole);
		m->appendRow(spacer);
		QStandardItem *happiness  = new QStandardItem("Happiness");
		happiness->setData(CT_HAPPINESS, Qt::UserRole);
		m->appendRow(happiness);
	} else if (type_name == "Labor") {
		foreach(Labor *l, gdr->get_ordered_labors()) {
			QStandardItem *i = new QStandardItem(l->name);
			i->setData(CT_LABOR, Qt::UserRole);
			i->setData(l->labor_id, Qt::UserRole + 1);
			m->appendRow(i);
		}
	} else if (type_name == "Skill") {
		QMap<int, QString> skills = gdr->get_skills();
		foreach(int skill_id, skills.uniqueKeys()) {
			QStandardItem *i = new QStandardItem(skills.value(skill_id, "UNKNOWN"));
			i->setData(CT_SKILL, Qt::UserRole);
			i->setData(skill_id, Qt::UserRole + 1);
			m->appendRow(i);
		}
	} else {
		ui->cb_column->setEnabled(false);
		ui->btn_add_col->setEnabled(false);
	}
	m->sort(0);
	ui->cb_column->setModel(m);
}

void ViewColumnSetDialog::add_column_from_gui() {
	COLUMN_TYPE type = static_cast<COLUMN_TYPE>(ui->cb_column->itemData(ui->cb_column->currentIndex()).toInt());
	QString name = ui->cb_column->currentText();
	ViewColumn *newcol = 0;
	switch(type) {
		case CT_SPACER:
			{
				SpacerColumn *c = new SpacerColumn("SPACER " + QString::number(m_pending_columns.size()), 0, this);
				newcol = c;
			}
			break;
		case CT_HAPPINESS:
			{
				HappinessColumn *c = new HappinessColumn(name, 0, this);
				newcol = c;
			}
			break;
		case CT_LABOR:
			{
				int labor_id = ui->cb_column->itemData(ui->cb_column->currentIndex(), Qt::UserRole + 1).toInt();
				GameDataReader *gdr = GameDataReader::ptr();
				Labor *l = gdr->get_labor(labor_id);
				if (l) {
					LaborColumn *c = new LaborColumn(name, l->labor_id, l->skill_id, 0, this);
					newcol = c;
				}
			}
			break;
		case CT_SKILL:
			{
				int skill_id = ui->cb_column->itemData(ui->cb_column->currentIndex(), Qt::UserRole + 1).toInt();
				GameDataReader *gdr = GameDataReader::ptr();
				SkillColumn *c = new SkillColumn(gdr->get_skill_name(skill_id), skill_id, 0, this);
				newcol = c;
			}
			break;
	}
	if (newcol) {
		m_pending_columns << newcol;
		draw_columns();
	}
}

bool ViewColumnSetDialog::eventFilter(QObject *, QEvent *e) {
	if (e->type() == QEvent::ChildRemoved) {
		order_changed(); // we're just here to help the signals along since this shit is broken in Qt...
	}
	return false; // don't actually interrupt anything
}

//! called when the user has re-arranged the columns in the gui..
void ViewColumnSetDialog::order_changed() {
	QList<ViewColumn*> new_cols;
	for (int i = 0; i < ui->list_columns->count(); ++i) {
		// find the VC that matches this item in the GUI list
		QListWidgetItem *item = ui->list_columns->item(i);
		QString title = item->data(Qt::UserRole).toString();
		COLUMN_TYPE type = static_cast<COLUMN_TYPE>(item->data(Qt::UserRole + 1).toInt());
		foreach(ViewColumn *vc, m_pending_columns) {
			if (vc->title() == title && vc->type() == type) {
				new_cols << vc;
			}
		}
	}
	m_pending_columns = new_cols;
	draw_columns();
}

void ViewColumnSetDialog::draw_column_context_menu(const QPoint &p) {
	QMenu m(this);
	QListWidgetItem *item = ui->list_columns->itemAt(p);
	if (!item)
		return;
	QAction *a = m.addAction(tr("Edit..."), this, SLOT(edit_column()));
	a->setData(ui->list_columns->row(item));
	a = m.addAction(tr("Remove..."), this, SLOT(remove_column()));
	a->setData(ui->list_columns->row(item));
	m.exec(ui->list_columns->viewport()->mapToGlobal(p));
}

void ViewColumnSetDialog::edit_column(QListWidgetItem *item) {
	int row = ui->list_columns->row(item);
	ViewColumn *vc = m_pending_columns.value(row, 0);
	if (vc)
		show_edit_column_dialog(vc);
}

void ViewColumnSetDialog::edit_column() {
	// find out which column we're editing...
	QAction *a = qobject_cast<QAction*>(QObject::sender());
	int row = a->data().toInt();
	ViewColumn *vc = m_pending_columns.value(row, 0);
	if (vc)
		show_edit_column_dialog(vc);
}

void ViewColumnSetDialog::remove_column() {
	QAction *a = qobject_cast<QAction*>(QObject::sender());
	int row = a->data().toInt();
	if (m_pending_columns.size() - 1 >= row) {
		m_pending_columns.removeAt(row);
		draw_columns();
	}
}

void ViewColumnSetDialog::show_edit_column_dialog(ViewColumn *vc) {
	// build the column dialog
	QDialog *d = new QDialog(this);
	Ui::ColumnEditDialog *dui = new Ui::ColumnEditDialog;
	dui->setupUi(d);
	d->setModal(true);

	connect(dui->cb_override, SIGNAL(toggled(bool)), dui->cp_bg_color, SLOT(setEnabled(bool)));

	if (vc->override_color())
		dui->cp_bg_color->setCurrentColor(vc->bg_color());
	else
		dui->cp_bg_color->setCurrentColor(m_pending_bg_color);
	dui->cp_bg_color->setStandardColors();
	dui->le_title->setText(vc->title());
	dui->cb_override->setChecked(vc->override_color());
	if (vc->type() == CT_SPACER) {
		SpacerColumn *c = static_cast<SpacerColumn*>(vc);
		dui->sb_width->setValue(c->width());
	} else { // don't show the width form for non-spacer columns
		dui->lbl_col_width->hide();
		dui->sb_width->hide();
		dui->verticalLayout->removeItem(dui->hbox_width);
	}

	int accepted = d->exec();
	if (accepted == QDialog::Accepted) {
		vc->set_title(dui->le_title->text());
		vc->set_override_color(dui->cb_override->isChecked());
		if (dui->cb_override->isChecked()) {
			vc->set_bg_color(dui->cp_bg_color->currentColor());
		}
		if (vc->type() == CT_SPACER) {
			SpacerColumn *c = static_cast<SpacerColumn*>(vc);
			int w = dui->sb_width->value();
			if (w < 1)
				w = DEFAULT_SPACER_WIDTH;
			c->set_width(w);
		}
		//REMOVEME
		draw_columns();
		emit column_changed(vc);
	}
	delete dui;
}

void ViewColumnSetDialog::accept() {
	if (ui->le_name->text().isEmpty()) {
		QMessageBox::warning(this, tr("Empty Name"), tr("Cannot save a set with no name!"));
		return;
	} else if (m_manager->get_set(ui->le_name->text())) {
		// this name exists
		if (!m_is_editing || (m_is_editing && m_original_name != ui->le_name->text())) {
			QMessageBox m(QMessageBox::Question, tr("Overwrite View?"),
				tr("There is already a set named <b>%1</b><h3>Do you want to overwrite it?</h3>").arg(ui->le_name->text()),
				QMessageBox::Yes | QMessageBox::No, 0);
			if (m.exec() == QMessageBox::Yes) {
				return QDialog::accept();
			} else {
				return;
			}
		}
	}
	return QDialog::accept();
}

