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

#include <QtScript>

#include "dwarfmodelproxy.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "profession.h"
#include "defines.h"
#include "dwarftherapist.h"
#include "mainwindow.h"

DwarfModelProxy::DwarfModelProxy(QObject *parent)
    :QSortFilterProxyModel(parent)
    , m_engine(new QScriptEngine(this))
{}

DwarfModel* DwarfModelProxy::get_dwarf_model() const {
    return static_cast<DwarfModel*>(sourceModel());
}

void DwarfModelProxy::cell_activated(const QModelIndex &idx) {
    bool valid = idx.isValid();
    QModelIndex new_idx = mapToSource(idx);
    valid = new_idx.isValid();
    return get_dwarf_model()->cell_activated(new_idx);
}

void DwarfModelProxy::setFilterFixedString(const QString &pattern) {
    m_filter_text = pattern;
    QSortFilterProxyModel::setFilterFixedString(pattern);
}

void DwarfModelProxy::apply_script(const QString &script_body) {
    m_active_filter_script = script_body;
    invalidateFilter();
}

bool DwarfModelProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    bool matches = true;

    int dwarf_id = 0;
    QSettings *s = DT->user_settings();
    const DwarfModel *m = get_dwarf_model();
    if (m->current_grouping() == DwarfModel::GB_NOTHING) {
        QModelIndex idx = m->index(source_row, 0, source_parent);
        dwarf_id = m->data(idx, DwarfModel::DR_ID).toInt();
        QString data = m->data(idx, filterRole()).toString();
        if (!m_filter_text.isEmpty())
            matches = matches && data.contains(m_filter_text, Qt::CaseInsensitive);
    } else {
        QModelIndex tmp_idx = m->index(source_row, 0, source_parent);
        QStandardItem *item = m->itemFromIndex(tmp_idx);
        if (m->data(tmp_idx, DwarfModel::DR_IS_AGGREGATE).toBool()) {
            int matches = 0;
            for(int i = 0; i < item->rowCount(); ++i) {
                if (filterAcceptsRow(i, tmp_idx)) // a child matches
                    matches++;
            }
            matches = matches && matches > 0;
        } else {
            QModelIndex idx = m->index(source_row, 0, source_parent);
            dwarf_id = m->data(idx, DwarfModel::DR_ID).toInt();
            QString data = m->data(idx, filterRole()).toString();
            if (!m_filter_text.isEmpty())
                matches = matches && data.contains(m_filter_text, Qt::CaseInsensitive);
        }
    }

    if (dwarf_id && !m_active_filter_script.isEmpty()) {
        Dwarf *d = m->get_dwarf_by_id(dwarf_id);
        if (d) {
            QScriptValue d_obj = m_engine->newQObject(d);
            m_engine->globalObject().setProperty("d", d_obj);
            matches = matches && m_engine->evaluate(m_active_filter_script).toBool();
        }
    }

    //filter children and babies if necessary
    bool hide_children = s->value("options/hide_children_and_babies",
                                       false).toBool();
    if(dwarf_id && hide_children) {
        Dwarf *d = m->get_dwarf_by_id(dwarf_id);

        short baby_id = -1;
        short child_id = -1;
        foreach(Profession *p, GameDataReader::ptr()->get_professions()) {
            if (p->name(true) == "Baby") {
                baby_id = p->id();
            }
            if (p->name(true) == "Child") {
                child_id = p->id();
            }
            if(baby_id > 0 && child_id > 0)
                break;
        }
        matches = (d->raw_profession() != baby_id &&
                   d->raw_profession() != child_id);
    }

    return matches;
}

bool DwarfModelProxy::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const {
    Q_UNUSED(source_column);
    Q_UNUSED(source_parent);
    return true;
}

void DwarfModelProxy::sort(int column, Qt::SortOrder order) {
    if (order == Qt::AscendingOrder)
        sort(column, DSR_NAME_ASC);
    else
        sort(column, DSR_NAME_DESC);
}

void DwarfModelProxy::sort(int column, DWARF_SORT_ROLE role) {
    Qt::SortOrder order;
    if (column == 0) {
        switch(role) {
            default:
            case DSR_NAME_ASC:
                order = Qt::AscendingOrder;
                setSortRole(DwarfModel::DR_SORT_VALUE);
                break;
            case DSR_NAME_DESC:
                order = Qt::DescendingOrder;
                setSortRole(DwarfModel::DR_SORT_VALUE);
                break;
            case DSR_ID_ASC:
                order = Qt::AscendingOrder;
                setSortRole(DwarfModel::DR_ID);
                break;
            case DSR_ID_DESC:
                order = Qt::DescendingOrder;
                setSortRole(DwarfModel::DR_ID);
                break;
            case DSR_GAME_ORDER:
                order = Qt::AscendingOrder;
                setSortRole(DwarfModel::DR_SORT_VALUE);
                break;
        }
    } else {
        switch(role) {
            default:
            case DSR_NAME_ASC:
            case DSR_ID_ASC:
                order = Qt::AscendingOrder;
                break;
            case DSR_NAME_DESC:
            case DSR_ID_DESC:
                order = Qt::DescendingOrder;
                break;
        }
        setSortRole(DwarfModel::DR_SORT_VALUE);
    }
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setSortLocaleAware(true);
    QSortFilterProxyModel::sort(column, order);
}
