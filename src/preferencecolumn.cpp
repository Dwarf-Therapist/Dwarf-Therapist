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

#include "preferencecolumn.h"
#include "dwarf.h"
#include "preference.h"

#include <QSettings>

PreferenceColumn::PreferenceColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s, set, parent)
{
    s.beginGroup("preference");
    bool updated;
    m_pref = RolePreference::parse(s, updated);
    if (updated) {
        LOGW << "Preference column" << title() << "from" << s.fileName() << "needs updating";
    }
    s.endGroup();
}

PreferenceColumn::PreferenceColumn(const QString &title, std::unique_ptr<RolePreference> &&pref, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title, CT_PREFERENCE, set, parent)
    , m_pref(std::move(pref))
{
}

PreferenceColumn::PreferenceColumn(const PreferenceColumn &to_copy)
    : ViewColumn(to_copy)
    , m_pref(to_copy.m_pref->copy())
{
}

static int matched_preferences(const RolePreference *rolepref, Dwarf *d, QStringList &pref_desc)
{
    int count = 0;
    auto range = d->get_preferences()->equal_range(rolepref->get_pref_category());
    for (auto it = range.first; it != range.second; ++it)
        if (rolepref->match(it->second.get(), d)) {
            ++count;
            pref_desc.append(it->second->get_description());
        }
    return count;
}

QStandardItem *PreferenceColumn::build_cell(Dwarf *d)
{
    QStandardItem *item = init_cell(d);
    item->setData(CT_PREFERENCE, DwarfModel::DR_COL_TYPE);

    QStringList matched_prefs;
    int count = matched_preferences(m_pref.get(), d, matched_prefs);

    item->setData(count, DwarfModel::DR_DISPLAY_RATING);
    item->setData(count > 0 ? 100 : 0, DwarfModel::DR_RATING);
    item->setData(count, DwarfModel::DR_SORT_VALUE);
    set_export_role(DwarfModel::DR_SORT_VALUE);

    item->setToolTip(QString("<center><h3 style=\"margin:0;\">%1</h3></center>%2")
            .arg(m_title)
            .arg(matched_prefs.isEmpty() ? tr("No preference") : matched_prefs.join(", ")));

    return item;
}

QStandardItem *PreferenceColumn::build_aggregate(const QString& group_name, const QVector<Dwarf *> &dwarves)
{
    QStandardItem *item = init_aggregate(group_name);
    item->setData(CT_PREFERENCE, DwarfModel::DR_COL_TYPE);

    QStringList matched_prefs;
    int count = 0;
    for (auto d: dwarves)
        count += matched_preferences(m_pref.get(), d, matched_prefs);

    item->setData(count, DwarfModel::DR_DISPLAY_RATING);
    item->setData(count > 0 ? 100 : 0, DwarfModel::DR_RATING);

    matched_prefs.removeDuplicates();
    item->setToolTip(QString("<center><h3 style=\"margin:0;\">%1</h3></center>%2")
            .arg(m_title)
            .arg(matched_prefs.isEmpty() ? tr("No preference") : matched_prefs.join(", ")));

    return item;
}

void PreferenceColumn::write_to_ini(QSettings &s)
{
    ViewColumn::write_to_ini(s);
    s.beginGroup("preference");
    m_pref->write(s);
    s.endGroup();
}

