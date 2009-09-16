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

DwarfDetailsDock::DwarfDetailsDock(QWidget *parent, Qt::WindowFlags flags)
	: QDockWidget(parent, flags)
	, ui(new Ui::DwarfDetailsDock)
	, m_skills_layout(new QGridLayout(this))
{
	ui->setupUi(this);
	setFeatures(QDockWidget::AllDockWidgetFeatures);
	setAllowedAreas(Qt::AllDockWidgetAreas);
	ui->sa_contents->setLayout(m_skills_layout);
}

void DwarfDetailsDock::show_dwarf(Dwarf *d) {
	//LOGD << "ABOUT TO DRAW" << d->nice_name();
	ui->lbl_dwarf_name->setText(d->nice_name());
	ui->lbl_translated_name->setText(QString("(%1)").arg(d->translated_name()));
	ui->lbl_profession->setText(d->profession());
	
	int raw_happiness = d->get_raw_happiness();
	
	if (raw_happiness > 150) {
		ui->pb_happiness->setMaximum(raw_happiness);
	} else {
		ui->pb_happiness->setMaximum(150);
	}
	ui->pb_happiness->setValue(raw_happiness);

	foreach(QObject *obj, m_cleanup_list) {
		obj->deleteLater();
	}
	m_cleanup_list.clear();
	
	//GameDataReader *gdr = GameDataReader::ptr();
	QVector<Skill> *skills = d->get_skills();
	qSort(*skills); // this will reverse sort by level 0 to 20...
	int row = 0;
	for (int i = skills->size() - 1; i >= 0; --i) {
		Skill s = skills->at(i);
		QLabel *skill_name = new QLabel;
		QProgressBar *pb = new QProgressBar(this);

		m_skills_layout->addWidget(skill_name, row, 0);
		m_skills_layout->addWidget(pb, row++, 1);
		m_cleanup_list << skill_name << pb;

		LOGD << d->nice_name() << s.to_string();
		LOGD << d->nice_name() << s.exp() << "/" << s.exp_for_next_level() - s.exp_for_current_level() ;

		skill_name->setText(s.to_string(true, false));
		pb->reset();
		pb->setRange(s.exp_for_current_level(), s.exp_for_next_level());
		pb->setValue(s.actual_exp());
		pb->setToolTip(s.exp_summary());
	}
	

}