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
#include "gamedatareader.h"
#include "viewcolumnset.h"
#include "columntypes.h"
#include "dwarfmodel.h"
#include "dwarf.h"

WeaponColumn::WeaponColumn(const QString &title, GameDataReader::weapon w, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title, CT_WEAPON, set, parent)
    , m_weapon(w)
{        
}

QStandardItem *WeaponColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);

    if(m_weapon.name==""){
        item->setData(CT_WEAPON, DwarfModel::DR_COL_TYPE);
        item->setData(0, DwarfModel::DR_RATING);
        item->setToolTip("Weapon not found.");
        return item;
    }
    if(d->body_size() < 0){
        item->setData(CT_WEAPON, DwarfModel::DR_COL_TYPE);
        item->setData(0, DwarfModel::DR_RATING);
        item->setToolTip("Missing body_size offset!");
        return item;
    }

    QString wep = m_weapon.name.toLower();
    if(wep.indexOf(",")>0)
        wep = "these weapons";
    short draw_rating = -3;
    short rating = 1;
    int body_size = d->body_size();
    bool onehand = false;
    bool twohand = false;        
    QString desc = tr("<b>Can only wield</b> %1 with <u>2 hands</u>.").arg(wep);
    if(body_size > m_weapon.singlegrasp_size)
        onehand = true;
    if(body_size > m_weapon.multigrasp_size)
        twohand = true;

    //setup drawing ratings
    if(!onehand && !twohand){
        desc = tr("<b>Cannot wield</b> %1.").arg(wep);
        draw_rating = -5;
        rating = 0;
    }
    else if (twohand && onehand){
        desc = tr("<b>Can wield</b> %1 with one or two hands.").arg(wep);
        draw_rating = 0;
        rating = 2;
    }

    QString group_name = "";
    if(!m_weapon.skill.isEmpty()){
        group_name = tr("<br><br>Skill Group: %1<br>").arg(m_weapon.skill.toLower());
        group_name[0] = group_name[0].toUpper();
    }

    QString tt_title = m_title;
    if(tt_title.indexOf(",")>0){
         //shorten the tooltip title if necessary
        int max = 50;
        if(tt_title.length() < max)
            max = tt_title.length();
        tt_title = tt_title.mid(0,max);
        if(tt_title.length() < m_title.length())
            tt_title.append("...");
        //add a weapon list to the description
        QStringList l = m_title.split(",",QString::SkipEmptyParts);
        desc += "<br><br><b>Weapons:</b><ul>";
        for(int i = 0; i<l.length(); i++){
            desc.append(QString("<li>%1</li>").arg(l.at(i)));
        }
        desc.append("</ul>");
    }

    item->setData(CT_WEAPON, DwarfModel::DR_COL_TYPE);
    item->setData(draw_rating, DwarfModel::DR_RATING);
    item->setData(rating, DwarfModel::DR_SORT_VALUE);

    item->setToolTip(QString("<h3>%1</h3>%2%3<h4>%4</h4>")
                     .arg(tt_title)
                     .arg(desc)
                     .arg(group_name)
                     .arg(d->nice_name()));
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
