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
#ifndef CUSTOM_COLOR_H
#define CUSTOM_COLOR_H

#include <QtGui>
#include <qtcolorpicker.h>

class CustomColor : public QWidget {
	Q_OBJECT
public:
	CustomColor(QString setting_name, QString tooltip, QString config_key, QColor default_color, QWidget *parent = 0);

	void set_color(QColor new_color) {m_picker->setCurrentColor(new_color);}
	QColor get_color() {return m_picker->currentColor();}
	QColor get_default() {return m_default;}
	bool is_dirty() {return m_dirty;}
	void reset_to_default() {m_picker->setCurrentColor(m_default);}
	void reset_to_last() {m_picker->setCurrentColor(m_last_color);}
	QString get_config_key() {return m_config_key;}
	

private:
	QString m_name;
	QString m_tooltip;
	QString m_config_key;
	QtColorPicker *m_picker;
	QLabel *m_label;
	QColor m_default;
	QColor m_last_color;

	bool m_dirty;

	private slots:
		void color_changed(const QColor &);

signals:
	void color_changed(QString config_key, const QColor &new_color);
};

#endif;
