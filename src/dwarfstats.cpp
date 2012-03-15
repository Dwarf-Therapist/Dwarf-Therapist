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
#include "dwarf.h"

QHash<int, QVector<int>* > DwarfStats::m_dwarf_attributes;
QVector <float> DwarfStats::m_dwarf_attribute_mean;
//QVector <float> DwarfStats::m_dwarf_attribute_maximum;
//QVector <float> DwarfStats::m_dwarf_attribute_minimum;
QVector <float> DwarfStats::m_dwarf_attribute_stdDev;

QHash<int, QVector<int>* > DwarfStats::m_dwarf_skills;
QVector <float> DwarfStats::m_dwarf_skill_mean;
QVector <float> DwarfStats::m_dwarf_skill_stdDev;

void DwarfStats::load_stats(QVector<Dwarf *> dwarves){
    //calculate attribute stats
    //clear existing stats
    m_dwarf_attributes.clear();
//    m_dwarf_attribute_minimum.clear();
//    m_dwarf_attribute_maximum.clear();
    m_dwarf_attribute_stdDev.clear();
    m_dwarf_attribute_mean.clear();

    //initialize the hash table, 0-18 are attributes
    for(int i=0; i < 19; i++){
        m_dwarf_attributes[i] = new QVector<int>; //&values;
    }

    for(int i=0; i < dwarves.count(); i++){
        Dwarf *d = dwarves[i];
        if (d->is_animal() == false){
            for(int j=0; j < 19; j++){
                m_dwarf_attributes[j]->append((int)d->attribute(j));
            }
        }
    }

    foreach(int id, m_dwarf_attributes.uniqueKeys()){
        QVector<int> *values = m_dwarf_attributes.value(id);
//        int min = *std::min_element(values->begin(),values->end());
//        int max = *std::max_element(values->begin(),values->end());
        float mean = calc_mean(values);
        float stdDev = calc_standard_deviation(mean,values);

        //store the stats
//        m_dwarf_attribute_minimum.append(min);
//        m_dwarf_attribute_maximum.append(max);
        m_dwarf_attribute_mean.append(mean);
        m_dwarf_attribute_stdDev.append(stdDev);
    }




    //calculate skill stats
    m_dwarf_skills.clear();
    m_dwarf_skill_stdDev.clear();
    m_dwarf_skill_mean.clear();

    //initialize the hash table 115 skills
    for(int i=0; i < 116; i++){
        m_dwarf_skills[i] = new QVector<int>; //&values;
    }

    int skill_value = 0;
    for(int i=0; i < dwarves.count(); i++){
        Dwarf *d = dwarves[i];
        if (d->is_animal() == false){
            for(int j=0; j < 116; j++){
                skill_value = d->skill_rating(j);
                if(skill_value < 0)
                    skill_value = 0;
                m_dwarf_skills[j]->append(skill_value);
            }
        }
    }

    foreach(int id, m_dwarf_skills.uniqueKeys()){
        QVector<int> *values = m_dwarf_skills.value(id);
        float mean = calc_mean(values);
        float stdDev = calc_standard_deviation(mean,values);

        //store the stats
        m_dwarf_skill_mean.append(mean);
        m_dwarf_skill_stdDev.append(stdDev);
    }



}

float DwarfStats::calc_mean(QVector<int> *values){
    float total = 0;
    for(int i = 0; i < values->count(); i++){
        total += values->at(i);
    }
    return total / values->count();
}

float DwarfStats::calc_standard_deviation(float mean, QVector<int> *values){
    float total = 0;
    for(int i = 0; i < values->count(); i++){
        total += pow((values->at(i)-mean) , 2);
    }
    return sqrt((total / (values->count()-1)));
}

float DwarfStats::calc_cdf(float mean, float stdev, int rawValue){
    double rating = 0.0;

    //standardize the value
    double x = (rawValue - mean) / stdev;

    //calculate the cumulative distribution
    double const a1 = 0.254829592, a2 = -0.284496736, a3 = 1.421413741;
    double const a4 = -1.453152027, a5 = 1.061405429;
    double p  =  0.3275911;

    int sign = 1;
    if (x<0)
        sign = -1;
    x = fabs(x)/sqrt(2.0);

    double t = 1.0/(1.0+p*x);
    double y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*exp(-x*x);
    rating = 0.5*(1.0+sign*y);
    return rating;
}

float DwarfStats::get_attribute_rating(Attribute::ATTRIBUTES_TYPE id, int rawValue){
    float mean = get_attribute_mean(id);
    float stdev = get_attribute_stdev(id);
    return calc_cdf(mean,stdev,rawValue);
}

float DwarfStats::get_trait_rating(int rawValue){
    float mean = 50;
    float stdev = 15;
    return calc_cdf(mean,stdev,rawValue);
}

float DwarfStats::get_skill_rating(int skill_id, int rawValue){
    float mean = get_skill_mean(abs(skill_id));
    float stdev = get_skill_stdev(abs(skill_id));
    //in the case for skills where no dwarf has the skill, return a mid value for all so it doesn't drop the weighted average to nothing
    if(mean==0 && stdev==0 && rawValue==0)
        return 0.5;

    return calc_cdf(mean,stdev,rawValue);
}
