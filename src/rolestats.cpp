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


RoleStats::RoleStats(QString role_name){
    m_role_name = role_name;
    m_mean = 0;
    m_priority_adjustment = 0;
    m_ecdf_rating_average = 0;
    m_max_rating = 0;
    m_min_rating = 0;
    m_use_alt_conversion = false;
    m_ecdf = NULL;
}

RoleStats::~RoleStats(){
    m_ecdf = 0;
    m_ecdf_ratings.clear();
    m_ratings.clear();
}

void RoleStats::calc_priority_adjustment(int total_roles, int rank){
    m_priority_adjustment = (100.0f - (((double)rank/(double)total_roles+1)/100.0f)) / 100.0f;
    if(isinf(m_priority_adjustment) || isnan(m_priority_adjustment))
        m_priority_adjustment = 0;

    LOGD << "Role " << m_role_name << " Base Priority " << m_priority_adjustment;

    load_ecdf_data();
}

double RoleStats::adjust_role_rating(double raw_role_rating){
    double new_rating = 0.0;
    if(m_use_alt_conversion){
        if((m_max_rating - m_min_rating) != 0)
            new_rating = (raw_role_rating - m_min_rating) / (m_max_rating-m_min_rating);
        if(raw_role_rating > 0){
            LOGD << "*** Using alternate adjustment for role " << m_role_name << " raw rating " << raw_role_rating << " new rating:" << new_rating;
        }
    }else{
        new_rating = get_ecdf_rating(raw_role_rating) * m_priority_adjustment;
    }
    return new_rating;
}

void RoleStats::load_ecdf_data(){
    m_ecdf = new ECDF(m_ratings);
    m_max_rating = m_ecdf->sorted_data().last();
    m_min_rating = m_ecdf->sorted_data().first();
    double total = 0.0;
    foreach(double raw_rating, m_ratings){
        double val = m_ecdf->fplus(raw_rating);
        total += val;
        m_ecdf_ratings.insert(rating_key(raw_rating),val);
    }
    m_ecdf_rating_average = total / (double)m_ratings.count();
    LOGD << "Role:" << m_role_name << " ECDF Average:" << m_ecdf_rating_average;
    if(fabs(m_ecdf_rating_average-0.5) >= 0.275){
        double max_freq = 0.0;
        int max_idx = 0;
        int freq = 1;
        float limit = m_ratings.count() * 0.5;
        for(int idx=0;idx<m_ecdf->sorted_data().count()-1;idx++){
            if(m_ecdf->sorted_data().at(idx) == m_ecdf->sorted_data().at(idx+1)){
                freq++;
                if(freq > max_freq){
                    max_freq = freq;
                    max_idx = idx;
                    if(max_freq > limit)
                        break;
                }
            }else{
                freq = 1;
            }
        }
        if((max_freq / m_ratings.count()) > 0.50){
            m_use_alt_conversion = true;
            LOGD << "Using alt method for role " << m_role_name << " due to excessive frequency of rating " << m_ecdf->sorted_data().at(max_idx);
        }
    }
}

double RoleStats::get_ecdf_rating(double raw_role_rating){
    if(m_ecdf == NULL)
        load_ecdf_data();

    if(m_ecdf_ratings.contains(rating_key(raw_role_rating)))
        return m_ecdf_ratings.value(rating_key(raw_role_rating));
    else
        return m_ecdf->fplus(raw_role_rating);
}

QString RoleStats::rating_key(double rating){
    return QString::number(rating,'f',8);
}
