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

#include "attributecolumn.h"
#include "columntypes.h"
#include "gamedatareader.h"
#include "dwarfmodel.h"
#include "attribute.h"
#include "dwarftherapist.h"
#include "dwarf.h"
#include "viewmanager.h"
#include "dtstandarditem.h"

#include <QSettings>

AttributeColumn::AttributeColumn(const QString &title, ATTRIBUTES_TYPE type, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title, CT_ATTRIBUTE, set, parent)
    , m_attribute_type(type)
{
    if (title.isEmpty()){ // Determine title based on type if no title was passed in
        m_title = GameDataReader::ptr()->get_attribute_name(type);
    }
    m_sortable_types << CST_DEFAULT << CST_MAXIMUM_VALUE;
    m_current_sort = ViewManager::get_default_col_sort(CT_ATTRIBUTE);
}

AttributeColumn::AttributeColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s, set, parent)
    , m_attribute_type(static_cast<ATTRIBUTES_TYPE>(s.value("attribute", -1).toInt()))
{
    m_sortable_types << CST_DEFAULT << CST_MAXIMUM_VALUE;
    m_current_sort = ViewManager::get_default_col_sort(CT_ATTRIBUTE);
}

AttributeColumn::AttributeColumn(const AttributeColumn &to_copy)
    : ViewColumn(to_copy)
    , m_attribute_type(to_copy.m_attribute_type)
{
    m_sortable_types = to_copy.m_sortable_types;
}

QStandardItem *AttributeColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);
    Attribute a = d->get_attribute(m_attribute_type);
    QString descriptor = a.get_descriptor();
    float rating = a.rating() * 100.0f;

    //if this is an animal, we won't have any caste balanced ratings, so just take a rating out of an arbitrary absolute of 2250
    //that means any rating over 2250 will essentially be 100%, which is pretty reasonable, since
    //scaling to 0-5000 makes the drawn squares fairly small
    if(d->is_animal())
        rating = (float)(a.get_value() / 2250.0f * 100.0f);

    //the rating is used for drawing, should be between 0-100 for attributes
    item->setData(rating, DwarfModel::DR_RATING);
    item->setData(roundf(rating), DwarfModel::DR_DISPLAY_RATING);
    //flag so we know if we need to draw a border or not
    item->setData(a.syndrome_names().count(),DwarfModel::DR_SPECIAL_FLAG);

    if(DT->multiple_castes()){
        descriptor != "" ? descriptor = "(" + descriptor + ")" : "";
    }

    item->setData(CT_ATTRIBUTE, DwarfModel::DR_COL_TYPE);

    refresh_sort(d,m_current_sort);

    if(!descriptor.isEmpty()){
        descriptor = QString("%1%2").arg("<br/>").arg(descriptor);
    }
    QString syn_desc = a.get_syndrome_desc();
    if(!syn_desc.isEmpty()){
        syn_desc = QString("%1%1%2").arg("<br/>").arg(syn_desc);
    }

    QString tooltip = QString("<center><h3>%1</h3><b>%2</b>%3%4%5</center>")
            .arg(m_title)
            .arg(a.get_value_display())
            .arg(descriptor)
            .arg(syn_desc)
            .arg(tooltip_name_footer(d));

    item->setToolTip(tooltip);

    return item;
}

void AttributeColumn::refresh_sort(Dwarf *d, COLUMN_SORT_TYPE sType){
    if(get_sortable_types().count() > 0){
        if(!m_sortable_types.contains(sType))
            sType = m_sortable_types.at(0);

        Attribute a = d->get_attribute(m_attribute_type);
        float sort_val = a.get_value();

        if(sType == CST_MAXIMUM_VALUE){
            sort_val = a.max();
        }
        m_cells.value(d)->setData(sort_val, DwarfModel::DR_SORT_VALUE);
    }
}

void AttributeColumn::refresh_sort(COLUMN_SORT_TYPE sType){
    foreach(Dwarf *d, m_cells.uniqueKeys()){
        refresh_sort(d,sType);
    }
    m_current_sort = sType;
}

QStandardItem *AttributeColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves){
    Q_UNUSED(dwarves);
    QStandardItem *item = init_aggregate(group_name);
    return item;
}
