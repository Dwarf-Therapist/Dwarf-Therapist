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

#include "truncatingfilelogger.h"
#include "ecdf.h"

class Dwarf;

class DwarfStats
{

public:
    static float calc_cdf(float mean, float stdev, float rawValue);    

    static void set_role_stats(float min, float max, float median){
        m_role_min = min;
        m_role_max = max;
        m_role_median  = median;
    }

    static float get_role_median(){return m_role_median;}
    static float get_role_min(){return m_role_min;}
    static float get_role_max(){return m_role_max;}


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
    static float calc_att_potential_value(int value, float max, float cti);

    //traits
    static float get_trait_role_rating(ASPECT_TYPE, int);
    static QHash<ASPECT_TYPE, QList<bin> > m_trait_bins;
    static void load_trait_bins(ASPECT_TYPE, QList<int>);

    static void cleanup();
    static void set_att_potential_weight(float val){m_att_pot_weight = val;}
    static float get_att_potential_weight(){return m_att_pot_weight;}

    static void set_skill_rate_weight(float val){m_skill_rate_weight = val;}
    static float get_skill_rate_weight(){return m_skill_rate_weight;}

//    static float get_empty_skill_rating(){return m_skill_padding;}

    static void init_attributes(QVector<double> attribute_values, QVector<double> attribute_raw_values){
        if(atts == 0)
            atts = QSharedPointer<ECDF>(new ECDF(attribute_values));
        else
            atts->set_list(attribute_values);
        if(atts_raw == 0)
            atts_raw = QSharedPointer<ECDF>(new ECDF(attribute_raw_values));
        else
            atts_raw->set_list(attribute_raw_values);
    }
    static double get_att_ecdf(int val,bool raw = false){
        if(raw)
            return atts_raw->fplus((double)val);
        else
            return atts->favg((double)val);
    }

    static void init_traits(QVector<double> trait_values){
        if(traits == 0)
            traits = QSharedPointer<ECDF>(new ECDF(trait_values));
        else
            traits->set_list(trait_values);
    }
    static double get_trait_ecdf(int val){
        return traits->favg((double)val);
    }

    static void init_prefs(QVector<double> pref_values){
        if(prefs == 0)
            prefs = QSharedPointer<ECDF>(new ECDF(pref_values));
        else
            prefs->set_list(pref_values);
    }
    static double get_pref_ecdf(double val){
        return prefs->favg(val);
    }

    static void init_skills(QVector<double> skill_values){
        if(skills == 0)
            skills = QSharedPointer<ECDF>(new ECDF(skill_values));
        else
            skills->set_list(skill_values);
    }
    static double get_skill_ecdf(float val){
        return skills->favg(val);
    }

private:
    static float m_att_pot_weight;
    static float m_skill_rate_weight;

    static float get_aspect_role_rating(float value, QList<bin> m_bins);

    static QHash<int, QHash<QString, att_info> > m_att_caste_bins;
//    static QHash<ATTRIBUTES_TYPE, QVector<float>* > m_attribute_ratings;

    static QList<bin> build_att_bins(QList<int>);
    static float m_role_min;
    static float m_role_max;
    static float m_role_median;

    static QSharedPointer<ECDF> atts;
    static QSharedPointer<ECDF> atts_raw;
    static QSharedPointer<ECDF> skills;
    static QSharedPointer<ECDF> traits;
    static QSharedPointer<ECDF> prefs;
};

#endif // DWARFSTATS_H
