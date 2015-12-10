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

#include "itemtypecolumn.h"
#include "dwarf.h"
#include "dwarfmodel.h"
#include "viewcolumnset.h"
#include "item.h"
#include "dtstandarditem.h"

ItemTypeColumn::ItemTypeColumn(const QString &title, const ITEM_TYPE &itype, ViewColumnSet *set, QObject *parent, COLUMN_TYPE cType)
    : ViewColumn(title, cType, set, parent)
    , m_iType(itype)
    , m_sort_val(CST_DEFAULT)
{
    if(title.isEmpty())
        m_title = Item::get_item_name_plural(m_iType);
}

ItemTypeColumn::ItemTypeColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s, set, parent)
    , m_iType(static_cast<ITEM_TYPE>(s.value("item_type",NONE).toInt()))
    , m_sort_val(CST_DEFAULT)
{
}

ItemTypeColumn::ItemTypeColumn(const ItemTypeColumn &to_copy)
    : ViewColumn(to_copy)
    , m_iType(to_copy.m_iType)
    , m_sort_val(to_copy.m_sort_val)
{
}

QStandardItem *ItemTypeColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);

    float rating = 0;
    float coverage = d->get_coverage_rating(m_iType);
    float uniform = d->get_uniform_rating(m_iType);
    float wear = d->get_max_wear_level(m_iType);
    int missing_count = d->get_missing_equip_count(m_iType);

    rating = uniform;
    if(uniform >= 100)
        rating = coverage;

    float sort_val = rating - wear;

    item->setData(wear,DwarfModel::DR_SPECIAL_FLAG); //determines border color
    item->setData(CT_ITEMTYPE, DwarfModel::DR_COL_TYPE);
    item->setData(missing_count, DwarfModel::DR_DISPLAY_RATING); //numeric drawing
    item->setData(rating, DwarfModel::DR_RATING); //glyph drawing (0-100)
    item->setData(m_iType, DwarfModel::DR_OTHER_ID);
    set_export_role(DwarfModel::DR_RATING);

    item->setData(sort_val,DwarfModel::DR_SORT_VALUE);
    item->setData(m_set->name(), DwarfModel::DR_SET_NAME);

    QString tooltip = QString("<center><h3>%1</h3></center>%2%3")
            .arg(m_title)
            .arg(build_tooltip_desc(d))
            .arg(tooltip_name_footer(d));

    m_cells.value(d)->setToolTip(tooltip);

    return item;

}

QString ItemTypeColumn::build_tooltip_desc(Dwarf *d){
    QString list_header = "<ul style=\"margin-top:0px; margin-bottom:0px; list-style-type: none;\">";
    QString list_footer = "</ul>";

    QString desc;
    QMap<int,QString> ordered_desc;
    QHash<QString,QList<Item*> > grouped_equipment = d->get_inventory_grouped();
    QList<Item*> missing = grouped_equipment.take(Item::missing_group_name());
    QList<Item*> uncovered = grouped_equipment.take(Item::uncovered_group_name());
    if(grouped_equipment.size() > 0){
        foreach(QString bp_name, grouped_equipment.uniqueKeys()){
            QString bp_group = "";
            if(grouped_equipment.value(bp_name).length() > 0){
                QString items;
                bool container_added = false;
                foreach(Item *i, grouped_equipment.value(bp_name)){
                    if(i->item_type() == m_iType || m_iType == NONE || Item::type_in_group(m_iType,i->item_type())){
                        items.append(QString("<li>%1</li>").arg(i->display_name(true)));
                        container_added = true;
                    }
                    if(i->contained_items().count() > 0){
                        if(container_added)
                            items.append(list_header);
                        foreach(Item *c, i->contained_items()){
                            if(c->item_type() == m_iType || m_iType == NONE || Item::type_in_group(m_iType,c->item_type()))
                                items.append(QString("<li>%1</li>").arg(c->display_name(true)));
                        }
                        if(container_added)
                            items.append(list_footer);
                    }
                }
                if(!items.isEmpty())
                    bp_group.append(items);
            }
            if(!bp_group.isEmpty()){
                QString group_desc;
                group_desc.append("<b>").append(capitalizeEach(bp_name)).append("</b>");
                group_desc.append(list_header);
                group_desc.append(bp_group);
                group_desc.append(list_footer);
                int ord_idx = 0;
                if(bp_name.contains(tr("body"),Qt::CaseInsensitive))
                    ord_idx +=1;
                if(bp_name.contains(tr("upper"),Qt::CaseInsensitive))
                    ord_idx += 2;
                if(bp_name.contains(tr("lower"),Qt::CaseInsensitive))
                    ord_idx += 3;
                if(bp_name.contains(tr("left"),Qt::CaseInsensitive))
                    ord_idx += 4;
                if(bp_name.contains(tr("right"),Qt::CaseInsensitive))
                    ord_idx += 5;
                if(bp_name.contains(tr("hand"),Qt::CaseInsensitive) || bp_name.contains(tr("arm"),Qt::CaseInsensitive))
                    ord_idx += 6;
                if(bp_name.contains(tr("foot"),Qt::CaseInsensitive) || bp_name.contains(tr("leg"),Qt::CaseInsensitive))
                    ord_idx += 7;
                ordered_desc.insertMulti(ord_idx,group_desc);
            }
        }
    }

    foreach(QString group_desc, ordered_desc.values()){
        desc.append(group_desc);
    }

    desc.append(split_list(uncovered,Item::uncovered_group_name(),list_header,list_footer,Item::color_uncovered()));
    desc.append(split_list(missing,Item::missing_group_name(),list_header,list_footer,Item::color_missing()));

    return desc;
}

QString ItemTypeColumn::split_list(QList<Item*> list, QString title, QString list_header, QString list_footer, QColor title_color){
    QString desc;
    if(list.count() > 0){
        int count = 0;
        QStringList items;
        foreach(Item *i, list){
            if(i->item_type() == m_iType || m_iType == NONE || Item::type_in_group(m_iType,i->item_type())){
                items.append(i->display_name(true));
                count++;
            }
        }
        if(count > 0){
            desc.append(QString("<font color=%1><b>%2</b></font>").arg(title_color.name()).arg(title));
            desc.append(list_header).append("<li>");
            QString split = (count > 5 ? ", " : "</li><li>");
            desc.append(items.join(split));
            desc.append("</li>").append(list_footer);
        }
    }
    return desc;
}

QStandardItem *ItemTypeColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) {
    Q_UNUSED(dwarves);
    QStandardItem *item = init_aggregate(group_name);
    return item;
}

void ItemTypeColumn::write_to_ini(QSettings &s) {
    ViewColumn::write_to_ini(s);
    if(m_iType != NONE)
        s.setValue("item_type", m_iType);
}

