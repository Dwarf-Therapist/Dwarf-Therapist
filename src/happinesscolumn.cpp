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
#include "dtstandarditem.h"
#include "gamedatareader.h"

HappinessColumn::HappinessColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s, set, parent)
{
    init_states();
    refresh_color_map();
}

HappinessColumn::HappinessColumn(QString title, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title, CT_HAPPINESS, set, parent)
{
    init_states();
    refresh_color_map();
}

HappinessColumn::HappinessColumn(const HappinessColumn &to_copy)
    : ViewColumn(to_copy)
{
}

QStandardItem *HappinessColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);

    QString pixmap_name = ":img/question-frame.png";
    QString stressed_mood_desc = "";
    if(d->in_stressed_mood()){
        stressed_mood_desc = tr("<br/>Mood: %1").arg(GameDataReader::ptr()->get_mood_desc(d->current_mood(),true));
        pixmap_name = QString(":img/exclamation-red-frame.png");
    }else{
        pixmap_name = QString(":happiness/%1.png").arg(Dwarf::happiness_name(d->get_happiness()));
    }

    item->setData(QIcon(pixmap_name), Qt::DecorationRole);
    item->setData(CT_HAPPINESS, DwarfModel::DR_COL_TYPE);
    item->setData(d->get_raw_happiness()*-1, DwarfModel::DR_SORT_VALUE); //invert sorting since low raw happiness (low stress) is better
    item->setData(d->get_happiness(),DwarfModel::DR_STATE);

    QString tooltip = QString("<center><h3>%1</h3><h4>%2<br/>%3%4</h4></center><p>%5</p>%6")
            .arg(m_title)
            .arg(Dwarf::happiness_name(d->get_happiness()))
            .arg(tr("Stress Level: ") + formatNumber(d->get_raw_happiness(),DT->format_SI()))
            .arg(stressed_mood_desc)
            .arg(d->get_emotions_desc())
            .arg(tooltip_name_footer(d));
    item->setToolTip(tooltip);

    return item;
}

QStandardItem *HappinessColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) {
    QStandardItem *item = init_aggregate(group_name);
    // find lowest happiness (highest stress) of all dwarfs this set represents, and show that color (so low happiness still pops out in a big group)
    Dwarf *d_stressed = 0;
    int stress = -1000000;
    foreach(Dwarf *d, dwarves) {
        int tmp = d->get_raw_happiness();
        if (tmp >= stress) {
            stress = tmp;
            d_stressed = d;
        }
    }
    QString name = tr("Nobody");
    DWARF_HAPPINESS highest = DH_FINE;
    if(d_stressed){
        name = d_stressed->nice_name();
        highest = d_stressed->get_happiness();
    }
    item->setToolTip(tr("<center><h3>%1</h3>Lowest Happiness in group:<br/><b>%2 (%3)</b><br/>%4</center>")
                     .arg(m_title)
                     .arg(name)
                     .arg(Dwarf::happiness_name(highest))
                     .arg(tr("Stress Level: ") + formatNumber(stress,DT->format_SI())));
    item->setData(highest,DwarfModel::DR_STATE);
    return item;
}

void HappinessColumn::redraw_cells() {
    foreach(Dwarf *d, m_cells.uniqueKeys()) {
        if (d && m_cells[d] && m_cells[d]->model())
            m_cells[d]->setBackground(DT->get_happiness_color(d->get_happiness()));
    }
}

void HappinessColumn::init_states(){
    m_available_states.clear();
    for(int i = 0; i < DH_TOTAL_LEVELS; i++){
        m_available_states << static_cast<DWARF_HAPPINESS>(i);
    }
}
void HappinessColumn::refresh_color_map(){
}

QColor HappinessColumn::get_state_color(int state) const{
    return DT->get_happiness_color(static_cast<DWARF_HAPPINESS>(state));
}
