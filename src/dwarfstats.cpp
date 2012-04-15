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
#include "truncatingfilelogger.h"

//QHash<int, QVector<int>* > DwarfStats::m_dwarf_skills;
//QVector <float> DwarfStats::m_dwarf_skill_mean;
//QVector <float> DwarfStats::m_dwarf_skill_stdDev;

QHash<ASPECT_TYPE, QList<DwarfStats::bin> > DwarfStats::m_attribute_bins;
QHash<ASPECT_TYPE, QList<DwarfStats::bin> > DwarfStats::m_trait_bins;


float DwarfStats::calc_cdf(float mean, float stdev, float rawValue){
    double rating = 0.0;

    if(mean==0 && stdev==0 && rawValue==0)
        return 0.5;

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

//void DwarfStats::load_skills(QVector<Dwarf *> dwarves){
//    //initialize the hash table 115 skills
//    for(int i=0; i < 116; i++){
//        m_dwarf_skills[i] = new QVector<int>;
//    }

//    foreach(Dwarf *d, dwarves){
//        if (d->is_animal() == false){
//            QVector<Skill> *skills = d->get_skills();
//            for (int i = 0; i < skills->size(); i++) {
//                Skill s = skills->at(i);
//                m_dwarf_skills.value(s.id())->append(s.actual_exp());
//            }
//        }
//    }

//    //find skill medians
//    foreach(int id, m_dwarf_skills.uniqueKeys()){
//        QVector<int> *values = m_dwarf_skills.value(id);
//        float median = 0;
//        if(values->count()>0){

//            if (values->count() % 2 == 0){
//                median = values->at(values->count() / 2 - 1);
//                median += values->at(values->count() / 2);
//                median /= 2;
//            }
//            else
//                median = values->at(values->count()/2 -0.5);
//        }
//        m_dwarf_skill_mean.append(median);
//    }
//}

void DwarfStats::load_attribute_bins(ASPECT_TYPE key, QList<int> raws){
    if(!m_attribute_bins.contains(key))
    {
        QList<bin> m_bins;
        bin temp;

        //below first bin
        temp.min = 0;
        temp.density = 0.00;
        temp.max = raws[0];
        temp.probability = 0.01;
        m_bins.append(temp);
        //mid range bins
        for(int i=0; i<7; i++){
            temp.probability = 0.16333333333;
            temp.density = temp.probability * i;
            if(i>0)
                temp.density = temp.density + 0.01;
            temp.min = raws[i];
            temp.max = raws[i+1];
            m_bins.append(temp);
        }
        //adjust final bin's probability
        m_bins[m_bins.length()-1].probability = 0.01;

        m_attribute_bins.insert(key, m_bins);
    }
}

float DwarfStats::get_attribute_role_rating(ASPECT_TYPE key, int value){
    return get_aspect_role_rating(value, m_attribute_bins.value(key));
}

//Normal
//0.00385    0 to 9
//0.01965    10 to 24
//0.0856    25 to 39
//0.7818    40 to 60
//0.0856    61 to 75
//0.01965    76 to 90
//0.00385    91 to 100

void DwarfStats::load_trait_bins(ASPECT_TYPE key, QList<int> raws){
    if(!m_trait_bins.contains(key))
    {
        QList<bin> m_bins;
        bin temp;

        for(int i=0; i<7; i++){
            switch(i){
            case 0:
                temp.density = 0;
                temp.probability = 0.004527777766667;
                break;
            case 1:
                temp.density = 0.004527777766667;
                temp.probability = 0.017646604933333;
                break;
            case 2:
                temp.density = 0.0221743827;
                temp.probability = 0.0883287037;
                break;
            case 3:
                temp.density = 0.1105030864;
                temp.probability = 0.7853132716;
                break;
            case 4:
                temp.density = 0.895816358;
                temp.probability = 0.080581790133333;
                break;
            case 5:
                temp.density = 0.976398148133333;
                temp.probability = 0.0204845679;
                break;
            case 6:
                temp.density = 0.996882716033333;
                temp.probability = 0.003117283966667;
                break;
            }

            temp.min = raws[i]-1;
            temp.max = raws[i+1]-1;
            m_bins.append(temp);
        }

        m_trait_bins.insert(key, m_bins);
    }
}

float DwarfStats::get_trait_role_rating(ASPECT_TYPE key, int value){
    return get_aspect_role_rating(value, m_trait_bins.value(key));
}

float DwarfStats::get_aspect_role_rating(int value, QList<bin> m_bins){
    for(int i=0; i<m_bins.length(); i++){
        if(value > m_bins[i].min && value <= m_bins[i].max){
            return ((((double)(value-m_bins[i].min) / (double)(m_bins[i].max - m_bins[i].min))
                     * m_bins[i].probability)
                    + m_bins[i].density);
        }
    }
    return 0;
}

//float DwarfStats::get_skill_role_rating(int skill_id, int value){
//    QVector<int> *skills = m_dwarf_skills.value(skill_id);
//    float result = 0.5;
//    if(skills->count() > 0){
//        float max = *std::max_element(skills->begin(),skills->end());
//        float median = get_skill_mean(skill_id);
//        if(median==max)
//            median = max / 2; //drop the median so that max = 1.0 and 0 is scaled accordingly

//        if(value<median){
//            result = ((value-median) / ((max-value)+0.001)+1.0) / 2.0;
//        }else{
//            result =  ((median-value) / ((median-max)+0.001)+1.0) / 2.0;
//        }
//    }
//    return result;
//}
