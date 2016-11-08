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

#include "dwarfstats.h"
#include "rolestats.h"

#include <QSharedPointer>

float DwarfStats::m_att_pot_weight;
float DwarfStats::m_skill_rate_weight;
int DwarfStats::m_max_unit_kills;

QSharedPointer<RoleStats> DwarfStats::m_skills;
QSharedPointer<RoleStats> DwarfStats::m_attributes;
QSharedPointer<RoleStats> DwarfStats::m_attributes_raw;
QSharedPointer<RoleStats> DwarfStats::m_traits;
QSharedPointer<RoleStats> DwarfStats::m_preferences;
QSharedPointer<RoleStats> DwarfStats::m_roles;

double DwarfStats::calc_att_potential_value(int value, float max, float cti){
    double potential_value = 0.0;
    double diff = max - value;
    if(cti <= 0)
        cti = 1.0;
    double gap = diff * 500 / cti;

    //uses the cost to improve and the attributes maximum to apply a bonus
    //based on the difference between the attribute's current value and the maximum possible
    if(value >= max){
        potential_value = value;
    }else{
        potential_value = 0.5f * (1-(gap/diff));
        if(potential_value >= 0)
            potential_value += 0.5f;
        else
            potential_value = -0.5f / (potential_value - 1.0f);
        potential_value = value + (potential_value * gap);
    }
    return potential_value;
}

void DwarfStats::init_attributes(QVector<double> attribute_values, QVector<double> attribute_raw_values){
    if(m_attributes == 0)
        m_attributes = QSharedPointer<RoleStats>(new RoleStats(attribute_values));
    else
        m_attributes->set_list(attribute_values);
    if(m_attributes_raw == 0)
        m_attributes_raw = QSharedPointer<RoleStats>(new RoleStats(attribute_raw_values));
    else
        m_attributes_raw->set_list(attribute_raw_values);
}
double DwarfStats::get_attribute_rating(double val,bool raw){
    if(raw)
        return m_attributes_raw->get_rating(val);
    else
        return m_attributes->get_rating(val);
}

void DwarfStats::init_traits(QVector<double> trait_values){
    if(m_traits == 0)
        m_traits = QSharedPointer<RoleStats>(new RoleStats(trait_values));
    else
        m_traits->set_list(trait_values);
}
double DwarfStats::get_trait_rating(int val){
    return m_traits->get_rating(val);
}

void DwarfStats::init_prefs(QVector<double> pref_values){
    if(m_preferences == 0)
        m_preferences = QSharedPointer<RoleStats>(new RoleStats(pref_values,0));
    else
        m_preferences->set_list(pref_values);
}
double DwarfStats::get_preference_rating(double val){
    return m_preferences->get_rating(val);
}

void DwarfStats::init_skills(QVector<double> skill_values){
    if(m_skills == 0)
        m_skills = QSharedPointer<RoleStats>(new RoleStats(skill_values,0));
    else
        m_skills->set_list(skill_values);
}
double DwarfStats::get_skill_rating(double val){
    return m_skills->get_rating(val);
}

void DwarfStats::init_roles(QVector<double> role_ratings){
    if(m_roles == 0)
        m_roles = QSharedPointer<RoleStats>(new RoleStats(role_ratings,-1,true));
    else
        m_roles->set_list(role_ratings);
}
double DwarfStats::get_role_rating(double val){
    return m_roles->get_rating(val);
}
