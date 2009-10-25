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
#include "customprofession.h"
#include "gamedatareader.h"
#include "ui_customprofession.h"
#include "dwarf.h"
#include "defines.h"
#include "labor.h"
#include "profession.h"

/*!
Default ctor. Creates a blank skill template with no name
*/
CustomProfession::CustomProfession(QObject *parent)
	: QObject(parent)
	, ui(new Ui::CustomProfessionEditor)
	, m_dwarf(0)
        , m_dialog(0)
{}

/*!
When passed in a pointer to a Dwarf, this new custom profession
will adopt whatever labors that Dwarf has enabled as its
own template. 

This is used by the "Create custom profession from this dwarf..." action.

\param[in] d The Dwarf to use as a labor template
\param[in] parent The Qt owner of this object
*/
CustomProfession::CustomProfession(Dwarf *d, QObject *parent)
	: QObject(parent)
	, ui(new Ui::CustomProfessionEditor)
	, m_dialog(0)
	, m_dwarf(d)
{
	GameDataReader *gdr = GameDataReader::ptr();	
	QList<Labor*> labors = gdr->get_ordered_labors();
	
	foreach(Labor *l, labors) {
		if (m_dwarf && m_dwarf->is_labor_enabled(l->labor_id))
			add_labor(l->labor_id);
	}
}

/*!
Change the enabled status of a template labor. This doesn't
affect any dwarves using this custom profession, only the 
template itself.

\param[in] labor_id The id of the labor to change
\param[in] active Should the labor be enabled or not
*/
void CustomProfession::set_labor(int labor_id, bool active) {
	if (m_active_labors.contains(labor_id) && !active)
		m_active_labors.remove(labor_id);
	if (active)
		m_active_labors.insert(labor_id, true);
}

/*!
Check if the template has a labor enabled

\param[in] labor_id The id of the labor to check
\returns true if this labor is enabled
*/
bool CustomProfession::is_active(int labor_id) {
	return m_active_labors.value(labor_id, false);
}

/*!
Get a vector of all enabled labors in this template by labor_id
*/
QVector<int> CustomProfession::get_enabled_labors() {
	QVector<int> labors;
	foreach(int labor, m_active_labors.uniqueKeys()) {
		if (m_active_labors.value(labor)) {
			labors << labor;
		}
	}
	return labors;
}

/*!
Pops up a dialog box asking for a name for this object as well
as a list of labors that can be enabled/disabled via checkboxes

\param[in] parent If set, the dialog will launch as a model under parent
\returns QDialog::exec() result (int)
*/
int CustomProfession::show_builder_dialog(QWidget *parent) {
	GameDataReader *gdr = GameDataReader::ptr();

	m_dialog = new QDialog(parent);
	ui->setupUi(m_dialog);
	ui->name_edit->setText(m_name);
	connect(ui->name_edit, SIGNAL(textChanged(const QString &)), this, SLOT(set_name(QString)));
	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	
	QList<Labor*> labors = gdr->get_ordered_labors();
	int num_active = 0;
	foreach(Labor *l, labors) {
		if (l->is_weapon)
			continue;
		QListWidgetItem *item = new QListWidgetItem(l->name, ui->labor_list);
		item->setData(Qt::UserRole, l->labor_id);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		if (is_active(l->labor_id)) {
			item->setCheckState(Qt::Checked);
			num_active++;
		} else {
			item->setCheckState(Qt::Unchecked);
		}
		ui->labor_list->addItem(item);
	}

	connect(ui->labor_list, 
			SIGNAL(itemChanged(QListWidgetItem*)),
			this,
			SLOT(item_check_state_changed(QListWidgetItem*)));


	ui->lbl_skill_count->setNum(num_active);
	int code = m_dialog->exec();
	m_dialog->deleteLater();
	return code;
}

/*!
Called when the show_builder_dialog widget's OK button is pressed, or the 
dialog is otherwise accepted by the user

We intercept this call to verify the form is valid before saving it.
\sa is_valid()
*/
void CustomProfession::accept() {
	if (!is_valid()) {
		return;
	}
	m_dialog->accept();
}

/*!
Called after the show_builder_dialog widget is accepted, used to verify that
the CustomProfession has all needed information to save

\returns true if this instance is ok to save.
*/
bool CustomProfession::is_valid() {
	if (!m_dialog)
		return true;

	QString proposed_name = ui->name_edit->text();
	if (proposed_name.isEmpty()) {
		QMessageBox::warning(m_dialog, tr("Naming Error!"),
			tr("You must enter a name for this custom profession!"));
		return false;
	}
	QHash<short, Profession*> profs = GameDataReader::ptr()->get_professions();
	foreach(Profession *p, profs) {
		if (proposed_name == p->name(true)) {
			QMessageBox::warning(m_dialog, tr("Naming Error!"),
				tr("The profession '%1' is a default game profession, please choose a different name.").arg(proposed_name));
			return false;
		}
	}
	return true;
}

void CustomProfession::item_check_state_changed(QListWidgetItem *item) {
	if (item->checkState() == Qt::Checked) {
		add_labor(item->data(Qt::UserRole).toInt());
		ui->lbl_skill_count->setNum(ui->lbl_skill_count->text().toInt() + 1);
	} else {
		remove_labor(item->data(Qt::UserRole).toInt());
		ui->lbl_skill_count->setNum(ui->lbl_skill_count->text().toInt() - 1);
	}
}

void CustomProfession::delete_from_disk() {
	QSettings s(QSettings::IniFormat, QSettings::UserScope, COMPANY, PRODUCT, this);
	s.beginGroup("custom_professions");
	s.remove(m_name);
	s.endGroup();
}
