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
#include "utils.h"
#include "defines.h"

OptionsMenu::OptionsMenu(MainWindow *parent)
	: QDialog(parent)
	, ui(new Ui::OptionsMenu)
	, m_reading_settings(false)
{
	ui->setupUi(this);
	
	m_general_colors 
		<< new CustomColor(tr("Skill"), "", "skill", from_hex("0xAAAAAAFF"), this)
		<< new CustomColor(tr("Active Labor Cell"), "", "active_labor", from_hex("0xE0FFE0FF"), this)
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
	connect(ui->btn_change_font, SIGNAL(pressed()), this, SLOT(show_font_chooser()));
	connect(ui->cb_auto_contrast, SIGNAL(toggled(bool)), m_general_colors[0], SLOT(setDisabled(bool)));
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
	}
	s->endGroup();
	s->beginGroup("grid");
	ui->sb_cell_size->setValue(s->value("cell_size", DEFAULT_CELL_SIZE).toInt());
	ui->sb_cell_padding->setValue(s->value("cell_padding", 0).toInt());
	
	m_font = s->value("font", QFont("Segoe UI", 8)).value<QFont>();
	m_dirty_font = m_font;
	ui->lbl_current_font->setText(m_font.family() + " [" + QString::number(m_font.pointSize()) + "pt]");
	
	s->endGroup();
	ui->cb_read_dwarves_on_startup->setChecked(s->value("read_on_startup", true).toBool());
	ui->cb_auto_contrast->setChecked(s->value("auto_contrast", true).toBool());
	ui->cb_show_aggregates->setChecked(s->value("show_aggregates", true).toBool());
	ui->cb_single_click_labor_changes->setChecked(s->value("single_click_labor_changes", true).toBool());
	ui->cb_show_toolbar_text->setChecked(s->value("show_toolbutton_text", true).toBool());
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
		s->setValue("cell_size", ui->sb_cell_size->value());
		s->setValue("cell_padding", ui->sb_cell_padding->value());
		s->setValue("font", m_font);
		s->endGroup();

		s->setValue("read_on_startup", ui->cb_read_dwarves_on_startup->isChecked());
		s->setValue("auto_contrast", ui->cb_auto_contrast->isChecked());
		s->setValue("show_aggregates", ui->cb_show_aggregates->isChecked());
		s->setValue("single_click_labor_changes", ui->cb_single_click_labor_changes->isChecked());
		s->setValue("show_toolbutton_text", ui->cb_show_toolbar_text->isChecked());
		
		s->endGroup();
	}
}

void OptionsMenu::accept() {
	m_font = m_dirty_font;
	write_settings();
	emit settings_changed();
	QDialog::accept();
}

void OptionsMenu::reject() {
	m_dirty_font = m_font;
	read_settings();
	QDialog::reject();
}

void OptionsMenu::restore_defaults() {
	foreach(CustomColor *cc, m_general_colors) {
		cc->reset_to_default();
	}
	ui->cb_read_dwarves_on_startup->setChecked(true);
	ui->cb_auto_contrast->setChecked(true);
	ui->cb_show_aggregates->setChecked(true);
	ui->cb_single_click_labor_changes->setChecked(false);
	ui->cb_show_toolbar_text->setChecked(true);

	m_font = QFont("Segoe UI", 8);
	m_dirty_font = m_font;
	ui->lbl_current_font->setText(m_font.family() + " [" + QString::number(m_font.pointSize()) + "pt]");

	ui->sb_cell_size->setValue(DEFAULT_CELL_SIZE);
	ui->sb_cell_padding->setValue(0);
}

void OptionsMenu::show_font_chooser() {
	bool ok;
	QFont tmp = QFontDialog::getFont(&ok, m_font, this, tr("Font used in main table"));
	if (ok) {
		ui->lbl_current_font->setText(tmp.family() + " [" + QString::number(tmp.pointSize()) + "pt]");
		m_dirty_font = tmp;
	}
}
