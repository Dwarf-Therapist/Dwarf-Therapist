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
#include "gridviewdialog.h"
#include "utils.h"

GridView::GridView(QString name, QObject *parent)
	: QObject(parent)
	, m_active(true)
	, m_name(name)
{}

GridView::GridView(const GridView &to_be_copied)
	: QObject(to_be_copied.parent())
	, m_name(to_be_copied.m_name)
	, m_active(to_be_copied.m_active)
	, m_is_custom(to_be_copied.m_is_custom)
{
	foreach(ViewColumnSet *s, to_be_copied.sets()) {
		add_set(new ViewColumnSet((const ViewColumnSet)*s));
	}
}

GridView::~GridView() {
	foreach(ViewColumnSet *set, m_sets) {
		set->deleteLater();
	}
}

void GridView::re_parent(QObject *parent) {
    setParent(parent);
    foreach(ViewColumnSet *set, m_sets) {
        set->re_parent(this);
    }
}

void GridView::add_set(ViewColumnSet *set) {
	m_sets << set;
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
};

void GridView::clear() {
	m_sets.clear();
}

ViewColumnSet *GridView::get_set(const QString &name) {
	ViewColumnSet *ret_val = 0;
	foreach(ViewColumnSet *set, m_sets) {
		if (set->name() == name) {
			ret_val = set;
			break;
		}
	}
	return ret_val;
}

void GridView::write_to_ini(QSettings &s) {
	s.setValue("name", m_name);
	s.setValue("active", m_active);
	s.beginWriteArray("sets", m_sets.size());
	int i = 0;
	foreach(ViewColumnSet *set, m_sets) {
		s.setArrayIndex(i++);
		set->write_to_ini(s);
	}
	s.endArray();
}

GridView *GridView::read_from_ini(QSettings &s, QObject *parent) {
	GridView *ret_val = new GridView("", parent);
	ret_val->set_name(s.value("name", "UNKNOWN").toString());
	ret_val->set_active(s.value("active", true).toBool());
	
	int total_sets = s.beginReadArray("sets");
	for (int i = 0; i < total_sets; ++i) {
		s.setArrayIndex(i);
		ViewColumnSet *set = ViewColumnSet::read_from_ini(s, parent);
		if (set)
			ret_val->add_set(set);
	}
	s.endArray();

	return ret_val;
}

void GridView::reorder_sets(const QStandardItemModel &model) {
	QList<ViewColumnSet*> new_sets;
	for (int i = 0; i < model.rowCount(); ++i) {
		// find the set that matches this item in the GUI list
		QStandardItem *item = model.item(i, 0);
		QString name = item->data(GridViewDialog::GPDT_TITLE).toString();
		foreach(ViewColumnSet *set, m_sets) {
			if (set->name() == name) {
				new_sets << set;
			}
		}
	}
	Q_ASSERT(new_sets.size() == m_sets.size());

	m_sets.clear();
	foreach(ViewColumnSet *set, new_sets) {
		m_sets << set;
	}
}
