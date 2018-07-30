/*
Dwarf Therapist
Copyright (c) 2018 Clement Vuchener

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

#include "needcolumn.h"
#include "columntypes.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "dtstandarditem.h"
#include "gamedatareader.h"
#include "unitneed.h"

#include <QSettings>

NeedColumn::NeedColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s, set, parent)
    , m_need_id(s.value("need_id", -1).toInt())
{
    m_sortable_types << CST_LEVEL << CST_FOCUS;
    init_states();
    refresh_color_map();
}

NeedColumn::NeedColumn(QString title, int need_id, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title, CT_NEED, set, parent)
    , m_need_id(need_id)
{
    m_sortable_types << CST_LEVEL << CST_FOCUS;
    init_states();
    refresh_color_map();
}

NeedColumn::NeedColumn(const NeedColumn &to_copy)
    : ViewColumn(to_copy)
    , m_need_id(to_copy.m_need_id)
{
}

QStandardItem *NeedColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);

    auto needs = d->get_needs(m_need_id);

    int need_level = 0;
    int min_focus_level = std::numeric_limits<int>::max();
    int min_focus_degree = -1;
    QStringList need_desc;
    QString min_adjective;
    for (auto it = needs.first; it != needs.second; ++it) {
        need_level += it->second->need_level();
        if (it->second->focus_level() < min_focus_level) {
            min_focus_level = it->second->focus_level();
            min_focus_degree = it->second->focus_degree();
            min_adjective = it->second->adjective();
        }
        need_desc.append(tr("<span style=\"color: %1\">%2</span> (Focus: %3, Level: %4)")
                .arg(UnitNeed::degree_color(it->second->focus_degree(), true).name())
                .arg(it->second->description())
                .arg(it->second->focus_level())
                .arg(it->second->need_level()));
    }
    item->setData(CT_NEED, DwarfModel::DR_COL_TYPE);
    item->setData(need_level, DwarfModel::DR_RATING);
    item->setData(need_level, DwarfModel::DR_DISPLAY_RATING);
    switch (m_current_sort) {
    case CST_LEVEL:
        item->setData(need_level, DwarfModel::DR_SORT_VALUE);
        break;
    case CST_FOCUS:
    default:
        item->setData(min_focus_level, DwarfModel::DR_SORT_VALUE);
        break;
    }
    item->setData(min_focus_degree, DwarfModel::DR_STATE);

    QString tooltip = min_focus_degree == -1
            ? QString("<center><h3>%1</h3><h4>Not needed</h4></center>%4")
                    .arg(m_title)
                    .arg(tooltip_name_footer(d))
            : QString("<center><h3>%1</h3><h4>%2</h4></center><p>%3</p>%4")
                    .arg(m_title)
                    .arg(min_adjective)
                    .arg(need_desc.join("<br />"))
                    .arg(tooltip_name_footer(d));
    item->setToolTip(tooltip);

    return item;
}

QStandardItem *NeedColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) {
    QStandardItem *item = init_aggregate(group_name);

    int need_level = 0;
    int min_focus_level = std::numeric_limits<int>::max();
    int min_focus_degree = -1;
    QString min_adjective;
    const Dwarf *min_dwarf = nullptr;
    for (const auto *d: dwarves) {
        auto needs = d->get_needs(m_need_id);
        for (auto it = needs.first; it != needs.second; ++it) {
            need_level += it->second->need_level();
            if (it->second->focus_level() < min_focus_level) {
                min_focus_level = it->second->focus_level();
                min_focus_degree = it->second->focus_degree();
                min_adjective = it->second->adjective();
                min_dwarf = d;
            }
        }
    }
    item->setToolTip(min_dwarf
            ? tr("<center><h3>%1</h3>Lowest Focus in group:<br/><b>%2 (%3, %4)</b></center>")
                    .arg(m_title)
                    .arg(min_dwarf->nice_name())
                    .arg(min_adjective)
                    .arg(min_focus_level)
            : tr("<center><h3>%1</h3>Not needed in group</center>")
                    .arg(m_title));
    item->setData(min_focus_degree,DwarfModel::DR_STATE);
    return item;
}

void NeedColumn::refresh_sort(COLUMN_SORT_TYPE sType)
{
    m_current_sort = sType;
    for (auto it = m_cells.begin(); it != m_cells.end(); ++it)
        refresh_sort(it.key(), it.value());
}

void NeedColumn::init_states()
{
    m_available_states.clear();
    m_available_states << -1;
    for(int i = 0; i < DH_TOTAL_LEVELS; i++){
        m_available_states << static_cast<DWARF_HAPPINESS>(i);
    }
}

void NeedColumn::refresh_color_map()
{
}

QColor NeedColumn::get_state_color(int state) const
{
    if (state < 0)
        return Qt::transparent;
    else
        return DT->get_happiness_color(static_cast<DWARF_HAPPINESS>(state >= DH_TOTAL_LEVELS ? DH_TOTAL_LEVELS-1 : state));
}

void NeedColumn::refresh_sort(const Dwarf *d, QStandardItem *item)
{
    switch (m_current_sort) {
    case CST_LEVEL:
        item->setData(d->get_need_type_level(m_need_id), DwarfModel::DR_SORT_VALUE);
        break;
    case CST_FOCUS:
    default:
        item->setData(d->get_need_type_focus(m_need_id), DwarfModel::DR_SORT_VALUE);
        break;
    }
}

void NeedColumn::write_to_ini(QSettings &s)
{
    ViewColumn::write_to_ini(s);
    s.setValue("need_id", m_need_id);
}
