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

ViewColumn::ViewColumn(QString title, ViewColumnSet *set, QObject *parent)
	: QObject(parent)
	, m_title(title)
	, m_set(set)
	, m_override_set_colors(false)
{
	if (set)
		set->add_column(this);
}

QStandardItem *ViewColumn::init_cell(Dwarf *d) {
	QStandardItem *item = new QStandardItem;
	item->setStatusTip(m_title + " :: " + d->nice_name());
	item->setData(set()->bg_color(), Qt::BackgroundColorRole);
	item->setData(set()->bg_color(), DwarfModel::DR_DEFAULT_BG_COLOR);
	item->setData(false, DwarfModel::DR_IS_AGGREGATE);
	item->setData(d->id(), DwarfModel::DR_ID);
	item->setData(0, DwarfModel::DR_DUMMY);
	return item;
}