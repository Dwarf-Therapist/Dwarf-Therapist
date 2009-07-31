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

GridView::GridView(QString name, QObject *parent)
	: QObject(parent)
	, m_name(name)
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

GridView *GridView::from_file(const QString &filepath, const QDir &sets_dir, QObject *parent) {
	QSettings s(filepath, QSettings::IniFormat);
	QString name = s.value("info/name", "UNKNOWN").toString();
	
	GridView *ret_val = new GridView(name, parent);

	QMap<QString, ViewColumnSet*> tmp_sets;
	foreach(QString filename, sets_dir.entryList()) {
		if (filename.endsWith(".ini")) {
			ViewColumnSet *set = ViewColumnSet::from_file(sets_dir.filePath(filename), ret_val);
			if (set)
				tmp_sets.insert(set->name(), set);
		}
	}

	int total_sets = s.beginReadArray("sets");
	for (int i = 0; i < total_sets; ++i) {
		s.setArrayIndex(i);
		QString name = s.value("name").toString();
		if (tmp_sets.contains(name)) {
			ret_val->add_set(tmp_sets.value(name));
		}
	}
	s.endArray();

	return ret_val;
}