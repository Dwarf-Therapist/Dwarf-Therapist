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
#include "columntypes.h"
#include "viewcolumnset.h"
#include "laborcolumn.h"
#include "happinesscolumn.h"
#include "spacercolumn.h"
#include "skillcolumn.h"
#include "gamedatareader.h"
#include "defines.h"
#include "labor.h"
#include "utils.h"
#include "ui_viewcolumnsetdialog.h"
#include "ui_columneditdialog.h"

ViewColumnSet::ViewColumnSet(QString name, QObject *parent)
	: QObject(parent)
	, ui(new Ui::ViewColumnSetDialog)
	, m_name(name)
	, m_dialog(0)
{}

void ViewColumnSet::set_name(const QString &name) {
	m_name = name;
}

void ViewColumnSet::add_column(ViewColumn *col) {
	m_columns << col;
}
	
void ViewColumnSet::clear_columns() {
	foreach(ViewColumn *col, m_columns) {
		col->deleteLater();
	}
	m_columns.clear();
}

void ViewColumnSet::reset_from_disk() {
	QSettings s(m_filename, QSettings::IniFormat);
	QString set_name = s.value("info/name", "UNKNOWN").toString();
	QString color_in_hex = s.value("info/bg_color", "0xFFFFFF").toString();
	QColor bg_color = from_hex(color_in_hex);

	clear_columns();
	
	m_name = set_name;
	m_bg_color = bg_color;
	
	int cols = s.beginReadArray("columns");
	for (int i = 0; i < cols; ++i) {
		s.setArrayIndex(i);
		QString tmp_type = s.value("type", "DEFAULT").toString();
		QString col_name = s.value("name", "UNKNOWN " + QString::number(i)).toString();
		COLUMN_TYPE type = get_column_type(tmp_type);
		switch (type) {
			case CT_SKILL:
				{
					int skill_id = s.value("skill_id", -1).toInt();
					//TODO: check that labor and skill are known ids
					new SkillColumn(col_name, skill_id, this, this);
				}
				break;
			case CT_LABOR:
				{
					int labor_id = s.value("labor_id", -1).toInt();
					int skill_id = s.value("skill_id", -1).toInt();
					//TODO: check that labor and skill are known ids
					new LaborColumn(col_name, labor_id, skill_id, this, this);
				}
				break;
			case CT_HAPPINESS:
				new HappinessColumn(col_name, this, this);
				break;
			case CT_SPACER:
				{
					int width = s.value("width", DEFAULT_SPACER_WIDTH).toInt();
					QString hex_color = s.value("bg_color").toString();
					QColor bg_color = from_hex(hex_color);
					SpacerColumn *c = new SpacerColumn(col_name, this, this);
					c->set_override_color(s.value("override_color").toBool());
					if (c->override_color()) {
						c->set_bg_color(bg_color);
					}
					c->set_width(width);
				}
				break;
			default:
				LOGW << "Column " << col_name << "in set" << set_name << "has unknown type: " << tmp_type;
				break;
		}
	}
	s.endArray();
}

ViewColumnSet *ViewColumnSet::from_file(QString filename, QObject *parent) {
	ViewColumnSet *ret_val = new ViewColumnSet("", parent);
	ret_val->set_filename(filename);
	ret_val->reset_from_disk();
	return ret_val;
}

void ViewColumnSet::write_settings() {
	if (m_filename.isEmpty())
		m_filename = m_name + ".ini";

	QSettings s(m_filename, QSettings::IniFormat);
	s.setValue("info/name", m_name);
	s.setValue("info/bg_color", to_hex(m_bg_color));

	s.remove("columns");
	s.beginWriteArray("columns", columns().size());
	int i = 0;
	foreach(ViewColumn *vc, columns()) {
		s.setArrayIndex(i++);
		
		//cleanup
		s.remove("override_color");
		s.remove("bg_color");
		s.remove("width");
		s.remove("labor_id");
		s.remove("skill_id");

		if (!vc->title().isEmpty())
			s.setValue("name", vc->title());
		s.setValue("type", get_column_type(vc->type()));
		if (vc->override_color()) {
			s.setValue("override_color", true);
			s.setValue("bg_color", to_hex(vc->bg_color()));
		}
		switch (vc->type()) {
			case CT_SKILL:
				{
					SkillColumn *c = static_cast<SkillColumn*>(vc);
					s.setValue("skill_id", c->skill_id());
				}
				break;
			case CT_LABOR:
				{
					LaborColumn *c = static_cast<LaborColumn*>(vc);
					s.setValue("labor_id", c->labor_id());
					s.setValue("skill_id", c->skill_id());
				}
				break;
			case CT_SPACER:
				{
					SpacerColumn *c = static_cast<SpacerColumn*>(vc);
					if (c->width() > 0)
						s.setValue("width", c->width());
				}
				break;
			default:
				break;
		}
	}
	s.endArray();
}

