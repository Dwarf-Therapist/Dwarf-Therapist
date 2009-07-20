#include <QtGui>
#include <qtcolorpicker.h>

#include "customcolor.h"

CustomColor::CustomColor(QString setting_name, QString tooltip, QString config_key, QColor default_color, QWidget *parent)
	: QWidget(parent)
	, m_name(setting_name)
	, m_tooltip(tooltip)
	, m_config_key(config_key)
	, m_default(default_color)
	, m_last_color(default_color)
	, m_label(new QLabel(setting_name, this))
	, m_picker(new QtColorPicker(this, -1, true))
	, m_dirty(false)
{
	m_label->setBuddy(m_picker);
	m_picker->insertColor(default_color, tr("Default"));
	m_picker->setToolTip(m_tooltip);
	m_picker->setStandardColors();
	m_picker->setCurrentColor(default_color);

	QHBoxLayout *hbox = new QHBoxLayout(this);
	hbox->addWidget(m_picker);
	hbox->addWidget(m_label);
	this->setLayout(hbox);
	
	connect(m_picker, SIGNAL(colorChanged(const QColor &)), this, SLOT(color_changed(const QColor &)));
}

void CustomColor::color_changed(const QColor &) {
	m_dirty = true;
	m_last_color = m_picker->currentColor();
}
