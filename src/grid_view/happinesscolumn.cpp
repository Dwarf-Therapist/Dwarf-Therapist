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

#include "happinesscolumn.h"
#include "columntypes.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "defines.h"
#include "dtstandarditem.h"

HappinessColumn::HappinessColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s, set, parent)
    , m_colors(QMap<DWARF_HAPPINESS, QColor>())
{
    read_settings();
}

HappinessColumn::HappinessColumn(QString title, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title, CT_HAPPINESS, set, parent)
    , m_colors(QMap<DWARF_HAPPINESS, QColor>())
{
    read_settings();
}

HappinessColumn::HappinessColumn(const HappinessColumn &to_copy)
    : ViewColumn(to_copy)
    , m_colors(to_copy.m_colors)
{
}

QStandardItem *HappinessColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);

    QString pixmap_name = QString(":happiness/%1.png").arg(Dwarf::happiness_name(d->get_happiness()));
    if(pixmap_name=="")
        pixmap_name = ":img/question-frame.png";

    item->setData(QIcon(pixmap_name), Qt::DecorationRole);
    item->setData(CT_HAPPINESS, DwarfModel::DR_COL_TYPE);
    item->setData(d->get_raw_happiness(), DwarfModel::DR_SORT_VALUE);

    QString tooltip = QString("<center><h3>%1</h3><h4>%2<br/>%3</h4></center><p>%4</p>%5")
            .arg(m_title)
            .arg(Dwarf::happiness_name(d->get_happiness()))
            .arg(tr("Stress Level: ") + formatNumber(d->get_raw_happiness()))
            .arg(d->get_thought_desc())
            .arg(tooltip_name_footer(d));

    item->setToolTip(tooltip);
    QColor bg = m_colors[d->get_happiness()];
//    item->setBackground(QBrush(bg));
    item->setData(bg,Qt::BackgroundColorRole);

    return item;
}

QStandardItem *HappinessColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) {
    QStandardItem *item = init_aggregate(group_name);
    // find lowest happiness of all dwarfs this set represents, and show that color (so low happiness still pops out in a big group)
    DWARF_HAPPINESS lowest = DH_ECSTATIC;
    QString lowest_dwarf = "Nobody";
    foreach(Dwarf *d, dwarves) {
        DWARF_HAPPINESS tmp = d->get_happiness();
        if (tmp <= lowest) {
            lowest = tmp;
            lowest_dwarf = d->nice_name();
            if(lowest == DH_VERY_UNHAPPY)
                break;
        }
    }
    item->setToolTip(tr("<center><h3>%1</h3></center>Lowest Happiness in group: <b>%2: %3</b>")
        .arg(m_title)
        .arg(lowest_dwarf)
        .arg(Dwarf::happiness_name(lowest)));
    item->setData(m_colors[lowest], Qt::BackgroundColorRole);
//    item->setData(m_colors[lowest], DwarfModel::DR_DEFAULT_BG_COLOR);
    return item;
}

void HappinessColumn::read_settings() {
    QSettings *s = new QSettings(QSettings::IniFormat, QSettings::UserScope, COMPANY, PRODUCT, this);
    s->beginGroup("options/colors/happiness");
    foreach(QString k, s->childKeys()) {
        DWARF_HAPPINESS h = static_cast<DWARF_HAPPINESS>(k.toInt());
        m_colors[h] = s->value(k).value<QColor>();
    }
    s->endGroup();
}

void HappinessColumn::redraw_cells() {
    foreach(Dwarf *d, m_cells.uniqueKeys()) {
        if (d && m_cells[d] && m_cells[d]->model())
            m_cells[d]->setBackground(m_colors[d->get_happiness()]);
    }
}
