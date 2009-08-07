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

#include "happinesscolumn.h"
#include "columntypes.h"
#include "viewcolumnset.h"
#include "dwarfmodel.h"
#include "dwarf.h"

HappinessColumn::HappinessColumn(QString title, ViewColumnSet *set, QObject *parent) 
	: ViewColumn(title, CT_HAPPINESS, set, parent)
{}

QStandardItem *HappinessColumn::build_cell(Dwarf *d) {
	QStandardItem *item = init_cell(d);
	
	item->setData(CT_HAPPINESS, DwarfModel::DR_COL_TYPE);
	item->setData(d->get_raw_happiness(), DwarfModel::DR_RATING);
	QString tooltip = "<h3>" + m_title + "</h3>"
		"\t" + QString::number(d->get_raw_happiness());
	tooltip += "\n<h4>" + d->nice_name() + "</h4>";
	item->setToolTip(tooltip);

	switch (d->get_happiness()) {
		case Dwarf::DH_MISERABLE:
			item->setIcon(QIcon(":img/exclamation.png"));
			break;
		case Dwarf::DH_UNHAPPY:
			item->setIcon(QIcon(":img/emoticon_unhappy.png"));
			break;
		case Dwarf::DH_FINE:
			item->setIcon(QIcon(":img/emoticon_smile.png"));
			break;
		case Dwarf::DH_CONTENT:
			item->setIcon(QIcon(":img/emoticon_smile.png"));
			break;
		case Dwarf::DH_HAPPY:
			item->setIcon(QIcon(":img/emoticon_grin.png"));
			break;
		case Dwarf::DH_ECSTATIC:
			item->setIcon(QIcon(":img/emoticon_happy.png"));
			break;
	}
	return item;
}

QStandardItem *HappinessColumn::build_aggregate(const QString &, const QVector<Dwarf*> &) {
	QStandardItem *item = new QStandardItem;
	QColor bg;
	if (m_override_set_colors)
		bg = m_bg_color;
	else
		bg = m_set->bg_color();
	item->setData(bg, Qt::BackgroundColorRole);
	item->setData(bg, DwarfModel::DR_DEFAULT_BG_COLOR);
	return item;
}