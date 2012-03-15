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

#include "dwarf.h"

class DwarfStats
{
private:
    static QHash<int, QVector<int>* > m_dwarf_attributes;
    static QVector<float> m_dwarf_attribute_mean;
    static QVector<float> m_dwarf_attribute_stdDev;
//    static QVector<float> m_dwarf_attribute_minimum;
//    static QVector<float> m_dwarf_attribute_maximum;

    static QHash<int, QVector<int>* > m_dwarf_skills;
    static QVector<float> m_dwarf_skill_mean;
    static QVector<float> m_dwarf_skill_stdDev;
public:
    static void load_stats(QVector<Dwarf*> dwarves);

    static float get_attribute_mean(int id){return m_dwarf_attribute_mean.at(id);}
    static float get_attribute_stdev(int id){return m_dwarf_attribute_stdDev.at(id);}

    static float get_skill_mean(int id){return m_dwarf_skill_mean.at(id);}
    static float get_skill_stdev(int id){return m_dwarf_skill_stdDev.at(id);}

    static float calc_cdf(float mean, float stdev, int rawValue);
    static float calc_mean(QVector<int> *values);
    static float calc_standard_deviation(float mean, QVector<int> *values);

    static float get_attribute_rating(Attribute::ATTRIBUTES_TYPE id, int rawValue);
    static float get_trait_rating(int rawValue);
    static float get_skill_rating(int skill_id, int rawValue);

};

#endif // DWARFSTATS_H
