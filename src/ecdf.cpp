/*
  Copyright (C) 2007 Steven L. Scott

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#include "ecdf.h"
#include <algorithm>

typedef QVector<double> VEC;
typedef VEC::iterator IT;
typedef VEC::const_iterator CIT;
using std::sort;
using std::upper_bound;
using std::lower_bound;

ECDF::ECDF(){
}

ECDF::ECDF(const VEC & unsorted)
    : m_sorted(unsorted)
{
    init_list();
}

void ECDF::set_list(const QVector<double> &unsorted){
    m_sorted.clear();
    m_sorted = unsorted;
    m_skew = false;
    m_skew_padding = 0.0;
    init_list();
}

void ECDF::init_list(){
    qSort(m_sorted.begin(), m_sorted.end());
    b = m_sorted.begin();
    e = m_sorted.end();
    n = static_cast<double>(m_sorted.size());
//    m_min_count = m_sorted.lastIndexOf(0)+1;//m_sorted.lastIndexOf(m_sorted.first())+1;
//    m_other_count = m_sorted.count() - m_min_count;

//    if((float)m_min_count / (float)m_sorted.count() > 0.5f){
//        m_skew = true;
//        double total = 0.0;
//        foreach(double val, m_sorted){
//            if(val != 0)
//                total += fplus(val);
//        }
//        m_skew_padding = 0.5-(total / m_sorted.count()/2);
//    }
}

double ECDF::fplus(double x)const{
    CIT it = upper_bound(b, e, x);
    unsigned pos = it-b;  // it is the first element >= x        
    return (pos/n);
//    if(!m_skew){
//        return pos/n;
//    }else{
//        if(x <= 0){
//            return m_skew_padding;
//        }else{
//            return (((pos-(float)m_min_count)/(float)m_other_count) / 2) + m_skew_padding;
//        }
//    }
}

//double ECDF::fplus_deskew(double x)const{
//    if(x == 0)
//        return 0;
//    CIT it = upper_bound(b, e, x);
//    unsigned pos = it-b;  // it is the first element >= x
//    double ret = (pos-(float)m_zero_counts)/(float)m_non_zero_counts;
//    return ret;
//}

double ECDF::fminus(double x)const{
    CIT it = lower_bound(b,e, x);
    unsigned pos = it-b;
    return pos/n;
}

double ECDF::favg(double x) const{
    return ((fplus(x)+fminus(x))/2.0f);
}
