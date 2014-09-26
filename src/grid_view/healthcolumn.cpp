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

#include "healthcolumn.h"
#include "columntypes.h"
#include "viewcolumnset.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "unithealth.h"
#include "gamedatareader.h"
#include "dwarftherapist.h"
#include "healthinfo.h"
#if QT_VERSION >= 0x050000
# include <QRegularExpression>
#else
# include <QRegExp>
#endif

HealthColumn::HealthColumn(const QString &title, eHealth::H_INFO categoryID, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title, CT_HEALTH, set, parent)
    , m_id(categoryID)
{
}

HealthColumn::HealthColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s, set, parent)
    , m_id(static_cast<eHealth::H_INFO>(s.value("id", -1).toInt()))
{
}

HealthColumn::HealthColumn(const HealthColumn &to_copy)
    : ViewColumn(to_copy)
    , m_id(to_copy.m_id)
{
}

QStandardItem *HealthColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);
    item->setData(CT_HEALTH, DwarfModel::DR_COL_TYPE);

    bool apply_color = false;
    bool use_symbols = false;

    UnitHealth dHealth = d->get_unit_health();

    //health info is in 3 sections: treatment, statuses and wounds
    QString health_summary;

    QStringList treatments = dHealth.get_treatment_summary(apply_color,use_symbols);
    if(treatments.size() > 0)
        health_summary.append(tr("<h4 style=\"margin:0px;\"><b>%1:</b></h4><ul style=\"margin:0px;\">%2</ul>").arg(tr("Treatment")).arg(treatments.join(", ")));

    QStringList statuses = dHealth.get_status_summary(apply_color,use_symbols);
    if(statuses.size() > 0)
        health_summary.append(tr("<h4 style=\"margin:0px;\"><b>%1:</b></h4><ul style=\"margin:0px;\">%2</ul>").arg(tr("Status")).arg(statuses.join(", ")));

    QMap<QString,QStringList> wound_details = dHealth.get_wound_summary(apply_color,use_symbols);
    if(wound_details.size() > 0){
        QString wnd_sum = "";
        foreach(QString key, wound_details.uniqueKeys()){
            wnd_sum.append("<li>").append("<b>").append(capitalizeEach(key)).append("</b>");
            if(wound_details.value(key).length() > 0){
                 wnd_sum.append("<ul><li>");
                 wnd_sum.append(wound_details.value(key).join(", "));
                 wnd_sum.append("</li></ul></li>");
            }
        }
        health_summary.append(tr("<h4 style=\"margin:0px;\"><b>%1:</b></h4><ul style=\"margin:0px;\">%2</ul>").arg(tr("Wounds")).arg(wnd_sum));
    }

    if(health_summary.trimmed().isEmpty())
         health_summary = tr("<b>No Issues.</b>");

    //get a list of the symbols for this category
    QString symbols = dHealth.get_all_category_desc(m_id,true,true).join(", ");

    int rating = 0;
    QString symbol = "";
    HealthInfo* hi = dHealth.get_most_severe(m_id);
    if(hi){
        symbol = hi->symbol(false);
        rating = 100 - hi->severity();

        //find the matching descriptions in the category in the summary, bold and color them
        if(UnitHealth::get_display_categories().count() > 0 && UnitHealth::get_display_categories().contains(m_id)){
            QList<HealthInfo*> cat_infos = UnitHealth::get_display_categories().value(m_id)->descriptions();
            foreach(HealthInfo *h_info, cat_infos){
#if QT_VERSION >= 0x050000
                QRegularExpression
#else
                QRegExp
#endif
                        re("((?<=, )|(?<=[>])|^)" + h_info->description(false));
                if(re.isValid())
                    health_summary.replace(re, QString("<b>%2</b>").arg(h_info->description(true)));
            }
        }
        item->setData(hi->color(),Qt::TextColorRole);
    }

    item->setData(rating, DwarfModel::DR_SORT_VALUE);
    item->setData(rating, DwarfModel::DR_RATING);
    item->setData(symbol, DwarfModel::DR_DISPLAY_RATING);

    QString tooltip = QString("<center><b><h3 style=\"margin:0;\">%1</h3><h5 style=\"margin:0;\">%2</h4></b></center><br>%3%4")
            .arg(m_title)
            .arg(symbols)
            .arg(health_summary)
            .arg(tooltip_name_footer(d));
    item->setToolTip(tooltip);

    return item;
}

QStandardItem *HealthColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) {
    Q_UNUSED(dwarves);
    QStandardItem *item = init_aggregate(group_name);
    return item;
}

