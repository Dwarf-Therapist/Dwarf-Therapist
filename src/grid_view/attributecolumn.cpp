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

AttributeColumn::AttributeColumn(const QString &title, Attribute::ATTRIBUTES_TYPE type, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title, CT_ATTRIBUTE, set, parent)
    , m_attribute_type(type)
{
    if (title.isEmpty()) { // Determine title based on type if no title was passed in
        switch(type) {
            case Attribute::AT_STRENGTH:  m_title = tr("Strength");   break;
            case Attribute::AT_AGILITY:   m_title = tr("Agility");    break;
            case Attribute::AT_TOUGHNESS: m_title = tr("Toughness");  break;
            case Attribute::AT_ENDURANCE: m_title = tr("Endurance");  break;
            case Attribute::AT_RECUPERATION: m_title = tr("Recuperation");  break;
            case Attribute::AT_DISEASE_RESISTANCE: m_title = tr("Disease resistance");  break;
            case Attribute::AT_ANALYTICAL_ABILITY: m_title = tr("Analytical Ability");  break;
            case Attribute::AT_CREATIVITY: m_title = tr("Creativity");  break;
            case Attribute::AT_EMPATHY: m_title = tr("Empathy");  break;
            case Attribute::AT_FOCUS: m_title = tr("Focus");  break;
            case Attribute::AT_INTUITION: m_title = tr("Intuition");  break;
            case Attribute::AT_KINESTHETIC_SENSE: m_title = tr("Kinesthetic Sense");  break;
            case Attribute::AT_LINGUISTIC_ABILITY: m_title = tr("Linguistic Ability");  break;
            case Attribute::AT_MEMORY: m_title = tr("Memory");  break;
            case Attribute::AT_MUSICALITY: m_title = tr("Musicality");  break;
            case Attribute::AT_PATIENCE: m_title = tr("Patience");  break;
            case Attribute::AT_SOCIAL_AWARENESS: m_title = tr("Social Awareness");  break;
            case Attribute::AT_SPATIAL_SENSE: m_title = tr("Spatial Sense");  break;
            case Attribute::AT_WILLPOWER: m_title = tr("Willpower");  break;
        }
    }
}

AttributeColumn::AttributeColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s, set, parent)
    , m_attribute_type(static_cast<Attribute::ATTRIBUTES_TYPE>(s.value("attribute", -1).toInt()))
{}

AttributeColumn::AttributeColumn(const AttributeColumn &to_copy)
    : ViewColumn(to_copy)
    , m_attribute_type(to_copy.m_attribute_type)
{}

QStandardItem *AttributeColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);
    short rawVal = 0;
    Attribute::level l = d->get_attribute_rating(m_attribute_type);
    if(l.rating>20)
        l.rating=20;
    QString msg;

    msg = l.description;
    rawVal = (int)d->attribute(m_attribute_type);

    if (l.rating){ //(val) {
        item->setData(l.rating);
    }

    item->setData(rawVal, DwarfModel::DR_SORT_VALUE);
    item->setData(l.rating,DwarfModel::DR_RATING);
    item->setData(CT_ATTRIBUTE, DwarfModel::DR_COL_TYPE);

    QString tooltip = QString("<h3>%1</h3>%2 (%3)<br><h4>%4</h4>")
            .arg(m_title)
            .arg(msg)
            .arg(rawVal)
            .arg(d->nice_name());
    item->setToolTip(tooltip);
    return item;
}

QStandardItem *AttributeColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) {
    Q_UNUSED(group_name);
    Q_UNUSED(dwarves);
    QStandardItem *item = new QStandardItem;
    item->setData(m_bg_color, DwarfModel::DR_DEFAULT_BG_COLOR);
    return item;
}
