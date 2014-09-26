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
#include <numeric>

RoleStats::RoleStats(const QVector<double> &unsorted)
{
    m_transform_type = TT_UNKNOWN;
    m_raws = QSharedPointer<ECDF>(new ECDF(unsorted));
    init_list();
}

RoleStats::RoleStats(const QVector<double> &unsorted, const TRANSFORM_TYPE t)
{
    m_transform_type = t;
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
    m_ecdf_median = 0;
    m_sum_over_median = 0;
    m_sum_upper = 0;
    m_factor = -1;
    m_upper_minmax_diff = -1;
    m_upper_raw_min = -1;
    m_transformations.clear();

    if(m_upper == 0){
        m_upper = QSharedPointer<ECDF>(new ECDF());
    }else{
        QSharedPointer<ECDF> new_ptr(new ECDF());
        m_upper.swap(new_ptr);
    }

    m_raw_median = find_median(m_raws->sorted_data());

    //indicates the data is badly skewed on the top end, and requires additional work
    //special case for the ecdf_rank_try, which if q1=median should use ecdf_rank on the upper
    //and otherwise use ecdf_rank on everything
    if(m_transform_type == TT_MIN_MAX_ALL){
        load_list_stats(m_raws->sorted_data(),true);
    }else{
        if(m_raws->sorted_data().at((int)m_raws->sorted_data().size()/4.0) == m_raw_median){
            if(m_transform_type == TT_ECDF_RANK_TRY){
                m_transform_type = TT_ECDF_RANK_UPPER;
            }
            configure_transformations();
        }else{
            if(m_transform_type == TT_ECDF_RANK_TRY){
                m_transform_type = TT_ECDF_RANK_ALL; //set the type and that's it
            }else{
                m_transform_type = TT_MULTI_ALL;
                //use multiple transformations for all values
                if(!load_transformations(m_raws->sorted_data())){
                    m_transformations.clear();
                    m_transform_type = TT_UNKNOWN;
                    configure_transformations();
                }
            }
        }
    }

    if(DT->get_log_manager()->get_appender("core")->minimum_level() <= LL_DEBUG){
        double total = 0.0;
        foreach(double val, m_raws->sorted_data()){
            total += get_rating(val);
        }
        transform_stats ts = load_list_stats(m_raws->sorted_data(),false);
        LOGD << "     - total raw values:" << m_raws->sorted_data().count();
        LOGD << "     - median raw values:" << ts.median;
        LOGD << "     - average of raw values:" << ts.average;
        LOGD << "     - min raw value:" << m_raws->sorted_data().first() << "max raw value:" << m_raws->sorted_data().last();
        if(m_factor > 0){
            LOGD << "     - number of values > median:" << m_upper->sorted_data().count();
            LOGD << "     - factor:" << m_factor;
        }
        LOGD << "     - min rating:" << get_rating(ts.min) << "max rating:" << get_rating(ts.max);
        LOGD << "     - average of final ratings:" << (total / m_raws->sorted_data().count());
        LOGD << "     ------------------------------";
    }
}

void RoleStats::configure_transformations(){
    QVector<double> ecdf_avgs;
    foreach(double val, m_raws->sorted_data()){
        ecdf_avgs.append(m_raws->favg(val));
    }

    int idx_mid = (double)ecdf_avgs.count() / 2.0f;
    m_ecdf_median = find_median(ecdf_avgs);
    double ecdf_q1 = ecdf_avgs.at(idx_mid/2.0f);
    double ecdf_q3 = ecdf_avgs.at(ecdf_avgs.count() *0.75);

    LOGD << "     - first quartile (ecdf/rank): " << ecdf_q1;
    LOGD << "     - second quartile (ecdf/rank): " << m_ecdf_median;
    LOGD << "     - third quartile (ecdf/rank): " << ecdf_q3;

    QVector<double>::Iterator i_last = std::upper_bound(ecdf_avgs.begin()+(idx_mid -1),ecdf_avgs.end(),m_ecdf_median);
    int upper_start_idx = i_last - ecdf_avgs.begin();
    if(upper_start_idx >= 0 && upper_start_idx < m_raws->sorted_data().size()){
        m_upper->set_list(ecdf_avgs.mid(upper_start_idx));
    }

    //if a particular method has already been set, transform without additional checks and tests
    //currently this only applies to preferences which sets either ecdf/rank all, or ecdf/rank upper before this point
    if(m_transform_type == TT_ECDF_RANK_UPPER){
        calculate_factor_value(upper_start_idx);
    }else{ //method is unknown, perform more checks
        float q4_q3_check = 0;
        if(upper_start_idx < m_raws->sorted_data().size()){
            float q3 = m_raws->sorted_data().at((int)((m_upper->sorted_data().count() *0.75f)+upper_start_idx));
            if(q3 > 0)
                q4_q3_check = (m_raws->sorted_data().last() / q3);
        }

        LOGD << "     - checking q4/q3 = " << q4_q3_check;

        if(q4_q3_check > 2.0f){
            //use the default ecdf/rank for the lower values, and a minmax conversion for upper values
            m_transform_type = TT_MIN_MAX_UPPER;
            calculate_factor_value(upper_start_idx);
        }else{
            //use multiple transformations for the upper values
            m_transform_type = TT_MULTI_UPPER;
            if(load_transformations(m_raws->sorted_data().mid(upper_start_idx))){
                //use the default ecdf/rank for the lower values
                calculate_factor_value(upper_start_idx);
            }else{
                m_transform_type = TT_ECDF_RANK_UPPER;
                LOGD << "     - setting up multiple transformations failed, using ecdf/rank for values above median";
                m_transformations.clear();
                calculate_factor_value(upper_start_idx);
            }
        }
    }

    ecdf_avgs.clear();
}

