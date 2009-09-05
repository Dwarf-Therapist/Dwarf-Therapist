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
#include "customprofessionsexportdialog.h"
#include "ui_customprofessionsexportdialog.h"
#include "dwarftherapist.h"
#include "customprofession.h"
#include "version.h"
#include "gamedatareader.h"
#include "labor.h"

CustomProfessionsExportDialog::CustomProfessionsExportDialog(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::CustomProfessionsExportDialog)
{
	ui->setupUi(this);
	connect(ui->btn_select_none, SIGNAL(clicked()), SLOT(clear_selection()));
	connect(ui->btn_select_all, SIGNAL(clicked()), SLOT(select_all()));
}

void CustomProfessionsExportDialog::setup_for_export() {
	m_mode = MODE_EXPORT;
	QString default_path = QString("%1/%2").arg(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation), "custom_professions.dt");
	m_path = QFileDialog::getSaveFileName(this, tr("Choose a file to export to"),	default_path, 
		"Dwarf Therapist Profession Exports (*.dt);;All Files (*.*)");
	if (m_path.isEmpty())
		return; // they cancelled
	LOGD << "exporting custom professions to:" << m_path;

	setWindowTitle(tr("Export Custom Professions"));
	ui->buttonBox->addButton(tr("Export Selected"), QDialogButtonBox::YesRole);
	Version v;
	QDateTime t = QDateTime::currentDateTime();
	ui->lbl_file_path->setText(m_path);
	ui->lbl_version->setText(v.to_string());
	ui->lbl_export_time->setText(t.toString());
	ui->lbl_professions_count->setText(QString::number(0));
	foreach(CustomProfession *cp, DT->get_custom_professions()) {
		QString title = QString("%1 (%2)").arg(cp->get_name()).arg(cp->get_enabled_labors().size());
		QListWidgetItem *i = new QListWidgetItem(title, ui->list_professions);
		i->setData(Qt::UserRole, cp->get_name());
		i->setData(Qt::UserRole+1, false); // not conflicting as far as we know
		i->setCheckState(Qt::Checked);
		m_profs << cp;
	}
	


}

void CustomProfessionsExportDialog::setup_for_import() {
	m_mode = MODE_IMPORT;
	QString default_path = QString("%1/%2").arg(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation), "custom_professions.dt");
	m_path = QFileDialog::getOpenFileName(this, tr("Choose a file to import"), default_path, 
		"Dwarf Therapist Profession Exports (*.dt);;All Files (*.*)");
	if (m_path.isEmpty())
		return; // they cancelled
	LOGD << "importing custom professions from:" << m_path;

	setWindowTitle(tr("Import Custom Professions"));
	ui->buttonBox->addButton(tr("Import Selected"), QDialogButtonBox::YesRole);

	QSettings s(m_path, QSettings::IniFormat);

	/* don't need to check versions yet, since everything will be compatible */
	Version file_version;
	file_version.major = s.value("info/DT_version/major", 0).toInt();
	file_version.minor = s.value("info/DT_version/minor", 0).toInt();
	file_version.patch = s.value("info/DT_version/patch", 0).toInt();
	QDateTime t = s.value("info/export_date").toDateTime();

	int cnt = s.beginReadArray("custom_professions");
	for(int i = 0; i < cnt; i++) {
		s.setArrayIndex(i);
		CustomProfession *cp = new CustomProfession(DT);
		cp->set_name(s.value("name", "UNKNOWN").toString());
		int labor_cnt = s.beginReadArray("labors");
		for(int j = 0; j < labor_cnt; ++j) {
			s.setArrayIndex(j);
			cp->add_labor(s.childKeys()[0].toInt());
		}
		s.endArray();
		m_profs << cp;
	}
	s.endArray();

	ui->lbl_file_path->setText(m_path);
	ui->lbl_version->setText(file_version.to_string());
	ui->lbl_export_time->setText(t.toString());
	ui->lbl_professions_count->setText(QString::number(m_profs.size()));
	foreach(CustomProfession *cp, m_profs) {
		QString title = QString("%1 (%2)").arg(cp->get_name()).arg(cp->get_enabled_labors().size());
		QListWidgetItem *i = new QListWidgetItem(title, ui->list_professions);
		i->setData(Qt::UserRole, cp->get_name());
		i->setData(Qt::UserRole+1, false); // not conflicting as far as we know
		i->setCheckState(Qt::Checked);
		QString tooltip = "<h3>Enabled Labors</h3><ul>";
		GameDataReader *gdr = GameDataReader::ptr();
		foreach(int labor_id, cp->get_enabled_labors()) {
			tooltip += QString("<li>%1</li>").arg(gdr->get_labor(labor_id)->name);
		}
		tooltip += "</ul>";
		i->setToolTip(tooltip);

		// watch out for conflicts!
		if (DT->get_custom_profession(cp->get_name())) {
			i->setTextColor(Qt::red);
			i->setData(Qt::UserRole+1, true); // conflicting flag
			i->setText(i->text() + "CONFLICT");
			i->setCheckState(Qt::Unchecked);
			i->setFlags(Qt::NoItemFlags);
			i->setToolTip(tr("You already have a custom profession with this name!"));
		}
	}

}

