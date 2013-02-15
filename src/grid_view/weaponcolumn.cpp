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

#include "weaponcolumn.h"
#include "viewcolumnset.h"
#include "columntypes.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "dwarftherapist.h"
#include "weapon.h"

WeaponColumn::WeaponColumn(const QString &title, Weapon *w, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title, CT_WEAPON, set, parent)
    , m_weapon(w)
{
    connect(DT, SIGNAL(settings_changed()), this, SLOT(read_settings()));
}

QStandardItem *WeaponColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);

    if(!m_weapon || m_weapon->name_plural()==""){
        item->setData(CT_WEAPON, DwarfModel::DR_COL_TYPE);
        item->setData(0, DwarfModel::DR_RATING);
        item->setData(0, DwarfModel::DR_DISPLAY_RATING);
        item->setToolTip("Weapon not found.");
        return item;
    }
    if(d->body_size() < 0){
        item->setData(CT_WEAPON, DwarfModel::DR_COL_TYPE);
        item->setData(0, DwarfModel::DR_RATING);
        item->setData(0, DwarfModel::DR_DISPLAY_RATING);
        item->setToolTip("Missing body_size offset!");
        return item;
    }

    QString wep = m_weapon->group_name.toLower();
    if(wep.indexOf(",")>0)
        wep = tr("these weapons");
    float rating = 40; //values for 49-51 aren't shown for this column, this is the smallest red square we can get
    QString numeric_rating = "/";
    short sort_val = 1;
    int body_size = d->body_size();
    bool onehand = false;
    bool twohand = false;        
    QString desc = tr("<b>Can only wield</b> %1 with <u>2 hands</u>.").arg(wep);
    if(body_size > m_weapon->single_grasp())
        onehand = true;
    if(body_size > m_weapon->multi_grasp())
        twohand = true;

    //setup drawing ratings
    if(!onehand && !twohand){
        desc = tr("<b>Cannot wield</b> %1.").arg(wep);
        rating = 25; //this will give us a medium red square as the further from the median the larger the square gets
        numeric_rating = "X";
        sort_val = 0;
    }
    else if (twohand && onehand){
        desc = tr("<b>Can wield</b> %1 with one or two hands.").arg(wep);
        rating = 50; //again 49-51 are not drawn, so any value in there to draw nothing
        numeric_rating = "";
        sort_val = 2;
    }

    QString group_name = "";
//    if(m_weapon->melee_skill() >= 0){
//        group_name = tr("<br><br>Melee Group: %1<br>").arg(m_weapon->melee_skill());
//    }
//    if(m_weapon->ranged_skill() >= 0){
//        group_name += tr("<br><br>Ranged Group: %1<br>").arg(m_weapon->ranged_skill());
//    }

    QString tt_title = m_title;
    if(tt_title.indexOf(",")>0){        
        //add a weapon list to the description
        QStringList l = m_title.split(",",QString::SkipEmptyParts);
        desc += "<br><br><b>Weapons:</b><ul>";
        for(int i = 0; i<l.length(); i++){
            desc.append(QString("<li>%1</li>").arg(l.at(i)));
        }
        desc.append("</ul>");
    }else
        desc.append("<br><br>");

    item->setData(CT_WEAPON, DwarfModel::DR_COL_TYPE);
    item->setData(rating, DwarfModel::DR_RATING);
    item->setData(numeric_rating, DwarfModel::DR_DISPLAY_RATING);
    item->setData((sort_val * 100) + d->body_size(), DwarfModel::DR_SORT_VALUE);

    QPalette tt;
    QColor norm_text = tt.toolTipText().color();

    QString tooltip = QString("<h3>%1</h3>%2%3%4%5<h4>%6 - %7</h4>")
                     .arg(tt_title)
                     .arg(desc)
                     .arg(group_name)
                     .arg(tr("1h: <font color=%1>%2</font> cm<sup>3</sup><br>")
                          .arg(onehand ? norm_text.name() : QColor(Qt::red).name())
                          .arg(m_weapon->single_grasp()*10))
                     .arg(tr("2h: <font color=%1>%2</font> cm<sup>3</sup>")
                          .arg(twohand ? norm_text.name() : QColor(Qt::red).name())
                          .arg(m_weapon->multi_grasp()*10))
                     .arg(d->nice_name())
                     .arg(tr("%1 cm<sup>3</sup><br>").arg(body_size*10));

    item->setToolTip(tooltip);

    return item;
}

QStandardItem *WeaponColumn::build_aggregate(const QString &, const QVector<Dwarf*> &) {
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