double RoleStats::get_rating(double val){
    double rating = 0.0;
    if(m_transform_type == TT_MULTI_ALL){
        rating = get_transformations_rating(val);
    }else if(m_transform_type == TT_ECDF_RANK_ALL){
        rating = m_raws->favg(val);
    }else if(m_transform_type == TT_MIN_MAX_ALL){
        rating = range_transform(val,m_transformations[0].min,m_transformations[0].median,m_transformations[0].max);
    }else{
        double base_rating = m_raws->favg(val); //start with ecdf/rank average
        if(val <= m_raw_median){
            rating = base_rating * m_factor;
        }else{
            if(m_transform_type == TT_ECDF_RANK_UPPER){
                rating = ((m_upper->fplus(base_rating)+m_upper->fminus(base_rating))/4.0f)+0.5;
            }else if(m_transform_type == TT_MIN_MAX_UPPER){
                rating = ((val - m_upper_raw_min)/m_upper_minmax_diff/2.0f+0.5f);
            }else if(m_transform_type == TT_MULTI_UPPER){
                rating = (get_transformations_rating(val) / 2.0f)+0.5f;
            }
        }
    }
    return rating;
}

double RoleStats::get_transformations_rating(double val){
    Q_ASSERT(m_transformations.count() >= 3);
    double rating = val;
    rating = range_transform(rating,m_transformations[0].min,m_transformations[0].average,m_transformations[0].max); //mean
    rating = range_transform(rating,m_transformations[1].min,m_transformations[1].median,m_transformations[1].max); //median
    rating += (0.5f-m_transformations[2].average); //median adj
    rating = range_transform(rating,m_transformations[3].min,m_transformations[3].average,m_transformations[3].max); //0-50-100
    rating = (rating * (1-m_transform_one_percent))+(m_transform_one_percent / 2.0f);
    return rating;
}


void RoleStats::calculate_factor_value(int upper_start_idx){
    if(m_transform_type == TT_MIN_MAX_UPPER){
        m_upper_raw_min = m_raws->sorted_data().at(upper_start_idx);
        m_upper_minmax_diff = (m_raws->sorted_data().last() - m_upper_raw_min) * 1.01f;
    }

    double raw_val = 0.0;
    double avg_val = 0.0;
    for(int idx = 0; idx < m_upper->sorted_data().size(); idx++){
        double val = m_upper->sorted_data().at(idx);
        m_sum_over_median += val;
        if(m_transform_type == TT_MIN_MAX_UPPER){
            //use a min max conversion
            raw_val = m_raws->sorted_data().at(upper_start_idx+idx);
            m_sum_upper += ((raw_val - m_upper_raw_min)/m_upper_minmax_diff/2.0f+0.5f);
        }else{
            //use ecdf/rank
            avg_val = ((m_upper->fplus(val)+m_upper->fminus(val))/4.0f) + 0.5f;
            m_sum_upper += avg_val;
        }
    }
    m_factor = (1.0f+(((m_sum_over_median / (double)m_raws->sorted_data().size())-(m_sum_upper / (double)m_raws->sorted_data().size()))) * 2.0f);
}

double RoleStats::find_median(QVector<double> v){
    int idx_mid = (double)v.count() / 2.0f;
    double m = 0.0;
    if(v.count() % 2 == 0){
        std::nth_element(v.begin(),v.begin()+idx_mid,v.end());
        m = 0.5*(v.at(idx_mid)+v.at(idx_mid-1));
    }else{
        std::nth_element(v.begin(),v.begin()+idx_mid,v.end());
        m =  v.at(idx_mid);
    }
    return m;
}

bool RoleStats::load_transformations(QVector<double> list){
    if(list.size() <= 0)
        return false;

    m_transform_one_percent = 1.0f/(float)list.count();
    transform_stats ts = load_list_stats(list); //0 = raws

    if(!transform_valid(ts,true)) //validate with average
        return false;

    QVector<double> mean_trans;
    foreach(double val, list){
        mean_trans.append(range_transform(val,ts.min,ts.average,ts.max));
    }
    ts = load_list_stats(mean_trans); //1st transform (mean)

    if(!transform_valid(ts,false)) //test with median
        return false;

    QVector<double> median_trans;
    foreach(double val, mean_trans){
        median_trans.append(range_transform(val,ts.min,ts.median,ts.max));
    }
    ts = load_list_stats(median_trans); //2nd transform (median)
    const double trans_adjust = 0.5f - ts.average;

    QVector<double> median_adj_trans;
    foreach(double val, median_trans){
        median_adj_trans.append(val + trans_adjust);
    }
    ts = load_list_stats(median_adj_trans); //3rd transform (adjusted median)

    return true;
}

bool RoleStats::transform_valid(transform_stats ts, bool mid_is_avg){
    if(mid_is_avg)
        return (ts.min != ts.average && ts.average != ts.max);
    else
        return (ts.min != ts.median && ts.median != ts.max);
}

RoleStats::transform_stats RoleStats::load_list_stats(const QVector<double> list, bool save){
    transform_stats ts;
    ts.average = std::accumulate(list.begin(),list.end(),0.0) /  (double)list.count();
    ts.max = list.last();
    ts.min = list.first();
    ts.median = find_median(list);
    if(save)
        m_transformations.append(ts);
    return ts;
}

double RoleStats::range_transform(double val, double min, double mid, double max){
    if(val <= mid)
        return (val-min)/(mid-min)/2.0f;
    else
        return ((val-mid)/(max-mid)/2.0f)+0.5f;
}