void ViewColumnSet::update_color(const QColor &new_color) {
	set_bg_color(new_color);
	draw_columns();
}

void ViewColumnSet::type_chosen(const QString &type_name) {
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

void ViewColumnSet::add_column_from_gui() {
	COLUMN_TYPE type = static_cast<COLUMN_TYPE>(ui->cb_column->itemData(ui->cb_column->currentIndex()).toInt());
	QString name = ui->cb_column->currentText();
	ViewColumn *newcol = 0;
	switch(type) {
		case CT_SPACER:
			{
				SpacerColumn *c = new SpacerColumn("", this, this);
				newcol = c;
			}
			break;
		case CT_HAPPINESS:
			{
				HappinessColumn *c = new HappinessColumn(name, this, this);
				newcol = c;
			}
			break;
		case CT_LABOR:
			{
				int labor_id = ui->cb_column->itemData(ui->cb_column->currentIndex(), Qt::UserRole + 1).toInt();
				GameDataReader *gdr = GameDataReader::ptr();
				Labor *l = gdr->get_labor(labor_id);
				if (l) {
					LaborColumn *c = new LaborColumn(name, l->labor_id, l->skill_id, this, this);
					newcol = c;
				}
			}
			break;
		case CT_SKILL:
			{
				int skill_id = ui->cb_column->itemData(ui->cb_column->currentIndex(), Qt::UserRole + 1).toInt();
				GameDataReader *gdr = GameDataReader::ptr();
				SkillColumn *c = new SkillColumn(gdr->get_skill_name(skill_id), skill_id, this, this);
				newcol = c;
			}
			break;
	}
	if (newcol) {
		add_column(newcol);
		draw_columns();
		/*QString title = QString("%1 %2").arg(get_column_type(newcol->type())).arg(newcol->title());
		QListWidgetItem *item = new QListWidgetItem(title, ui->list_columns);
		item->setData(Qt::UserRole, newcol->title());
		if (newcol->override_color()) 
			item->setBackgroundColor(newcol->bg_color());
		else
			item->setBackgroundColor(m_bg_color);
			*/

	}
}

void ViewColumnSet::draw_column_context_menu(const QPoint &p) {
	QMenu m(m_dialog);
	QAction *a = m.addAction(tr("Edit..."), this, SLOT(edit_column()));
	a->setData(p);
	a = m.addAction(tr("Remove..."), this, SLOT(remove_column()));
	a->setData(p);
	m.exec(ui->list_columns->viewport()->mapToGlobal(p));
}

void ViewColumnSet::edit_column(QListWidgetItem *item) {
	int row = ui->list_columns->row(item);
	ViewColumn *vc = m_columns.value(row, 0);
	if (vc)
		show_edit_column_dialog(vc);
}

void ViewColumnSet::edit_column() {
	// find out which column we're editing...
	QAction *a = qobject_cast<QAction*>(QObject::sender());
	QPoint  p = a->data().value<QPoint>();
	QModelIndex idx = ui->list_columns->indexAt(p);
	ViewColumn *vc = m_columns.value(idx.row(), 0);
	if (vc)
		show_edit_column_dialog(vc);
}

void ViewColumnSet::show_edit_column_dialog(ViewColumn *vc) {
	// build the column dialog
	QDialog *d = new QDialog(m_dialog);
	Ui::ColumnEditDialog *dui = new Ui::ColumnEditDialog;
	dui->setupUi(d);
	d->setModal(true);
	
	connect(dui->cb_override, SIGNAL(toggled(bool)), dui->cp_bg_color, SLOT(setEnabled(bool)));

	if (vc->override_color())
		dui->cp_bg_color->setCurrentColor(vc->bg_color());
	else
		dui->cp_bg_color->setCurrentColor(m_bg_color);
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
		draw_columns();
		emit set_changed();
	}
	delete dui;
}

