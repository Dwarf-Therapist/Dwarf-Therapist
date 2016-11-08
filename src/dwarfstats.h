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
#ifndef DWARFSTATS_H
#define DWARFSTATS_H

#include <QVector>

class RoleStats;
template <class T> class QSharedPointer;

class DwarfStats
{

public:

    static void set_att_potential_weight(float val){m_att_pot_weight = val;}
    static void set_skill_rate_weight(float val){m_skill_rate_weight = val;}
    static void set_max_unit_kills(int val){m_max_unit_kills = val;}

    static float get_att_potential_weight(){return m_att_pot_weight;}
    static float get_skill_rate_weight(){return m_skill_rate_weight;}
    static int get_max_unit_kills(){return m_max_unit_kills;}
    static double calc_att_potential_value(int value, float max, float cti);

    static void init_attributes(QVector<double> attribute_values, QVector<double> attribute_raw_values);
    static double get_attribute_rating(double val, bool raw = false);

    static void init_traits(QVector<double> trait_values);
    static double get_trait_rating(int val);

    static void init_prefs(QVector<double> pref_values);
    static double get_preference_rating(double val);

    static void init_skills(QVector<double> skill_values);
    static double get_skill_rating(double val);

    static void init_roles(QVector<double> role_ratings);
    static double get_role_rating(double val);

private:
    static float m_att_pot_weight;
    static float m_skill_rate_weight;
    static int m_max_unit_kills;

    static QSharedPointer<RoleStats> m_attributes;
    static QSharedPointer<RoleStats> m_attributes_raw;
    static QSharedPointer<RoleStats> m_skills;
    static QSharedPointer<RoleStats> m_traits;
    static QSharedPointer<RoleStats> m_preferences;
    static QSharedPointer<RoleStats> m_roles;
};

#endif // DWARFSTATS_H
