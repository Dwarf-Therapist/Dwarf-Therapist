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

public:


    static void load_stats(QVector<Dwarf*> dwarves);

    static float get_attribute_mean(int id){return m_dwarf_attribute_mean.at(id);}
    static float get_attribute_stdev(int id){return m_dwarf_attribute_stdDev.at(id);}

    static float get_skill_mean(int id){return m_dwarf_skill_mean.at(id);}    
    static float get_skill_stdev(int id){return m_dwarf_skill_stdDev.at(id);}

    static float get_trait_mean(int id){return m_dwarf_trait_mean.at(id);}
    static float get_trait_stdev(int id){return m_dwarf_trait_stdDev.at(id);}

    static float calc_cdf(float mean, float stdev, float rawValue);    
    static float calc_mean(QVector<int> *values);
    static float calc_variance(QVector<int> * values, float mean);
    static float calc_standard_deviation(float mean, QVector<int> *values);    

    struct bin{
        int min;
        int max;
        double density;
        double probability;
    };

    static void load_skills(QVector<Dwarf *> dwarves);

    static void load_attribute_bins(ASPECT_TYPE, QList<int>);
    static float get_attribute_role_rating(ASPECT_TYPE, int);
    static float get_trait_role_rating(ASPECT_TYPE, int);
    static float get_skill_role_rating(int, int);

    static QHash<ASPECT_TYPE, QList<bin> > m_trait_bins;
    static void load_trait_bins(ASPECT_TYPE, QList<int>);

    static QHash<int, QVector<int>* > m_dwarf_skills;

private:
    static QHash<int, QVector<int>* > m_dwarf_attributes;
    static QVector<float> m_dwarf_attribute_mean;
    static QVector<float> m_dwarf_attribute_stdDev;


    static QVector<float> m_dwarf_skill_mean;
    static QVector<float> m_dwarf_skill_stdDev;

    static QHash<int, QVector<int>* > m_dwarf_traits;
    static QVector<float> m_dwarf_trait_mean;
    static QVector<float> m_dwarf_trait_stdDev;

    static QHash<ASPECT_TYPE, QList<bin> > m_attribute_bins;

    static float get_aspect_role_rating(int value, QList<bin> m_bins);


};

#endif // DWARFSTATS_H
