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

#include "rolestats.h"

RoleStats::RoleStats(const QVector<double> &unsorted, bool hack)
{
    m_hack = hack;
    m_raws = QSharedPointer<ECDF>(new ECDF(unsorted));
    init_list();
}

void RoleStats::set_list(const QVector<double> &unsorted){
    if(m_raws == 0)
        m_raws = QSharedPointer<ECDF>(new ECDF(unsorted));
    else
        m_raws->set_list(unsorted);
    init_list();
}

void RoleStats::init_list(){
    m_median = 0;
    m_sum_over_median = 0;
    m_sum_upper = 0;
    m_factor = -1;    
    m_upper_minmax_diff = -1;
    m_upper_raw_min = -1;


    if(m_upper == 0)
        m_upper = QSharedPointer<ECDF>(new ECDF());
    else
        m_upper.reset(new ECDF());

    QVector<double> ecdf_avgs;
    //for an unknown reason, the trait values when passed in cause all kinds of heap problems, due to
    //something attempting to read past the list. reducing the size of the list by one item seems to resolve this
    //so the hack is to drop the lowest item from the sorted list, often a 0 which won't impact the final ratings much
    if(m_hack){
            ecdf_avgs = QVector<double>(m_raws->sorted_data().size()-1);
            for(int i=1; i < m_raws->sorted_data().size()-1; i++){
                double val = m_raws->favg((double)m_raws->sorted_data().at(i));
                ecdf_avgs[i-1] = val;
            }
    }else{
        foreach(double val, m_raws->sorted_data()){
            ecdf_avgs.append(m_raws->favg(val));
        }
    }

    double idx_mid = (double)ecdf_avgs.count() / 2.0f;
    if(ecdf_avgs.count() % 2 == 0){
        std::nth_element(ecdf_avgs.begin(),ecdf_avgs.begin()+(int)idx_mid,ecdf_avgs.end());
        m_median = 0.5*(ecdf_avgs.at((int)idx_mid)+ecdf_avgs.at((int)idx_mid-1));
    }else{
        std::nth_element(ecdf_avgs.begin(),ecdf_avgs.begin()-(int)idx_mid,ecdf_avgs.end());
        m_median =  ecdf_avgs.at((int)idx_mid);
    }

    double first_quartile = ecdf_avgs.at((int)idx_mid/2.0f);
    double third_quartile = ecdf_avgs.at((int)ecdf_avgs.count() *0.75);

    if(first_quartile == m_median && ecdf_avgs.first() != ecdf_avgs.last()){
        QVector<double>::Iterator i_last = std::upper_bound(ecdf_avgs.begin()+(int)(idx_mid -1),ecdf_avgs.end(),m_median);
        int upper_start_idx = i_last - ecdf_avgs.begin();
        m_upper->set_list(ecdf_avgs.mid(upper_start_idx)); //upper ecdf from the avg ecdf values

        //upper max / upper 3rd quartile
        bool min_max_flag = ((m_raws->sorted_data().last() / m_raws->sorted_data().at((int)((m_upper->sorted_data().count() *0.75f)+upper_start_idx))) > 5);

        if(min_max_flag){
            LOGD << "     - adjusting with min/max";
            m_upper_raw_min = m_raws->sorted_data().at(upper_start_idx);
            m_upper_minmax_diff = (m_raws->sorted_data().last() - m_upper_raw_min) * 1.01f;
        }

        for(int idx = 0; idx < m_upper->sorted_data().size(); idx++){
            double val = m_upper->sorted_data().at(idx);
            m_sum_over_median += val;
            double raw_val = m_raws->sorted_data().at(upper_start_idx+idx);
            double avg_val = ((m_upper->fplus(val)+m_upper->fminus(val))/4.0f) + 0.5f;
            if(min_max_flag){
                m_sum_upper += (((raw_val - m_upper_raw_min)/m_upper_minmax_diff/2.0f+0.5f) + avg_val) / 2.0f;
            }else{
                m_sum_upper += avg_val;
            }
        }
//        foreach(double val, m_upper->sorted_data()){
//            m_sum_over_median += val;
//            m_sum_upper += ((m_upper->fplus(val)+m_upper->fminus(val))/4.0f) + 0.5f;
//        }
        m_factor = (1.0f+(((m_sum_over_median / (double)m_raws->sorted_data().size())-(m_sum_upper / (double)m_raws->sorted_data().size()))) * 2.0f);

    }
    ecdf_avgs.clear();

    double total = 0.0;
    foreach(double val, m_raws->sorted_data()){
        total += get_rating(val);
    }
    LOGD << "     - total raw values: " << m_raws->sorted_data().count();
    LOGD << "     - min raw value: " << m_raws->sorted_data().first() << "max raw value: " << m_raws->sorted_data().last();
    LOGD << "     - first quartile: " << first_quartile;
    LOGD << "     - second quartile: " << m_median;
    LOGD << "     - third quartile: " << third_quartile;    
    if(m_factor > 0){
        LOGD << "     - number of values > median: " << m_upper->sorted_data().count();
        LOGD << "     - factor: " << m_factor;
    }
    LOGD << "     - average of final ratings: " << (total / m_raws->sorted_data().count());
}

double RoleStats::get_rating(double val){
    double rating = m_raws->favg(val);
    if(m_factor > 0.0){
        if(rating <= m_median){
            rating *= m_factor;
        }else{
            rating = ((m_upper->fplus(rating)+m_upper->fminus(rating))/4.0f)+0.5;
            if(m_upper_raw_min > 0)
                rating = (((val - m_upper_raw_min)/m_upper_minmax_diff/2.0f+0.5f) + rating) / 2.0f;
        }
    }
    return rating;
}
