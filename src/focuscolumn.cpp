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

#include "focuscolumn.h"
#include "columntypes.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "dtstandarditem.h"
#include "gamedatareader.h"

FocusColumn::FocusColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s, set, parent)
{
    init_states();
    refresh_color_map();
}

FocusColumn::FocusColumn(QString title, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title, CT_FOCUS, set, parent)
{
    init_states();
    refresh_color_map();
}

FocusColumn::FocusColumn(const FocusColumn &to_copy)
    : ViewColumn(to_copy)
{
}

QStandardItem *FocusColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);

    int current_focus = d->get_current_focus();
    int undistracted_focus = d->get_undistracted_focus();
    double ratio = (current_focus*100.0)/undistracted_focus;

    item->setData(CT_FOCUS, DwarfModel::DR_COL_TYPE);
    item->setData(ratio, DwarfModel::DR_RATING);
    item->setData(ratio, DwarfModel::DR_SORT_VALUE);
    item->setData(d->get_focus_degree(),DwarfModel::DR_STATE);

    QString tooltip = QString("<center><h3>%1</h3><h4>%2<br />%3% (%4/%5)</h3></center><p>%6</p>%7")
            .arg(m_title)
            .arg(d->get_focus_adjective())
            .arg(ratio)
            .arg(current_focus)
            .arg(undistracted_focus)
            .arg(d->get_focus_desc())
            .arg(tooltip_name_footer(d));
    item->setToolTip(tooltip);

    return item;
}

QStandardItem *FocusColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) {
    QStandardItem *item = init_aggregate(group_name);

    Dwarf *min_dwarf = nullptr;
    double min_ratio = 200.0;
    foreach(Dwarf *d, dwarves) {
        double ratio = (d->get_current_focus()*100.0)/d->get_undistracted_focus();
        if (ratio < min_ratio) {
            min_dwarf = d;
            min_ratio = ratio;
        }
    }
    if(min_dwarf){
        item->setToolTip(tr("<center><h3>%1</h3><h4>Lowest focus in group:<br/><b>%2 (%3)</b></h4></center><p>%4</p>")
                .arg(m_title)
                .arg(min_dwarf->nice_name())
                .arg(min_dwarf->get_focus_adjective())
                .arg(tr("Focus: %1% (%2/%3)")
                        .arg(min_ratio)
                        .arg(min_dwarf->get_current_focus())
                        .arg(min_dwarf->get_undistracted_focus())));
        item->setData(min_dwarf->get_focus_degree(),DwarfModel::DR_STATE);
    }
    else {
        item->setToolTip(tr("<center><h3>%1</h3><h4>Empty group</h4></center>")
                .arg(m_title));
        item->setData(-1,DwarfModel::DR_STATE);
    }
    return item;
}

void FocusColumn::init_states(){
    m_available_states.clear();
    for(int i = 0; i < DH_TOTAL_LEVELS; i++){
        m_available_states << static_cast<DWARF_HAPPINESS>(i);
    }
}
void FocusColumn::refresh_color_map(){
}

QColor FocusColumn::get_state_color(int state) const{
    if (state < 0)
        return Qt::transparent;
    else
        return DT->get_happiness_color(static_cast<DWARF_HAPPINESS>(state >= DH_TOTAL_LEVELS ? DH_TOTAL_LEVELS-1 : state));
}
