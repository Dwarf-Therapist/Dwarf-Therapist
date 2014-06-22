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

QHash<ASPECT_TYPE, QList<DwarfStats::bin> > DwarfStats::m_trait_bins;
QHash<int, QHash<QString,DwarfStats::att_info> > DwarfStats::m_att_caste_bins;
float DwarfStats::m_att_pot_weight;
//float DwarfStats::m_role_mean;
//QHash<ATTRIBUTES_TYPE, QVector<float>* > DwarfStats::m_attribute_ratings;

QSharedPointer<ECDF> DwarfStats::skills;
QSharedPointer<ECDF> DwarfStats::atts;
QSharedPointer<ECDF> DwarfStats::traits;
QSharedPointer<ECDF> DwarfStats::prefs;

double DwarfStats::m_skill_padding;
double DwarfStats::m_pref_padding;

float DwarfStats::calc_cdf(float mean, float stdev, float rawValue){
    double rating = 0.0;

    if((mean==0 && stdev==0 && rawValue==0) || stdev==0)
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

void DwarfStats::load_trait_bins(ASPECT_TYPE key, QList<int> raws){
    if(key != unknown_aspect && !m_trait_bins.contains(key))
    {
        QList<bin> m_bins;
        bin temp;

        QList<float> probabilities;
        probabilities << 0.00375 << 0.01925 << 0.08475 << 0.7845 << 0.08475 << 0.01925 << 0.00375;
        float den = raws[0]+1; //raw starts at 1 below lowest value

        for(int i=0; i<7; i++){
            temp.density = den;
            temp.probability = probabilities.at(i);
            temp.min = raws[i];
            temp.max = raws[i+1]-1;
            m_bins.append(temp);
            den += temp.probability;
        }

        m_trait_bins.insert(key, m_bins);
    }
}

float DwarfStats::get_trait_role_rating(ASPECT_TYPE key, int value){
    //until it's verified how the castes' min/median/max raw values work,
    //don't modify the value AT ALL
    if(DT->traits_modified || key == unknown_aspect)
        return (float)value/100.0f;

    float mid = 50;
    float fVal = (float)value;    
    if(key==negative)
        mid=45;
    else if(key==positive)
        mid=55;

    if(mid!=50){
        if(value < mid)
            fVal = (((float)value / mid) /2) * 100;
        else
            fVal = ((((value-mid)/(100-mid)) /2)+0.5)*100;
    }
    return get_aspect_role_rating(fVal,m_trait_bins.value(key));
}

float DwarfStats::get_aspect_role_rating(float value, QList<bin> m_bins){
    if(value > m_bins[m_bins.length()-1].max){
        m_bins[m_bins.length()-1].max = value;
        //return 1.0;
    }

    int min = 0;
    int max = 0;
    for(int i=0; i<m_bins.length(); i++){
        min = m_bins[i].min -1;
        if(min < 0)
            min = 0;
        max = m_bins[i].max;
        if(value > min && value <= max)
            return ((
                        ((value - min) / (max - min))
                        * m_bins[i].probability)
                    + m_bins[i].density);
    }
    return 0;
}

QList<DwarfStats::bin> DwarfStats::build_att_bins(QList<int> raws){
    QList<bin> m_bins;
    bin temp;

    //above/below the top/bottom bin are jammed into 1%, so our probability is shrunk to 98%
    float prob = (float)(0.98f / (raws.count() - 2.0f)); //0.1633333333f;
    //below first bin
    temp.min = 0;
    temp.density = 0.00;
    temp.max = raws.at(0);
    temp.probability = 0.01;
    m_bins.append(temp);
    //mid range bins
    float den = temp.probability;
    for(int i=0; i < raws.size()-1; i++){
        temp.probability = prob;
        temp.density = den;
        if(i>0)
            temp.density += 0.01;
        temp.min = raws.at(i);
        temp.max = raws.at(i+1);
        m_bins.append(temp);
        den += temp.probability;
    }
    //adjust final bin's probability & density (bin for > max value, but less than absolute 5000 limit)
    m_bins[m_bins.length()-1].probability = 0.01;
    m_bins[m_bins.length()-1].density -= 0.01;
    //check the last bin
    if(m_bins[m_bins.length()-1].min > m_bins[m_bins.length()-1].max)
        m_bins[m_bins.length()-1].max = m_bins[m_bins.length()-1].min * 1.5;
    return m_bins;
}

void DwarfStats::load_att_caste_bins(int id, float ratio, QList<int> l){
    QString key = "";
    for(int i = 0; i < l.count(); i++){
        key.append(QString::number(l.at(i)));
    }

    QHash<QString,att_info> stats;
    att_info a;
    //check for this set of raws in the specific attribute bin
    if(!m_att_caste_bins.contains(id) || !m_att_caste_bins.value(id).contains(key)){        
        a = att_info();

        if(m_att_caste_bins.contains(id)){
            stats = m_att_caste_bins.take(id);
            if(stats.contains(key))
                a = stats.take(key);
        }

        QList<bin> m_bins = build_att_bins(l);

        //add the pdf bins
        a.bins = m_bins;
        //add this ratio with the first count
        a.ratios_counts.insert(ratio,1);

//        stats.insert(key,a);
//        m_att_caste_bins.insert(id,stats);
    }else{
        //the bins already exist, add this ratio if it doesn't exist, or increase a count
        stats = m_att_caste_bins.take(id);
        a = stats.take(key);
        int cnt = 1;
        if(a.ratios_counts.contains(ratio)){
            cnt = a.ratios_counts.value(ratio) + 1;
        }
        a.ratios_counts.insert(ratio,cnt);
        //m_att_caste_bins.value(id).insert(key,a);
    }

    stats.insert(key,a);
    m_att_caste_bins.insert(id,stats);

//    if(a && !DT->using_caste_ranges() && a->ratios_counts.count() > 2)
//        DT->set_use_caste_ranges(true);
}

float DwarfStats::calc_att_potential_rating(int value, float max, float cti){
    float potential_value = 0.0;
    float diff = max - value;
    if(cti < 0)
        cti = 1.0;
    float gap = diff * 500 / cti;

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

float DwarfStats::get_att_caste_role_rating(Attribute &a){
    att_info a_info;
    float sum = 0.0;
    float rating = 0.0;

    float bonus_sum = 0.0;
    float bonus_rating = 0.0;

    float potential_value = calc_att_potential_rating(a.value(),a.max(),a.cti());

    QHash<QString,att_info> stats = m_att_caste_bins.value(a.att_type());
    foreach(QString key, stats.uniqueKeys()){
        a_info = stats.value(key);
        rating = get_aspect_role_rating(a.value(),a_info.bins);

        //pass the potential rating into the set of bins
        bonus_rating = get_aspect_role_rating(potential_value, a_info.bins);
        //******************

        //apply the caste frequencies to the ratings
        QMapIterator<float, int> i(a_info.ratios_counts);
        while (i.hasNext()) {
            i.next();
            sum += (rating * i.key() * i.value());
            bonus_sum += (bonus_rating * i.key() * i.value());        
        }
    }

    a.set_rating(sum); //base rating before any weights or bonus is applied, we'll use this for drawing so save it
    a.set_rating((float)(sum * (1.0f-m_att_pot_weight))+(bonus_sum * m_att_pot_weight),true); //apply the weights and the bonus and save

    return a.rating(true);
}

void DwarfStats::cleanup(){
//    LOGD << "cleaning up dwarfstats...";
    //traits
    m_trait_bins.clear();
    //attributes
    m_att_caste_bins.clear();

//    skills = 0;
//    atts = 0;
//    traits = 0;
//    LOGD << "done cleaning dwarfstats!";
}
