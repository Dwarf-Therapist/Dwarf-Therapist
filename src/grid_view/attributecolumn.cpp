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
#include "viewcolumnset.h"
#include "gamedatareader.h"
#include "dwarfmodel.h"
#include "truncatingfilelogger.h"
#include "dwarfstats.h"
#include "caste.h"
#include "attribute.h"
#include "dwarftherapist.h"
#include "dwarf.h"

AttributeColumn::AttributeColumn(const QString &title, ATTRIBUTES_TYPE type, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title, CT_ATTRIBUTE, set, parent)
    , m_attribute_type(type)
{
    if (title.isEmpty()) { // Determine title based on type if no title was passed in
        switch(type) {
            case AT_STRENGTH:  m_title = tr("Strength");   break;
            case AT_AGILITY:   m_title = tr("Agility");    break;
            case AT_TOUGHNESS: m_title = tr("Toughness");  break;
            case AT_ENDURANCE: m_title = tr("Endurance");  break;
            case AT_RECUPERATION: m_title = tr("Recuperation");  break;
            case AT_DISEASE_RESISTANCE: m_title = tr("Disease resistance");  break;
            case AT_ANALYTICAL_ABILITY: m_title = tr("Analytical Ability");  break;
            case AT_CREATIVITY: m_title = tr("Creativity");  break;
            case AT_EMPATHY: m_title = tr("Empathy");  break;
            case AT_FOCUS: m_title = tr("Focus");  break;
            case AT_INTUITION: m_title = tr("Intuition");  break;
            case AT_KINESTHETIC_SENSE: m_title = tr("Kinesthetic Sense");  break;
            case AT_LINGUISTIC_ABILITY: m_title = tr("Linguistic Ability");  break;
            case AT_MEMORY: m_title = tr("Memory");  break;
            case AT_MUSICALITY: m_title = tr("Musicality");  break;
            case AT_PATIENCE: m_title = tr("Patience");  break;
            case AT_SOCIAL_AWARENESS: m_title = tr("Social Awareness");  break;
            case AT_SPATIAL_SENSE: m_title = tr("Spatial Sense");  break;
            case AT_WILLPOWER: m_title = tr("Willpower");  break;
        }
    }    
}

AttributeColumn::AttributeColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s, set, parent)
    , m_attribute_type(static_cast<ATTRIBUTES_TYPE>(s.value("attribute", -1).toInt()))
{    
}

AttributeColumn::AttributeColumn(const AttributeColumn &to_copy)
    : ViewColumn(to_copy)
    , m_attribute_type(to_copy.m_attribute_type)
{    
}

QStandardItem *AttributeColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);
    Attribute a = d->get_attribute((int)m_attribute_type);
    short rawVal = a.value();
    QString descriptor = a.get_descriptor();
    float rating = a.rating() * 100.0f;    

    //if this is an animal, we won't have any caste balanced ratings, so just take a rating out of an arbitrary absolute of 2250
    //that means any rating over 2250 will essentially be 100%, which is pretty reasonable, since
    //scaling to 0-5000 makes the drawn squares fairly small
    if(d->is_animal())
        rating = (float)(a.value() / 2250.0f * 100.0f);

    //the rating is used for drawing, should be between 0-100 for attributes
    item->setData(rating, DwarfModel::DR_RATING);
    item->setData(roundf(rating), DwarfModel::DR_DISPLAY_RATING);


    //if no descriptor (middle ranges) then set the rating to a middle (hidden) value
    //this is primarily for vanilla, as the mid ranges don't have a description and are hidden in game
    //for multiple castes, we may as well draw everything as the descriptors can be different for each caste
    //since multiple castes append 'for a <caste name>' to the descriptor, this will only ever affect vanilla for now
    if(!DT->multiple_castes && a.get_descriptor_rank() == 4){
        item->setData(50.0f, DwarfModel::DR_RATING); //49-51 aren't drawn for attributes
    }else{
        descriptor != "" ? descriptor = "(" + descriptor + ")" : "";
    }

    //sort on the raw value
    item->setData(rawVal, DwarfModel::DR_SORT_VALUE);    
    item->setData(CT_ATTRIBUTE, DwarfModel::DR_COL_TYPE);

    QString tooltip = QString("<center><h3>%1</h3></center><b>%2</b> %4<h4>%5</h4>")
            .arg(m_title)            
            .arg(a.get_value_display())
            .arg(descriptor)
            .arg(d->nice_name());

    item->setToolTip(tooltip);

    return item;
}

QStandardItem *AttributeColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves){
    Q_UNUSED(dwarves);
    QStandardItem *item = init_aggregate(group_name);
    return item;
}
