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

class CustomProfession : public QObject {
	Q_OBJECT
public:
	CustomProfession(QObject *parent = 0);
	CustomProfession(Dwarf *d, QObject *parent = 0);

	void save();
	void delete_from_disk();
	QString get_name() {return m_name;}
	bool is_active(int labor_id);
	int show_builder_dialog(QWidget *parent = 0);
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