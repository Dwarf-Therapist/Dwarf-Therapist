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
#include "laborcolumn.h"
#include "happinesscolumn.h"
#include "spacercolumn.h"
#include "skillcolumn.h"
#include "gamedatareader.h"
#include "defines.h"

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
	QString set_name = s.value("info/name", "UNKNOWN").toString();
	QString color_in_hex = s.value("info/bg_color", "0xFFFFFF").toString();

	bool ok;
	QColor bg_color(color_in_hex.toInt(&ok, 16));
	if (!ok)
		bg_color = Qt::white;

	ViewColumnSet *ret_val = new ViewColumnSet(set_name, parent);
	ret_val->set_bg_color(bg_color);

	int cols = s.beginReadArray("columns");
	for (int i = 0; i < cols; ++i) {
		s.setArrayIndex(i);
		QString tmp_type = s.value("type", "DEFAULT").toString();
		QString col_name = s.value("name", "UNKNOWN " + QString::number(i)).toString();
		COLUMN_TYPE type = get_column_type(tmp_type);
		switch (type) {
			case CT_SKILL:
				{
					int skill_id = s.value("skill_id", -1).toInt();
					//TODO: check that labor and skill are known ids
					new SkillColumn(col_name, skill_id, ret_val, ret_val);
				}
				break;
			case CT_LABOR:
				{
					int labor_id = s.value("labor_id", -1).toInt();
					int skill_id = s.value("skill_id", -1).toInt();
					//TODO: check that labor and skill are known ids
					new LaborColumn(col_name, labor_id, skill_id, ret_val, ret_val);
				}
				break;
			case CT_HAPPINESS:
				new HappinessColumn(col_name, ret_val, ret_val);
				break;
			case CT_SPACER:
				{
					int width = s.value("width", 4).toInt();
					QString hex_color = s.value("bg_color").toString();
					bool ok;
					QColor bg_color(hex_color.toInt(&ok, 16));
					if (!ok)
						bg_color = Qt::gray;
					SpacerColumn *c = new SpacerColumn(col_name, ret_val, ret_val);
					c->set_override_color(true);
					c->set_bg_color(bg_color);
					c->set_width(width);
				}
				break;
			default:
				LOGW << "Column " << col_name << "in set" << set_name << "has unknown type: " << tmp_type;
				break;
		}
	}
	s.endArray();

	return ret_val;
}
	