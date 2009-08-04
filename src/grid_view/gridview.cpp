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
#include "gridview.h"
#include "viewcolumnset.h"
#include "viewmanager.h"

GridView::GridView(QString name, QObject *parent)
	: QObject(parent)
	, m_name(name)
	, m_active(true)
{}

void GridView::add_set(ViewColumnSet *set) {
	m_sets << set;
	m_set_map.insert(set->name(), set);
}

void GridView::remove_set(QString name) {
	ViewColumnSet *to_remove = 0;
	foreach(ViewColumnSet *s, m_sets) {
		if (name == s->name()) {
			to_remove = s;
			break;
		}
	}
	m_sets.removeOne(to_remove);
	m_set_map.remove(name);
};

void GridView::clear() {
	foreach(ViewColumnSet *set, m_sets) {
		set->deleteLater();
	}
	m_sets.clear();
	m_set_map.clear();
}

GridView *GridView::from_file(const QString &filepath, ViewManager *mgr, QObject *parent) {
	QSettings s(filepath, QSettings::IniFormat);
	QString name = s.value("info/name", "UNKNOWN").toString();
	bool active = s.value("info/active", true).toBool();
	
	GridView *ret_val = new GridView(name, parent);
	ret_val->set_filename(filepath);
	ret_val->set_active(active);

	int total_sets = s.beginReadArray("sets");
	for (int i = 0; i < total_sets; ++i) {
		s.setArrayIndex(i);
		QString name = s.value("name").toString();
		ViewColumnSet *set = mgr->get_set_by_name(name);
		if (set) {
			ret_val->add_set(set);
		}
	}
	s.endArray();

	return ret_val;
}

void GridView::write_settings() {
	if (m_filename.isEmpty())
		m_filename = m_name + ".ini";

	QSettings s(m_filename, QSettings::IniFormat);
	s.setValue("info/name", m_name);
	s.setValue("info/active", m_active);

	s.remove("sets");
	s.beginWriteArray("sets", sets().size());
	int i = 0;
	foreach(ViewColumnSet *set, sets()) {
		s.setArrayIndex(i++);
		s.setValue("name", set->name());
		set->write_settings();
	}
	s.endArray();
}