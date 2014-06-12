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

#include "qhash.h"
#include "qmap.h"
#include "attribute.h"

class Dwarf;

class DwarfStats
{

public:
    static float calc_cdf(float mean, float stdev, float rawValue);    

    //static void set_role_mean(float mean){m_role_mean = mean;}
    //static float get_role_mean(){return m_role_mean;}

    struct bin{
        int min;
        int max;
        double density;
        double probability;
    };

    struct att_info
    {
        //ratio, caste count
        QMap<float,int> ratios_counts;
        QList<bin> bins;
    };

    //caste attributes
    static void load_att_caste_bins(int id, float ratio, QList<int> l);
//    static float get_att_caste_role_rating(ATTRIBUTES_TYPE atype, int val); //old  version
    static float get_att_caste_role_rating(Attribute &a);

    //traits
    static float get_trait_role_rating(ASPECT_TYPE, int);
    static QHash<ASPECT_TYPE, QList<bin> > m_trait_bins;
    static void load_trait_bins(ASPECT_TYPE, QList<int>);

    static void cleanup();
    static void set_att_potential_weight(float val){m_att_pot_weight = val;}

private:
    static float m_att_pot_weight;
    static float get_aspect_role_rating(float value, QList<bin> m_bins);

    static QHash<int, QHash<QString, att_info> > m_att_caste_bins;
//    static QHash<ATTRIBUTES_TYPE, QVector<float>* > m_attribute_ratings;

    static QList<bin> build_att_bins(QList<int>);
    //static float m_role_mean;
};

#endif // DWARFSTATS_H