void ViewColumnSet::remove_column() {
	QAction *a = qobject_cast<QAction*>(QObject::sender());
	QPoint  p = a->data().value<QPoint>();
	QListWidgetItem *item = ui->list_columns->itemAt(p);
	int row = ui->list_columns->row(item);
	if (m_columns.size() >= row) {
		m_columns.removeAt(row);
		draw_columns();
	}
}

bool ViewColumnSet::eventFilter(QObject *, QEvent *e) {
	if (e->type() == QEvent::ChildRemoved) {
		order_changed();
		return false;
	}
	return false;
}

//! called when the user has re-arranged the columns in the gui..
void ViewColumnSet::order_changed() {
	QList<ViewColumn*> new_views;
	for (int i = 0; i < ui->list_columns->count(); ++i) {
		// find the VC that matches this item in the GUI list
		QListWidgetItem *item = ui->list_columns->item(i);
		QString title = item->data(Qt::UserRole).toString();
		COLUMN_TYPE type = static_cast<COLUMN_TYPE>(item->data(Qt::UserRole + 1).toInt());
		foreach(ViewColumn *vc, columns()) {
			if (vc->title() == title && vc->type() == type) {
				new_views << vc;
			}
		}
	}
	m_columns = new_views;
	draw_columns();
}

void ViewColumnSet::draw_columns() {
	ui->list_columns->clear();
	foreach(ViewColumn *vc, m_columns) {
		QString title = QString("%1 %2").arg(get_column_type(vc->type())).arg(vc->title());
		QListWidgetItem *item = new QListWidgetItem(title, ui->list_columns);
		item->setData(Qt::UserRole, vc->title());
		item->setData(Qt::UserRole + 1, vc->type());
		if (vc->override_color())
			item->setBackgroundColor(vc->bg_color());
		else
			item->setBackgroundColor(m_bg_color);
	}
}

void ViewColumnSet::delete_from_disk() {
	int answer = QMessageBox::question(
		0, tr("Really delete '%1' forever?").arg(m_name),
		tr("Deleting '%1' will permanently delete it from disk. Also if any views are currently using"
		   " this set, they may become corrupted. There is no undo!").arg(m_name),
		QMessageBox::Yes | QMessageBox::No);
	if (answer == QMessageBox::Yes) {
		LOGD << "permanently deleting set" << m_name;
		QFile f;
		if (f.remove(m_filename)) {
			emit set_deleted();
			deleteLater();
		}
	}
}

int ViewColumnSet::show_builder_dialog() {
	return show_builder_dialog(0);
}

int ViewColumnSet::show_builder_dialog(QWidget *parent) {
	m_dialog = new QDialog(parent);
	ui->setupUi(m_dialog);
	ui->le_name->setText(m_name);
	ui->cp_bg_color->setCurrentColor(m_bg_color);
	ui->cp_bg_color->setStandardColors();

	// Pop
	ui->cb_col_type->addItem("Special");
	ui->cb_col_type->addItem("Labor");
	ui->cb_col_type->addItem("Skill");


	connect(ui->le_name, SIGNAL(textChanged(const QString &)), this, SLOT(set_name(const QString &)));
	connect(ui->cp_bg_color, SIGNAL(colorChanged(const QColor &)), this, SLOT(update_color(const QColor &)));
	connect(ui->cb_col_type, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(type_chosen(const QString &)));
	connect(ui->btn_add_col, SIGNAL(clicked()), this, SLOT(add_column_from_gui()));
	connect(ui->list_columns, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_column_context_menu(const QPoint &)));
	connect(ui->list_columns, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(edit_column(QListWidgetItem*)));

	ui->list_columns->installEventFilter(this);

	draw_columns();

	int code = m_dialog->exec();
	if (code == QDialog::Accepted) {
		//write to disk?
	} else {
		reset_from_disk();
	}
	delete m_dialog;
	return code;
}