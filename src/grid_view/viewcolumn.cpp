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
#include "viewcolumnset.h"
#include "viewcolumn.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "utils.h"

ViewColumn::ViewColumn(QString title, COLUMN_TYPE type, ViewColumnSet *set, QObject *parent)
	: QObject(parent)
	, m_title(title)
	, m_set(set)
	, m_override_set_colors(false)
	, m_type(type)
{
	if (set) {
		set->add_column(this);
		set_bg_color(set->bg_color());
	}	
}

ViewColumn::ViewColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
	: QObject(parent)
	, m_title(s.value("name", "UNKNOWN").toString())
	, m_set(set)
	, m_override_set_colors( s.value("override_color", false).toBool())
	, m_type(get_column_type(s.value("type", "DEFAULT").toString()))
{
	if (m_override_set_colors)
		m_bg_color = from_hex(s.value("bg_color", to_hex(set->bg_color())).toString());
}

QStandardItem *ViewColumn::init_cell(Dwarf *d) {
	QStandardItem *item = new QStandardItem;
	item->setStatusTip(QString("%1 :: %2").arg(m_title).arg(d->nice_name()));
	QColor bg;
	if (m_override_set_colors) {
		bg = m_bg_color;
	} else {
		bg = set()->bg_color();
	}
	item->setData(bg, Qt::BackgroundColorRole);
	item->setData(bg, DwarfModel::DR_DEFAULT_BG_COLOR);
	item->setData(false, DwarfModel::DR_IS_AGGREGATE);
	item->setData(d->id(), DwarfModel::DR_ID);
	item->setData(0, DwarfModel::DR_DUMMY);
	m_cells[d] = item;
	return item;
}

void ViewColumn::write_to_ini(QSettings &s) {
	if (!m_title.isEmpty())
		s.setValue("name", m_title);
	else
		s.setValue("name", "UNKNOWN");

	s.setValue("type", get_column_type(m_type));
	if (m_override_set_colors) {
		s.setValue("override_color", true);
		s.setValue("bg_color", to_hex(m_bg_color));
	}
}