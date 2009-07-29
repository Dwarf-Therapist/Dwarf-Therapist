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
#include "columntypes.h"
#include "viewcolumnset.h"
#include "viewcolumn.h"

ViewColumnSet::ViewColumnSet(QString name, QObject *parent)
	: QObject(parent)
	, m_name(name)
{}
void ViewColumnSet::add_column(ViewColumn *col) {
	m_columns.insert(m_columns.uniqueKeys().size(), col);
}
	
void ViewColumnSet::clear_columns() {
	foreach(ViewColumn *col, m_columns) {
		col->deleteLater();
	}
	m_columns.clear();
}

ViewColumnSet *ViewColumnSet::from_file(QString filename, QObject *parent) {
	QSettings s(filename, QSettings::IniFormat);
	QString name = s.value("info/name", "UNKNOWN").toString();
	QColor bg_color = s.value("info/bg_color", Qt::white).value<QColor>();

	int cols = s.beginReadArray("columns");
	for (int i = 0; i < cols; ++i) {
		s.setArrayIndex(i);
		QString tmp_type = s.value("type", "DEFAULT").toString();
		QString name = s.value("name", "UNKNOWN " + QString::number(i)).toString();
		COLUMN_TYPE type = get_column_type(tmp_type);
	}
	s.endArray();


	ViewColumnSet *ret_val = new ViewColumnSet(name, parent);

	return ret_val;
}
	