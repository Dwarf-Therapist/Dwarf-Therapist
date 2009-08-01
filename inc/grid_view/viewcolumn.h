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
#ifndef VIEW_COLUMN_H
#define VIEW_COLUMN_H

#include <QtGui>
#include "columntypes.h"

class ViewColumnSet;
class Dwarf;

/*!
ViewColumn
TODO: maybe subclass these for different accessors
I can think of a need for:
* Labor cols
* status cols (happiness, wounds, etc...)
* radio cols (choose which weapon skill, or armor type)
* simple counters (num kills, num kids, total gold, total items)
*/
class ViewColumn : public QObject {
	Q_OBJECT
public:
	ViewColumn(QString title, COLUMN_TYPE type, ViewColumnSet *set = 0, QObject *parent = 0);

	QString title() {return m_title;}
	void set_title(QString title) {m_title = title;}
	bool override_color() {return m_override_set_colors;}
	void set_override_color(bool yesno) {m_override_set_colors = yesno;}
	QColor bg_color() {return m_bg_color;}
	void set_bg_color(QColor c) {m_bg_color = c;}
	ViewColumnSet *set() {return m_set;}
	virtual COLUMN_TYPE type() {return m_type;}

	QStandardItem *init_cell(Dwarf *d);
	virtual QStandardItem *build_cell(Dwarf *d) = 0; // create a suitable item based on a dwarf
	virtual QStandardItem *build_aggregate(const QString &group_name, 
										   const QVector<Dwarf*> &dwarves) = 0; // create an aggregate cell based on several dwarves

protected:
	QString m_title;
	QColor m_bg_color;
	bool m_override_set_colors;
	ViewColumnSet *m_set;
	COLUMN_TYPE m_type;
};

#endif
