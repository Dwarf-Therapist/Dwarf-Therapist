#include <QtGui>

#include "optionsmenu.h"

QColor OptionsMenu::default_color_cursor = QColor(0xFF00FF);
QColor OptionsMenu::default_color_active_labor = QColor(0xE0FFE0);
QColor OptionsMenu::default_color_active_group = QColor(0x33FF33);
QColor OptionsMenu::default_color_dirty_border = QColor(0xFF6600);

OptionsMenu::OptionsMenu(MainWindow *parent)
	: QDialog(parent)
	, ui(new Ui::OptionsMenu)
	, m_reading_settings(false)
{
	ui->setupUi(this);

	QList<QtColorPicker*> pickers;
	pickers << ui->clr_active_group << ui->clr_active_labor << ui->clr_cursor << ui->clr_dirty_border;

	foreach(QtColorPicker* p, pickers) {
		p->setStandardColors();
		connect(p, SIGNAL(colorChanged(const QColor &)), this, SLOT(color_changed(const QColor &)));
	}
	connect(ui->btn_restore_defaults, SIGNAL(pressed()), this, SLOT(restore_defaults()));
}

OptionsMenu::~OptionsMenu() {}

void OptionsMenu::read_settings(QSettings *s) {
	m_reading_settings = true;

	s->beginGroup("options");
	s->beginGroup("colors");
	QColor c;
	// cursor color
	c = s->value("cursor", default_color_cursor).value<QColor>();
	ui->clr_cursor->setCurrentColor(c);
	c = s->value("active_labor", default_color_active_labor).value<QColor>();
	ui->clr_active_labor->setCurrentColor(c);
	c = s->value("active_group", default_color_active_group).value<QColor>();
	ui->clr_active_group->setCurrentColor(c);
	c = s->value("dirty_border", default_color_dirty_border).value<QColor>();
	ui->clr_dirty_border->setCurrentColor(c);
	s->endGroup();
	s->endGroup();
	m_reading_settings = false;
}

void OptionsMenu::write_settings(QSettings *s) {
	if (s && !m_reading_settings) {
		s->beginGroup("options");
		s->beginGroup("colors");
		s->setValue("cursor", ui->clr_cursor->currentColor());
		s->setValue("active_labor", ui->clr_active_labor->currentColor());
		s->setValue("active_group", ui->clr_active_group->currentColor());
		s->setValue("dirty_border", ui->clr_dirty_border->currentColor());
		s->endGroup();
		s->endGroup();
	}
}

void OptionsMenu::color_changed(const QColor &c) {
	QtColorPicker *cp = dynamic_cast<QtColorPicker*>(QObject::sender());
	if (cp == ui->clr_cursor) {
		emit picker_changed(MainWindow::CC_CURSOR, c);
	} else if (cp == ui->clr_active_labor) {
		emit picker_changed(MainWindow::CC_ACTIVE_LABOR, c);
	} else if (cp == ui->clr_active_group) {
		emit picker_changed(MainWindow::CC_ACTIVE_GROUP, c);
	} else if (cp == ui->clr_dirty_border) {
		emit picker_changed(MainWindow::CC_DIRTY_BORDER, c);
	}
}

void OptionsMenu::restore_defaults() {
	ui->clr_cursor->setCurrentColor(default_color_cursor);
	ui->clr_active_labor->setCurrentColor(default_color_active_labor);
	ui->clr_active_group->setCurrentColor(default_color_active_group);
	ui->clr_dirty_border->setCurrentColor(default_color_dirty_border);
	emit picker_changed(MainWindow::CC_CURSOR, default_color_cursor);
	emit picker_changed(MainWindow::CC_ACTIVE_LABOR, default_color_active_labor);
	emit picker_changed(MainWindow::CC_ACTIVE_GROUP, default_color_active_group);
	emit picker_changed(MainWindow::CC_DIRTY_BORDER, default_color_dirty_border);
}