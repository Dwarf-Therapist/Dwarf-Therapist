#include <QtGui>

#include "optionsmenu.h"

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
}

OptionsMenu::~OptionsMenu() {}

void OptionsMenu::read_settings(QSettings *s) {
	m_reading_settings = true;

	s->beginGroup("options");
	s->beginGroup("colors");
	QColor c;
	// cursor color
	c = s->value("cursor", QColor(0xFF00FF)).value<QColor>();
	ui->clr_cursor->setCurrentColor(c);
	c = s->value("active_labor", QColor(0xE0FFE0)).value<QColor>();
	ui->clr_active_labor->setCurrentColor(c);
	c = s->value("active_group", QColor(0x33FF33)).value<QColor>();
	ui->clr_active_group->setCurrentColor(c);
	c = s->value("dirty_border", QColor(0xFF6600)).value<QColor>();
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