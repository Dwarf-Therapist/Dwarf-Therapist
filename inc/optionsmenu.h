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
#ifndef OPTIONS_MENU_H
#define OPTIONS_MENU_H

#include "ui_optionsmenu.h"
#include "uberdelegate.h"

class CustomColor;

class OptionsMenu : public QDialog {
	Q_OBJECT
public:
	OptionsMenu(QWidget *parent = 0);
	virtual ~OptionsMenu();

	void read_settings();
	void write_settings();

	bool event(QEvent *evt);

	public slots:
		void accept();
		void reject();
		void restore_defaults();
		void show_font_chooser();
		void set_skill_drawing_method(const UberDelegate::SKILL_DRAWING_METHOD&);
	
private:
	bool m_reading_settings;
	Ui::OptionsMenu *ui;
	QList<CustomColor*> m_general_colors;
	QList<CustomColor*> m_happiness_colors;
	QFont m_font;
	QFont m_dirty_font;

signals:
	void color_changed(const QString &, const QColor &);
	//! emitted when the options menu "ok" button is hit
	void settings_changed(); 
};
#endif
