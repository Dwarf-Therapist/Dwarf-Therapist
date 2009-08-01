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

#include "optionsmenu.h"
#include "customcolor.h"
#include "dwarftherapist.h"

OptionsMenu::OptionsMenu(MainWindow *parent)
	: QDialog(parent)
	, ui(new Ui::OptionsMenu)
	, m_reading_settings(false)
{
	ui->setupUi(this);
	
	m_general_colors 
		<< new CustomColor(tr("Cursor"), "", "cursor", QColor(0xFF00FF), this)
		<< new CustomColor(tr("Active Labor Cell"), "", "active_labor", QColor(0xE0FFE0), this)
		<< new CustomColor(tr("Active Group Cell"), "", "active_group", QColor(0x33FF33), this)
		<< new CustomColor(tr("Inactive Group Cell"), "", "inactive_group", QColor(0x999999), this)
		<< new CustomColor(tr("Partial Group Cell"), "", "partial_group", QColor(0xCCCCCC), this)
		<< new CustomColor(tr("Selection Guides"), "", "guides", QColor(0x0099FF), this)
		<< new CustomColor(tr("Main Border"), "", "border", QColor(0xd9d9d9), this)
		<< new CustomColor(tr("Dirty Cell Indicator"), "", "dirty_border", QColor(0xFF6600), this);

	QVBoxLayout *main_layout = new QVBoxLayout();
	foreach(CustomColor *cc, m_general_colors) {
		main_layout->addWidget(cc);
	}
	ui->group_general_colors->setLayout(main_layout);

	connect(ui->btn_restore_defaults, SIGNAL(pressed()), this, SLOT(restore_defaults()));
	read_settings();
}

OptionsMenu::~OptionsMenu() {}

void OptionsMenu::read_settings() {
	m_reading_settings = true;
	QSettings *s = DT->user_settings();
	s->beginGroup("options");
	s->beginGroup("colors");
	QColor c;
	foreach(CustomColor *cc, m_general_colors) {
		c = s->value(cc->get_config_key(), cc->get_default()).value<QColor>();
		cc->set_color(c);
		emit color_changed(cc->get_config_key(), c);
	}
	s->endGroup();
	s->beginGroup("grid");
	int cell_padding = s->value("cell_padding", 0).toInt();
	ui->sb_cell_padding->setValue(cell_padding);
	s->endGroup();
	s->endGroup();
	m_reading_settings = false;
}

void OptionsMenu::write_settings() {
	if (!m_reading_settings) {
		QSettings *s = DT->user_settings();
		s->beginGroup("options");
		s->beginGroup("colors");
		foreach(CustomColor *cc, m_general_colors) {
			s->setValue(cc->get_config_key(), cc->get_color());
		}
		s->endGroup();
		s->beginGroup("grid");
		s->setValue("cell_padding", ui->sb_cell_padding->value());
		s->endGroup();
		s->endGroup();
	}
}

void OptionsMenu::accept() {
	write_settings();
	emit settings_changed();
	QDialog::accept();
}

void OptionsMenu::reject() {
	read_settings();
	QDialog::reject();
}

void OptionsMenu::restore_defaults() {
	foreach(CustomColor *cc, m_general_colors) {
		cc->reset_to_default();
	}
}