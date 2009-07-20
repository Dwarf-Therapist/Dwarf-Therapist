#include <QtGui>

#include "optionsmenu.h"
#include "customcolor.h"

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
		connect(cc, SIGNAL(colorChanged(const QColor &)), this, SLOT(color_changed(const QColor &)));
	}
	ui->group_general_colors->setLayout(main_layout);

	connect(ui->btn_restore_defaults, SIGNAL(pressed()), this, SLOT(restore_defaults()));
}

OptionsMenu::~OptionsMenu() {}

void OptionsMenu::read_settings(QSettings *s) {
	m_reading_settings = true;
	s->beginGroup("options");
	s->beginGroup("colors");
	QColor c;
	foreach(CustomColor *cc, m_general_colors) {
		c = s->value(cc->get_config_key(), cc->get_default()).value<QColor>();
		cc->set_color(c);
		emit color_changed(cc->get_config_key(), c);
	}
	s->endGroup();
	s->endGroup();
	m_reading_settings = false;
}

void OptionsMenu::write_settings(QSettings *s) {
	if (s && !m_reading_settings) {
		s->beginGroup("options");
		s->beginGroup("colors");
		foreach(CustomColor *cc, m_general_colors) {
			s->setValue(cc->get_config_key(), cc->get_color());
		}
		s->endGroup();
		s->endGroup();
	}
}

void OptionsMenu::accept() {
	foreach(CustomColor *cc, m_general_colors) {
		if (cc->is_dirty())
			emit color_changed(cc->get_config_key(), cc->get_color());
	}
	QDialog::accept();
}

void OptionsMenu::reject() {
	foreach(CustomColor *cc, m_general_colors) {
		if (cc->is_dirty())
			cc->reset_to_last();
	}
	QDialog::reject();
}

void OptionsMenu::restore_defaults() {
	foreach(CustomColor *cc, m_general_colors) {
		cc->reset_to_default();
		emit color_changed(cc->get_config_key(), cc->get_color());
	}
}