void CustomProfessionsExportDialog::select_all() {
	for(int i = 0; i < ui->list_professions->count(); ++i) {
		QListWidgetItem *item = static_cast<QListWidgetItem*>(ui->list_professions->item(i));
		if (item->data(Qt::UserRole + 1).toBool())
			continue;
		ui->list_professions->item(i)->setCheckState(Qt::Checked);
	}
}

void CustomProfessionsExportDialog::clear_selection() {
	for(int i = 0; i < ui->list_professions->count(); ++i) {
		ui->list_professions->item(i)->setCheckState(Qt::Unchecked);
	}
}

QVector<CustomProfession*> CustomProfessionsExportDialog::get_profs() {
	QVector<CustomProfession*> out;
	for(int i = 0; i < ui->list_professions->count(); ++i) {
		QListWidgetItem *item = static_cast<QListWidgetItem*>(ui->list_professions->item(i));
		if (item->checkState() != Qt::Checked)
			continue;
		QString name = item->data(Qt::UserRole).toString();
		foreach(CustomProfession *cp, m_profs) {
			if (name == cp->get_name())
				out << cp;
		}
	}
	return out;
}

void CustomProfessionsExportDialog::accept() {
	switch(m_mode) {
		case MODE_EXPORT:
			export_selected();
			break;
		case MODE_IMPORT:
			import_selected();
			break;
	}
	return QDialog::accept();
}

void CustomProfessionsExportDialog::export_selected() {
	QSettings s(m_path, QSettings::IniFormat);
	s.remove(""); // clear out the file if there was anything there.
	Version v;
	s.setValue("info/DT_version/major", v.major);
	s.setValue("info/DT_version/minor", v.minor);
	s.setValue("info/DT_version/patch", v.patch);
	s.setValue("info/export_date", QDateTime::currentDateTime());

	int exported = 0;
	int i = 0;
	s.beginWriteArray("custom_professions");
	foreach(CustomProfession *cp, get_profs()) {
		s.setArrayIndex(i++);
		s.setValue("name", cp->get_name());
		s.beginWriteArray("labors");
		int j = 0;
		foreach(int labor_id, cp->get_enabled_labors()) {
			s.setArrayIndex(j++);
			s.setValue(QString::number(labor_id), true);
		}
		s.endArray();
		exported++;
	}
	s.endArray();
	s.sync();
	if (exported)
		QMessageBox::information(this, tr("Export Successful"), 
			tr("Exported %n custom profession(s)", "", exported));
}

void CustomProfessionsExportDialog::import_selected() {
	int imported = 0;
	foreach(CustomProfession *cp, get_profs()) {
		DT->add_custom_profession(cp);
		imported++;
	}
	if (imported)
		QMessageBox::information(this, tr("Import Successful"), 
			tr("Imported %n custom profession(s)", "", imported));
}
