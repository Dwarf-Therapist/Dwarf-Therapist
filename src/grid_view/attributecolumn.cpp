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
    QString key("attributes/%1/level_%2");
    short val = -1;
    short rawVal = 0;
    val = d->get_attribute((int)m_attribute_type);
    if(val>15){val=15;};
    QString msg;
    switch (m_attribute_type) {
        case Attribute::AT_STRENGTH:
            key = key.arg("Strength").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Strength", val);
            rawVal = (int)d->strength();
            break;
        case Attribute::AT_AGILITY:
            key = key.arg("Agility").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Agility", val);
            rawVal = (int)d->agility();
            break;
        case Attribute::AT_TOUGHNESS:
            key = key.arg("Toughness").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Toughness", val);
            rawVal = (int)d->toughness();
            break;
        case Attribute::AT_ENDURANCE:
            key = key.arg("Endurance").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Endurance", val);
            rawVal = (int)d->endurance();
            break;
        case Attribute::AT_RECUPERATION:
            key = key.arg("Recuperation").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Recuperation", val);
            rawVal = (int)d->recuperation();
            break;
        case Attribute::AT_DISEASE_RESISTANCE:
            key = key.arg("Disease Resistance").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Disease Resistance", val);
            rawVal = (int)d->disease_resistance();
            break;
        case Attribute::AT_ANALYTICAL_ABILITY:
            key = key.arg("Analytical Ability").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Analytical Ability", val);
            rawVal = (int)d->analytical_ability();
            break;
        case Attribute::AT_CREATIVITY:
            key = key.arg("Creativity").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Creativity", val);
            rawVal = (int)d->creativity();
            break;
        case Attribute::AT_EMPATHY:
            key = key.arg("Empathy").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Empathy", val);
            rawVal = (int)d->empathy();
            break;
        case Attribute::AT_FOCUS:
            key = key.arg("Focus").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Focus", val);
            rawVal = (int)d->focus();
            break;
        case Attribute::AT_INTUITION:
            key = key.arg("Intuition").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Intuition", val);
            rawVal = (int)d->intuition();
            break;
        case Attribute::AT_KINESTHETIC_SENSE:
            key = key.arg("Kinesthetic Sense").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Kinesthetic Sense", val);
            rawVal = (int)d->kinesthetic_sense();
            break;
        case Attribute::AT_LINGUISTIC_ABILITY:
            key = key.arg("Linguistic Ability").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Linguistic Ability", val);
            rawVal = (int)d->linguistic_ability();
            break;
        case Attribute::AT_MEMORY:
            key = key.arg("Memory").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Memory", val);
            rawVal = (int)d->memory();
            break;
        case Attribute::AT_MUSICALITY:
            key = key.arg("Musicality").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Musicality", val);
            rawVal = (int)d->musicality();
            break;
        case Attribute::AT_PATIENCE:
            key = key.arg("Patience").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Patience", val);
            rawVal = (int)d->patience();
            break;
        case Attribute::AT_SOCIAL_AWARENESS:
            key = key.arg("Social Awareness").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Social Awareness", val);
            rawVal = (int)d->social_awareness();
            break;
        case Attribute::AT_SPATIAL_SENSE:
            key = key.arg("Spatial Sense").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Spatial Sense", val);
            rawVal = (int)d->spatial_sense();
            break;
        case Attribute::AT_WILLPOWER:
            key = key.arg("Willpower").arg(val);
            msg = GameDataReader::ptr()->get_attribute_level_name("Willpower", val);
            rawVal = (int)d->willpower();
        break;

        default:
            LOGW << "Attribute column can't build cell since type is set to" << m_attribute_type;
    }

    if (val) {
        //msg = GameDataReader::ptr()->get_string_for_key(key);
        item->setData(val, Qt::DisplayRole);
    }

    item->setData(rawVal, DwarfModel::DR_SORT_VALUE);
    item->setData(val, DwarfModel::DR_RATING);
    item->setData(CT_ATTRIBUTE, DwarfModel::DR_COL_TYPE);

    QString tooltip = QString("<h3>%1</h3>%2 (%3)<h4>%4</h4>")
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
