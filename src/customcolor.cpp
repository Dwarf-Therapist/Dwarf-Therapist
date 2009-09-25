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
#include "qtcolorpicker.h"

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
	m_label->setStatusTip(m_tooltip);
	m_picker->insertColor(default_color, tr("Default"));
	//m_picker->setToolTip(m_tooltip);
	m_picker->setStatusTip(m_tooltip);
	m_picker->setStandardColors();
	m_picker->setCurrentColor(default_color);
	m_picker->setStyleSheet("text-align: left;");
	m_last_color = m_picker->currentColor();

	QHBoxLayout *hbox = new QHBoxLayout(this);
	hbox->setSpacing(2);
	hbox->setMargin(0);
	hbox->addWidget(m_picker);
	hbox->addWidget(m_label);
	this->setLayout(hbox);
	
	connect(m_picker, SIGNAL(colorChanged(const QColor &)), this, SLOT(color_changed(const QColor &)));
}

void CustomColor::color_changed(const QColor &) {
	m_dirty = true;
}
