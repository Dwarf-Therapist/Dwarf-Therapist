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
#include "rolecalcminmax.h"
#include "rolecalcrecenter.h"
#include "truncatingfilelogger.h"

using std::unique_copy;
using std::distance;
using std::accumulate;

RoleStats::RoleStats(const QVector<double> &unsorted, const double invalid_value, const bool override)
    : m_null_rating(-1)
    , m_invalid(invalid_value)
    , m_override(override)
{
    set_list(unsorted);
}

void RoleStats::set_list(const QVector<double> &unsorted){
    m_total_count = static_cast<double>(unsorted.size());
    set_mode(unsorted);
}

void RoleStats::set_mode(const QVector<double> &unsorted){
    m_valid = unsorted;
    std::sort(m_valid.begin(), m_valid.end());
    bool skewed = false;
    double valid_size = m_valid.size();
    m_median = RoleCalcBase::find_median(m_valid);

    if(!m_override){
        double first_quartile = m_valid.at((int)m_valid.size()/4.0);
        skewed = (m_median == first_quartile);

        QVector<double>::iterator valid_start = m_valid.begin();
        if(m_valid.first() <= m_invalid){
            valid_start = std::upper_bound(m_valid.begin(),m_valid.end(),m_valid.first());
            m_valid.erase(m_valid.begin(),valid_start);
        }

        valid_size = static_cast<double>(m_valid.size());
        if(valid_size <= 0)
            return;

        if(skewed){
            //count of distinct valid values
            QVector<double> uniques(valid_size);
            QVector<double>::Iterator u_end;
            u_end = unique_copy(m_valid.begin(),m_valid.end(),uniques.begin());
            int unique_count = distance(uniques.begin(),u_end);
            float perc_unique = unique_count / valid_size;
            if(perc_unique < 0.25){
                //rankecdf
                m_calc = QSharedPointer<RoleCalcBase>(new RoleCalcBase(m_valid));
                LOGV << "     - using only ecdfrank";
            }else{
                //rankecdf and min/max
                m_calc = QSharedPointer<RoleCalcBase>(new RoleCalcMinMax(m_valid));
                LOGV << "     - using a combination of ecdfrank and minmax";
            }
        }else{
            //rankecdf and recenter
            LOGV << "     - using a combination of ecdfrank and recenter";
            m_calc = QSharedPointer<RoleCalcBase>(new RoleCalcRecenter(m_valid));
        }
    }else{
        LOGV << "     - using basic range transform";
    }

    bool print_debug_info = (DT->get_log_manager()->get_appender("core")->minimum_level() <= LL_VERBOSE);

    if(print_debug_info){
        RoleCalcBase tmp(m_valid);
        double tmp_total = 0;
        foreach(double val, m_valid){
            tmp_total += tmp.base_rating(val);
        }
        double tmp_avg = tmp_total / m_valid.size();
        LOGV << "     - base avg:" << tmp_avg;
    }

    double total = 0.0;
    if(skewed && !m_calc.isNull()){
        for(QVector<double>::const_iterator it = m_valid.begin()
            ; it != m_valid.end()
            ; it++){
            total += m_calc->rating(*it);
        }
        m_null_rating = ((m_total_count * 0.5f) - total) / (m_total_count - valid_size);
        LOGV << "     - null rating:" << m_null_rating;
    }

    if(print_debug_info){
        if(total <= 0){
            foreach(double val, m_valid){
                total += get_rating(val);
            }
        }
        if(m_null_rating != -1)
            total += ((m_total_count - valid_size) * m_null_rating);
        LOGV << "     - total raw values:" << m_total_count;
        LOGV << "     - median raw values:" << m_median;
        LOGV << "     - average of valid raw values:" << accumulate(m_valid.begin(),m_valid.end(),0.0) /  valid_size;
        LOGV << "     - min raw valid value:" << m_valid.first() << "max raw valid value:" << m_valid.last();
        LOGV << "     - min rating:" << (m_null_rating > 0 ? m_null_rating : get_rating(m_valid.first())) << "max rating:" << get_rating(m_valid.last());
        LOGV << "     - average of final ratings:" << (total / m_total_count);
        LOGV << "     ------------------------------";
    }
}

double RoleStats::get_rating(double val){
    if(!m_calc.isNull()){
        if(val <= m_invalid && m_null_rating != -1){
            return m_null_rating;
        }else{
            return m_calc->rating(val);
        }
    }else{
        if(m_override){
            return RoleCalcBase::range_transform(val,m_valid.first(),m_median,m_valid.last());
        }else{
            return 0.0;
        }
    }
}
