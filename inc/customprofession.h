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
#ifndef CUSTOM_PROFESSION_H
#define CUSTOM_PROFESSION_H

#include <QtGui>
class QxtListWidgetItem;
class Dwarf;

namespace Ui
{
    class CustomProfessionEditor;
}

//! Manages custom professions independent of a fortress
/*!
CustomProfession objects hold all data needed to map a set of labors onto a
dwarf. A dwarf can either have a default-profession (determined by the game
based on the dwarf's highest skill) or a custom-profession. In DF, custom
professions don't actually have any real meaning other than semantic.

Using Dwarf Therapist, you can associate any number of labors with a custom 
profession and save this association outside of the game. You can then
apply a custom profession to a dwarf with this tool, and it will set the 
appropriate labors on that dwarf.

Example:
*/
class CustomProfession : public QObject {
	Q_OBJECT
public:
	//! Constructor with blank labor template
	CustomProfession(QObject *parent = 0);
	//! Constructor with labor template based on Dwarf *d
	CustomProfession(Dwarf *d, QObject *parent = 0);

	//! Writes to disk for later use
	void save();

	//! Completely kills this profession
	void delete_from_disk();

	//! Get the game-visible name of this profession
	QString get_name() {return m_name;}

	//! Check if our template has a particular labor enabled
	bool is_active(int labor_id);

	//! Shows a small editing dialog for this profession
	int show_builder_dialog(QWidget *parent = 0);

	//! Returns a vector of all enabled labor_ids in this template
	QVector<int> get_enabled_labors();


	public slots:
		void add_labor(int labor_id) {set_labor(labor_id, true);}
		void remove_labor(int labor_id) {set_labor(labor_id, false);}
		void set_labor(int labor_id, bool active);
		void set_name(QString name) {m_name = name;}
		void accept();
		void cancel() {return;}
		void item_check_state_changed(QxtListWidgetItem*);

private:
	bool is_valid();
	Dwarf *m_dwarf;

	Ui::CustomProfessionEditor *ui;
	QString m_name;
	QMap<int, bool> m_active_labors;
	QDialog *m_dialog;

};
#endif