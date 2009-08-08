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
#include "viewcolumnsetdialog.h"
#include "laborcolumn.h"
#include "happinesscolumn.h"
#include "spacercolumn.h"
#include "skillcolumn.h"
#include "gamedatareader.h"
#include "defines.h"
#include "labor.h"
#include "utils.h"

ViewColumnSet::ViewColumnSet(QString name, ViewManager *mgr, QObject *parent)
	: QObject(parent)
	, m_name(name)
	, m_manager(mgr)
	, m_bg_color(Qt::white)
{}

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

void ViewColumnSet::reset_from_disk() {
	QSettings s(m_filename, QSettings::IniFormat);
	QString set_name = s.value("info/name", "UNKNOWN").toString();
	QString color_in_hex = s.value("info/bg_color", "0xFFFFFF").toString();
	QColor bg_color = from_hex(color_in_hex);

	clear_columns();
	
	m_name = set_name;
	m_bg_color = bg_color;
	
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
					new SkillColumn(col_name, skill_id, this, this);
				}
				break;
			case CT_LABOR:
				{
					int labor_id = s.value("labor_id", -1).toInt();
					int skill_id = s.value("skill_id", -1).toInt();
					//TODO: check that labor and skill are known ids
					new LaborColumn(col_name, labor_id, skill_id, this, this);
				}
				break;
			case CT_HAPPINESS:
				new HappinessColumn(col_name, this, this);
				break;
			case CT_SPACER:
				{
					int width = s.value("width", DEFAULT_SPACER_WIDTH).toInt();
					QString hex_color = s.value("bg_color").toString();
					QColor bg_color = from_hex(hex_color);
					SpacerColumn *c = new SpacerColumn(col_name, this, this);
					c->set_override_color(s.value("override_color").toBool());
					if (c->override_color()) {
						c->set_bg_color(bg_color);
					}
					c->set_width(width);
				}
				break;
			default:
				LOGW << "Column " << col_name << "in set" << set_name << "has unknown type: " << tmp_type;
				break;
		}
	}
	s.endArray();
}

ViewColumnSet *ViewColumnSet::from_file(QString filename, ViewManager *mgr, QObject *parent) {
	ViewColumnSet *ret_val = new ViewColumnSet("", mgr, parent);
	ret_val->set_filename(filename);
	ret_val->reset_from_disk();
	return ret_val;
}

void ViewColumnSet::write_settings() {
	if (m_filename.isEmpty())
		m_filename = m_manager->set_path() + "/" + m_name + ".ini";

	QSettings s(m_filename, QSettings::IniFormat);
	s.setValue("info/name", m_name);
	s.setValue("info/bg_color", to_hex(m_bg_color));

	s.remove("columns");
	s.beginWriteArray("columns", columns().size());
	int i = 0;
	foreach(ViewColumn *vc, columns()) {
		s.setArrayIndex(i++);
		
		//cleanup
		s.remove("override_color");
		s.remove("bg_color");
		s.remove("width");
		s.remove("labor_id");
		s.remove("skill_id");

		if (!vc->title().isEmpty())
			s.setValue("name", vc->title());
		s.setValue("type", get_column_type(vc->type()));
		if (vc->override_color()) {
			s.setValue("override_color", true);
			s.setValue("bg_color", to_hex(vc->bg_color()));
		}
		switch (vc->type()) {
			case CT_SKILL:
				{
					SkillColumn *c = static_cast<SkillColumn*>(vc);
					s.setValue("skill_id", c->skill_id());
				}
				break;
			case CT_LABOR:
				{
					LaborColumn *c = static_cast<LaborColumn*>(vc);
					s.setValue("labor_id", c->labor_id());
					s.setValue("skill_id", c->skill_id());
				}
				break;
			case CT_SPACER:
				{
					SpacerColumn *c = static_cast<SpacerColumn*>(vc);
					if (c->width() > 0)
						s.setValue("width", c->width());
				}
				break;
			default:
				break;
		}
	}
	s.endArray();
}

void ViewColumnSet::delete_from_disk() {
	int answer = QMessageBox::question(
		0, tr("Really delete '%1' forever?").arg(m_name),
		tr("Deleting '%1' will permanently delete it from disk. Also if any views are currently using"
		   " this set, they may become corrupted. There is no undo!").arg(m_name),
		QMessageBox::Yes | QMessageBox::No);
	if (answer == QMessageBox::Yes) {
		LOGD << "permanently deleting set" << m_name;
		QFile f;
		if (f.remove(m_filename)) {
			emit set_deleted();
			deleteLater();
		}
	}
}

void ViewColumnSet::update_from_dialog(ViewColumnSetDialog *d) {
	m_name = d->name();
	m_bg_color = d->bg_color();
	clear_columns();
	foreach(ViewColumn *vc, d->columns()) {
		add_column(vc);
	}
}