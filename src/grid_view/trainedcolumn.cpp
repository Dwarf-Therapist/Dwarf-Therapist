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

#include "trainedcolumn.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "viewcolumnset.h"

TrainedColumn::TrainedColumn(const QString &title, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title, CT_TRAINED, set, parent)
{
}

TrainedColumn::TrainedColumn(const TrainedColumn &to_copy)
    : ViewColumn(to_copy)

{
}

QStandardItem *TrainedColumn::build_cell(Dwarf *d){
    QStandardItem *item = init_cell(d);

    QString desc = get_animal_trained_descriptor(d->trained_level());
    float rating = (float)d->trained_level();
    float sort_val = rating;
    QString draw_rating = QString::number(rating);

    if(rating == 7){ //tame
            rating = 50.0f; //don't draw tame animals, this has to be within the uberdelegate's ignore range
            sort_val = -1;
            draw_rating = tr("D");
    }else if(rating >= 1 && rating <= 6){ //trained levels
        rating = (rating / 6.0f * 100.0f / 2.0f) + 50.0f; //scale from 50 to 100
    }else if(rating == 0){ //semi-wild (medium red square)
        rating = 30.0f;
        draw_rating = tr("Sw");
    }else{ //wild, unknown (large red square)
        rating = 5.0f;
        sort_val = -2;
        draw_rating = tr("W");
    }

    item->setData(CT_TRAINED, DwarfModel::DR_COL_TYPE);
    item->setData(draw_rating, DwarfModel::DR_DISPLAY_RATING); //numeric drawing, single digits
    item->setData(rating, DwarfModel::DR_RATING); //other drawing 0-100
    item->setData(sort_val, DwarfModel::DR_SORT_VALUE);

    QString tooltip = QString("<center><h3>%1</h3><h4>%3</h4></center>")
            .arg(desc)
            .arg(d->nice_name());
    item->setToolTip(tooltip);
    return item;
}


QStandardItem *TrainedColumn::build_aggregate(const QString &, const QVector<Dwarf *> &){
    QStandardItem *item = new QStandardItem;
    QColor bg;
    if (m_override_set_colors)
        bg = m_bg_color;
    else
        bg = m_set->bg_color();
    item->setData(bg, Qt::BackgroundColorRole);
    item->setData(bg, DwarfModel::DR_DEFAULT_BG_COLOR);
    return item;
}
