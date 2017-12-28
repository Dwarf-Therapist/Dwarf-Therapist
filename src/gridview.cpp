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
#include "gridviewdialog.h"

#include <QSettings>
#include <QStandardItem>
#include <QStandardItemModel>

GridView::GridView(QString name, QObject *parent)
    : QObject(parent)
    , m_active(true)
    , m_name(name)
    , m_is_custom(true)
    , m_show_animals(false)
{}

GridView::GridView(const GridView &to_be_copied)
    : QObject(to_be_copied.parent())
    , m_active(to_be_copied.m_active)
    , m_name(to_be_copied.m_name)
    , m_is_custom(to_be_copied.m_is_custom)
    , m_show_animals(to_be_copied.m_show_animals)
{
    foreach(ViewColumnSet *s, to_be_copied.sets()) {
        add_set(new ViewColumnSet((const ViewColumnSet)*s));
    }
}

GridView::~GridView() {
    m_sets.clear();
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
}

void GridView::clear() {
    qDeleteAll(m_sets);
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

ViewColumn *GridView::get_column(const int idx){
    int min = 0;
    foreach(ViewColumnSet *set, m_sets){
        if(idx <= (min + set->columns().count())){
            return set->column_at(idx - min -1);
        }
        min += set->columns().count();
    }
    return 0;
}

void GridView::write_to_ini(QSettings &s) {
    s.setValue("name", m_name);
    s.setValue("active", m_active);
    s.setValue("animals", m_show_animals);
    s.beginWriteArray("sets", m_sets.size());
    int i = 0;
    for(int idx = 0; idx < m_sets.count(); idx++){
        s.setArrayIndex(i++);
        m_sets.at(idx)->write_to_ini(s,(idx==0 ? 1 : 0)); //exclude the first column of the first set (global sort)
    }
    s.endArray();
}

GridView *GridView::read_from_ini(QSettings &s, QObject *parent) {
    GridView *ret_val = new GridView("", parent);
    ret_val->set_name(s.value("name", "UNKNOWN").toString());
    ret_val->set_active(s.value("active", true).toBool());
    ret_val->set_show_animals(s.value("animals",false).toBool());
    //support old views before the animal flag was available
    if(!ret_val->show_animals() && ret_val->name().contains(tr("animal"),Qt::CaseInsensitive))
        ret_val->set_show_animals(true);

    int total_sets = s.beginReadArray("sets");
    for (int i = 0; i < total_sets; ++i) {
        s.setArrayIndex(i);
        ViewColumnSet *set = new ViewColumnSet(s, parent, i);
        if (set)
            ret_val->add_set(set);
    }
    s.endArray();

    return ret_val;
}

void GridView::reorder_sets(const QStandardItemModel &model) {
    QList<ViewColumnSet*> new_sets;
    for (int i = 0; i < model.rowCount(); ++i) {
        new_sets << model.item(i,0)->data().value<ViewColumnSet *>();
    }
    Q_ASSERT(new_sets.size() == m_sets.size());

    m_sets = std::move(new_sets);
}
