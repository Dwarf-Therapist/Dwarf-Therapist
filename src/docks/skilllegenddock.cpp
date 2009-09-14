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
#include "skilllegenddock.h"
#include "statetableview.h"
#include "gamedatareader.h"
#include "dwarfmodel.h"
#include "columntypes.h"
#include "rotatedheader.h"
#include "dwarftherapist.h"
#include "optionsmenu.h"

SkillLegendDock::SkillLegendDock(QWidget *parent, Qt::WindowFlags flags) 
	: QDockWidget(parent, flags)
{
	setObjectName("dock_skill_legend");
	setWindowTitle(tr("Skill Legend"));
	QWidget *main_widget = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(this);
	main_widget->setLayout(layout);
	
	StateTableView *stv = new StateTableView(this);
	stv->setContextMenuPolicy(Qt::ActionsContextMenu);

	QAction *a;
	for(int i = 0; i < UberDelegate::SDM_TOTAL_METHODS; ++i) {
		UberDelegate::SKILL_DRAWING_METHOD m = static_cast<UberDelegate::SKILL_DRAWING_METHOD>(i);
		a = new QAction(QString("Use %1 Method")
			.arg(UberDelegate::name_for_method(m)), stv);
		a->setData(m);
		connect(a, SIGNAL(triggered()), SLOT(set_SDM()));
		stv->addAction(a);
	}
	
	layout->addWidget(stv);
	setWidget(main_widget);

	QStandardItemModel *m = new QStandardItemModel(this);
	stv->setModel(m);
	QList<QStandardItem*> items;
	for(int i = 20; i >= 0; --i) {
		QList<QStandardItem*> sub_items;
		QStandardItem *name = new QStandardItem;
		name->setText(GameDataReader::ptr()->get_skill_level_name(i));
		QStandardItem *item = new QStandardItem;
		item->setData(i, DwarfModel::DR_RATING);
		item->setData(CT_SKILL, DwarfModel::DR_COL_TYPE);
		item->setData(Qt::white, DwarfModel::DR_DEFAULT_BG_COLOR);
		sub_items << name << item;
		m->appendRow(sub_items);
	}
	stv->setHeaderHidden(true);
	for(int i = 1; i < 16; ++i) {
		stv->get_header()->resizeSection(i, 16);
	}

	connect(this, SIGNAL(change_skill_drawing_method(const UberDelegate::SKILL_DRAWING_METHOD&)),
		DT->get_options_menu(), SLOT(set_skill_drawing_method(const UberDelegate::SKILL_DRAWING_METHOD&)));
}

void SkillLegendDock::set_SDM() {
	QAction *a = qobject_cast<QAction*>(QObject::sender());
	UberDelegate::SKILL_DRAWING_METHOD sdm = static_cast<UberDelegate::SKILL_DRAWING_METHOD>(a->data().toInt());
	emit change_skill_drawing_method(sdm);
}