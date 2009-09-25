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
#include "dwarfdetailsdock.h"
#include "ui_dwarfdetailsdock.h"
#include "gamedatareader.h"
#include "dwarf.h"
#include "skill.h"
#include "labor.h"
#include "defines.h"
#include "dwarftherapist.h"
#include "utils.h"

DwarfDetailsDock::DwarfDetailsDock(QWidget *parent, Qt::WindowFlags flags)
	: QDockWidget(parent, flags)
	, ui(new Ui::DwarfDetailsDock)
	, m_skills_layout(new QGridLayout(this))
{
	ui->setupUi(this);
	setFeatures(QDockWidget::AllDockWidgetFeatures);
	setAllowedAreas(Qt::AllDockWidgetAreas);
}

void DwarfDetailsDock::show_dwarf(Dwarf *d) {
	//LOGD << "ABOUT TO DRAW" << d->nice_name();
	// Draw the name/profession text labels...
	ui->lbl_dwarf_name->setText(d->nice_name());
	ui->lbl_translated_name->setText(QString("(%1)").arg(d->translated_name()));
	ui->lbl_profession->setText(d->profession());

	QMap<QProgressBar*, int> things;
	int str = d->strength();
	int agi = d->agility();
	int tou = d->toughness();
	things.insert(ui->pb_strength, str);
	things.insert(ui->pb_agility, agi);
	things.insert(ui->pb_toughness, tou);
	
	foreach(QProgressBar *pb, things.uniqueKeys()) {
		int stat = things.value(pb);
		pb->setMaximum(stat > 5 ? stat : 5);
		pb->setValue(stat);
	}
	GameDataReader *gdr = GameDataReader::ptr();
	int xp_for_current = gdr->get_xp_for_next_attribute_level(str + agi + tou - 1);
	int xp_for_next = gdr->get_xp_for_next_attribute_level(str + agi + tou);
	if (xp_for_current && xp_for_next) {// is 0 when we don't know when the next level is...
		ui->pb_next_attribute_gain->setRange(xp_for_current, xp_for_next);
		ui->pb_next_attribute_gain->setValue(d->total_xp());
		ui->pb_next_attribute_gain->setToolTip(QString("%L1xp / %L2xp").arg(d->total_xp()).arg(xp_for_next));
	} else {
		ui->pb_next_attribute_gain->setValue(-1); //turn it off
		ui->pb_next_attribute_gain->setToolTip("Off the charts! (I have no idea when you'll get the next gain)");
	}
	
	
	Dwarf::DWARF_HAPPINESS happiness = d->get_happiness();
	ui->lbl_happiness->setText(QString("<b>%1</b> (%2)").arg(d->happiness_name(happiness)).arg(d->get_raw_happiness()));
	QColor color = DT->user_settings()->value(
		QString("options/colors/happiness/%1").arg(static_cast<int>(happiness))).value<QColor>();
	QString style_sheet_color = QString("#%1%2%3")
									.arg(color.red(), 2, 16, QChar('0'))
									.arg(color.green(), 2, 16, QChar('0'))
									.arg(color.blue(), 2, 16, QChar('0'));
	ui->lbl_happiness->setStyleSheet(QString("background-color: %1;").arg(style_sheet_color));
	
	foreach(QObject *obj, m_cleanup_list) {
		obj->deleteLater();
	}
	m_cleanup_list.clear();

	QVector<Skill> *skills = d->get_skills();
	QTableWidget *tw = new QTableWidget(skills->size(), 3, this);
	ui->vbox_main->insertWidget(4, tw, 10);
	m_cleanup_list << tw;
	tw->setEditTriggers(QTableWidget::NoEditTriggers);
	tw->setGridStyle(Qt::NoPen);
	tw->setAlternatingRowColors(true);
	tw->setHorizontalHeaderLabels(QStringList() << "Skill" << "Level" << "Progress");
	tw->verticalHeader()->hide();
	tw->horizontalHeader()->setStretchLastSection(true);
	tw->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
	tw->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
	tw->setSortingEnabled(false); // no sorting while we're inserting
	for (int row = 0; row < skills->size(); ++row) {
		tw->setRowHeight(row, 18);
		Skill s = skills->at(row);
		QTableWidgetItem *text = new QTableWidgetItem(s.name());
		QTableWidgetItem *level = new QTableWidgetItem;
		level->setData(0, d->get_rating_by_skill(s.id()));

		
		QProgressBar *pb = new QProgressBar(tw);
		pb->setRange(s.exp_for_current_level(), s.exp_for_next_level());
		pb->setValue(s.actual_exp());
		pb->setToolTip(s.exp_summary());
		pb->setDisabled(true);// this is to keep them from animating and looking all goofy

		tw->setItem(row, 0, text);
		tw->setItem(row, 1, level);
		tw->setCellWidget(row, 2, pb);
	}
	tw->setSortingEnabled(true); // no sorting while we're inserting
	tw->sortItems(1, Qt::DescendingOrder); // order by level descending
}