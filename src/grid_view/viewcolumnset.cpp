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
#include "viewmanager.h"
#include "viewcolumnset.h"
#include "laborcolumn.h"
#include "happinesscolumn.h"
#include "spacercolumn.h"
#include "skillcolumn.h"
#include "idlecolumn.h"
#include "traitcolumn.h"
#include "attributecolumn.h"
#include "gamedatareader.h"
#include "defines.h"
#include "labor.h"
#include "utils.h"
#include "dwarftherapist.h"
#include "dwarf.h"
#include "mainwindow.h"
#include "dwarfmodel.h"
#include "gridviewdialog.h"

ViewColumnSet::ViewColumnSet(QString name, QObject *parent)
	: QObject(parent)
	, m_name(name)
	, m_bg_color(Qt::white)
{}

ViewColumnSet::ViewColumnSet(const ViewColumnSet &copy) 
	: QObject(copy.parent())
	, m_name(copy.m_name)
	, m_bg_color(copy.m_bg_color)
{
	foreach(ViewColumn *vc, copy.m_columns) {
        ViewColumn *new_c = vc->clone();
        new_c->setParent(this);
        new_c->set_viewcolumnset(this);
        add_column(new_c); // manually add a cloned copy
	}
}

void ViewColumnSet::re_parent(QObject *parent) {
    setParent(parent);
    foreach(ViewColumn *vc, m_columns) {
        vc->setParent(parent);
    }
}

void ViewColumnSet::set_name(const QString &name) {
	m_name = name;
}

void ViewColumnSet::add_column(ViewColumn *col) {
	m_columns << col;
}
	
void ViewColumnSet::clear_columns() {
	foreach(ViewColumn *col, m_columns) {
		col->deleteLater();
	}
	m_columns.clear();
}

void ViewColumnSet::toggle_for_dwarf_group() {
	QAction *a = qobject_cast<QAction*>(QObject::sender());
	QString group_name = a->data().toString();
	DwarfModel *dm = DT->get_main_window()->get_model();

	TRACE << "toggling set:" << name() << "for group:" << group_name;

	int total_enabled = 0;
	int total_labors = 0;
	foreach(Dwarf *d, dm->get_dwarf_groups()->value(group_name)) {
		foreach(ViewColumn *vc, m_columns) {
			if (vc->type() == CT_LABOR) {
				total_labors++;
				LaborColumn *lc = static_cast<LaborColumn*>(vc);
				if (d->is_labor_enabled(lc->labor_id()))
					total_enabled++;
			}
		}
	}
	bool turn_on = total_enabled < total_labors;
	foreach(Dwarf *d, dm->get_dwarf_groups()->value(group_name)) {
		foreach(ViewColumn *vc, m_columns) {
			if (vc->type() == CT_LABOR) {
				LaborColumn *lc = static_cast<LaborColumn*>(vc);
				d->set_labor(lc->labor_id(), turn_on);
			}
		}
	}
	DT->get_main_window()->get_model()->calculate_pending();
}

void ViewColumnSet::toggle_for_dwarf() {
	// find out which dwarf this is for...
	QAction *a = qobject_cast<QAction*>(QObject::sender());
	int dwarf_id = a->data().toInt();
	Dwarf *d = DT->get_dwarf_by_id(dwarf_id);
	toggle_for_dwarf(d);
}

void ViewColumnSet::toggle_for_dwarf(Dwarf *d) {
	TRACE << "toggling set:" << name() << "for" << d->nice_name();
	int total_enabled = 0;
	int total_labors = 0;
	foreach(ViewColumn *vc, m_columns) {
		if (vc->type() == CT_LABOR) {
			total_labors++;
			LaborColumn *lc = static_cast<LaborColumn*>(vc);
			if (d->is_labor_enabled(lc->labor_id()))
				total_enabled++;
		}
	}
	bool turn_on = total_enabled < total_labors;
	foreach(ViewColumn *vc, m_columns) {
		if (vc->type() == CT_LABOR) {
			LaborColumn *lc = static_cast<LaborColumn*>(vc);
			d->set_labor(lc->labor_id(), turn_on);
		}
	}
	DT->get_main_window()->get_model()->calculate_pending();
}


void ViewColumnSet::reorder_columns(const QStandardItemModel &model) {
	QList<ViewColumn*> new_cols;
	for (int i = 0; i < model.rowCount(); ++i) {
		// find the VC that matches this item in the GUI list
		QStandardItem *item = model.item(i, 0);
		QString title = item->data(GridViewDialog::GPDT_TITLE).toString();
		COLUMN_TYPE type = static_cast<COLUMN_TYPE>(item->data(GridViewDialog::GPDT_COLUMN_TYPE).toInt());
		foreach(ViewColumn *vc, m_columns) {
			if (vc->title() == title && vc->type() == type) {
				new_cols << vc;
			}
		}
	}
	Q_ASSERT(new_cols.size() == m_columns.size());

	m_columns.clear();
	foreach(ViewColumn *vc, new_cols) {
		m_columns << vc;
	}
}

void ViewColumnSet::write_to_ini(QSettings &s) {
	s.setValue("name", m_name);
	s.setValue("bg_color", to_hex(m_bg_color));
	s.beginWriteArray("columns", m_columns.size());
	int i = 0;
	foreach(ViewColumn *vc, m_columns) {
		s.setArrayIndex(i++);
		vc->write_to_ini(s);
	}
	s.endArray();
}

ViewColumnSet *ViewColumnSet::read_from_ini(QSettings &s, QObject *parent) {
	ViewColumnSet *ret_val = new ViewColumnSet(s.value("name", "UNKNOWN").toString(), parent);
	QString color_in_hex = s.value("bg_color", "0xFFFFFF").toString();
	QColor bg_color = from_hex(color_in_hex);
	ret_val->set_bg_color(bg_color);
	
	int total_columns = s.beginReadArray("columns");
	for (int i = 0; i < total_columns; ++i) {
		s.setArrayIndex(i);
		switch(get_column_type(s.value("type", "DEFAULT").toString())) {
			case CT_SPACER:
				new SpacerColumn(s, ret_val, parent);
                break;
            case CT_HAPPINESS:
                new HappinessColumn(s.value("name", "UNKNOWN").toString(), ret_val, parent);
                break;
            case CT_LABOR:
                new LaborColumn(s, ret_val, parent);
                break;
            case CT_SKILL:
                new SkillColumn(s, ret_val, parent);
                break;
            case CT_IDLE:
                new IdleColumn(s.value("name", "UNKNOWN").toString(), ret_val, parent);
                break;
            case CT_TRAIT:
                new TraitColumn(s, ret_val, parent);
                break;
            case CT_ATTRIBUTE:
                new AttributeColumn(s, ret_val, parent);
                break;
            
            case CT_DEFAULT:
            default:
                LOGW << "unidentified column type in set" << ret_val->name() << "!";
                break;
		}
	}
	s.endArray();

	return ret_val;
}