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
#include <memory>

class RoleStats;

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

    static DwarfStats attributes;
    static DwarfStats attributes_raw;
    static DwarfStats skills;
    static DwarfStats facets;
    static DwarfStats beliefs;
    static DwarfStats needs;
    static DwarfStats preferences;
    static DwarfStats roles;

    void init(const QVector<double> &values);
    double rating(double val) const;

private:
    template<typename T>
    struct use_method_t {};

    template<typename T>
    DwarfStats(use_method_t<T>, double invalid_value = -1);

    static float m_att_pot_weight;
    static float m_skill_rate_weight;
    static int m_max_unit_kills;

    std::unique_ptr<RoleStats> m_stats;
};

#endif // DWARFSTATS_H
