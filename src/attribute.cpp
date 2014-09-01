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

#include "attribute.h"
#include "dwarfstats.h"
#include "dwarftherapist.h"
#include "gamedatareader.h"

QHash<int, QVector<QString> > Attribute::m_display_descriptions;

Attribute::Attribute()
    : m_id(AT_NONE)
    , m_value(0)
    , m_value_potential(-1)
    , m_value_balanced(-1)
    , m_display_value(0)
    , m_max(0)
    , m_rating_potential(-1)    
    , m_rating(-1)
    , m_cti(0)
    , m_descriptor("")
    , m_descriptor_index(0)
{
}

Attribute::Attribute(ATTRIBUTES_TYPE id, int value, int display_value, int max, int cost_to_improve, int desc_index, QString desc)
        : m_id(id)
        , m_value(value)
        , m_value_potential(-1)
        , m_value_balanced(-1)
        , m_display_value(display_value)
        , m_max(max)
        , m_rating_potential(-1)
        , m_rating(-1)
        , m_cti(cost_to_improve)
        , m_descriptor(desc)
        , m_descriptor_index(desc_index)
{
}

QString Attribute::find_descriptor(ATTRIBUTES_TYPE id, int index){
    QString desc = "";
    QVector<QString> descriptions = m_display_descriptions.value(id);
    if(descriptions.count() > 0){
        if(index < 0)
             index = descriptions.count()-1;
        desc = descriptions.at(index);
    }
    return desc;
}

QString Attribute::get_name(){
    return GameDataReader::ptr()->get_attribute_name(m_id);
}

QString Attribute::get_value_display(){
    return QString("%1/%2").arg(m_display_value,0,10).arg(m_max,0,10);
}

QString Attribute::get_syndrome_desc(){
    if(m_syn_names.count() > 0)
        return QString("Currently affected by %1").arg(m_syn_names.join(", "));
    else
        return "";
}

float Attribute::get_potential_value(){
    if(m_value_potential < 0){
        m_value_potential = DwarfStats::calc_att_potential_value(m_value,m_max,m_cti);
    }
    return m_value_potential;
}

void Attribute::set_rating(float rating, bool potential){
    if(potential)
        m_rating_potential = rating;
    else
        m_rating = rating;
}
float Attribute::rating(bool potential){
    if(potential){
        if(m_rating_potential < 0){
            m_rating_potential = DwarfStats::get_attribute_rating(get_balanced_value());
        }
        return m_rating_potential;
    }else{
        return m_rating;
    }
}
float Attribute::get_balanced_value(){
    calculate_balanced_value();
    return m_value_balanced;
}

void Attribute::calculate_balanced_value(){
    if(m_value_balanced < 0){
        float m_pot_att_weight = DwarfStats::get_att_potential_weight();
        m_value_balanced = (m_value * (1.0f-m_pot_att_weight)) + (get_potential_value() * m_pot_att_weight);
    }
}

void Attribute::set_syn_names(QStringList names){
    m_syn_names = names;
}

void Attribute::load_attribute_descriptors(QSettings &s){
    if(m_display_descriptions.count() <= 0){
        int attributes = s.beginReadArray("attributes");
        int levels = 0;
        for(int i = 0; i < attributes; ++i) {
            QVector<QString> descriptors;
            s.setArrayIndex(i);
            int id = s.value("id",0).toInt();
            levels = s.beginReadArray("levels");
            for (int j = 0; j < levels; j++) {
                s.setArrayIndex(j);
                descriptors.append(s.value("level_name").toString());
            }
            s.endArray();
            m_display_descriptions.insert(id,descriptors);
        }
        s.endArray();
    }
}
