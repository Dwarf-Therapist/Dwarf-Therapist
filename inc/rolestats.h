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
#ifndef ROLESTATS_H
#define ROLESTATS_H

#include <QObject>
#include <QVector>
#include <math.h>
#include "ecdf.h"
#include "gamedatareader.h"
#include "truncatingfilelogger.h"

class RoleStats{

public:
    RoleStats(QString role_name){
        m_role_name = role_name;
        m_mean = -9999;
        m_log_mean = 0.0;
        m_ecdf = NULL;
        m_invalid_mean = false;
    }

    virtual ~RoleStats(){
        m_ecdf = 0;
        m_ecdf_ratings.clear();
        m_ratings.clear();
    }    

//    void calc_priority_adjustment(double log_mean_diff, double max_rank_log_mean, int total_role_count = -1){
//        if(m_mean == -9999){
//            double total = 0.0;
//            foreach(double r, m_ratings)
//                total += r;
//            m_mean = total / m_ratings.size();
//            m_log_mean = log(m_mean);
//            if(total_role_count <= 0)
//                total_role_count = GameDataReader::ptr()->get_roles().count();
//            m_rank_mean_perc = m_rank_mean / (double)total_role_count;
//            m_rank_log_mean = m_log_mean * m_rank_mean_perc;
//        }

//        m_priority_adjustment = (100.0f+((m_rank_log_mean / max_rank_log_mean) * log_mean_diff)-log_mean_diff) / 100.0f;
//        if(isinf(m_priority_adjustment) || isnan(m_priority_adjustment))
//            m_priority_adjustment = 0;

//        load_ecdf_data();
//    }
    void calc_priority_adjustment(int total_roles, int rank){
        m_priority_adjustment = (100.0f - (((double)rank/(double)total_roles+1)/100.0f)) / 100.0f;
        if(isinf(m_priority_adjustment) || isnan(m_priority_adjustment))
            m_priority_adjustment = 0;

        LOGD << "Role " << m_role_name << " Base Priority " << m_priority_adjustment;

        load_ecdf_data();
    }

    double adjust_role_rating(double raw_role_rating){
        if(m_invalid_mean){
            if(raw_role_rating > 0){
                LOGD << "*** Using alternate adjustment for role " << m_role_name
                     << " raw rating " << raw_role_rating << " new rating:" << ((raw_role_rating - m_min_rating) / (m_max_rating-m_min_rating));
            }
            return ((raw_role_rating - m_min_rating) / (m_max_rating-m_min_rating));
        }else{
            return (get_ecdf_rating(raw_role_rating) * m_priority_adjustment);
        }
    }

    double priority_adjustment(){return m_priority_adjustment;}

    void add_rating(double val){m_ratings.append(val);}

    void set_mean(double val){
        m_mean = val;
        m_log_mean = log(m_mean);
    }
    double get_mean(){return m_mean;}
    double get_rank_log_mean(){return m_rank_log_mean;}

    void set_mean_rank(int val, int total_role_count){
        m_rank_mean = val;
        m_rank_mean_perc = m_rank_mean / (double)total_role_count;
        m_rank_log_mean = m_log_mean * m_rank_mean_perc;
    }

    QString role_name(){return m_role_name;}

    static bool sort_means(const RoleStats *r1, const RoleStats *r2){return r1->m_mean > r2->m_mean;}

private:
    QString m_role_name;

    double m_mean;
    double m_rank_mean;
    double m_rank_mean_perc;

    double m_log_mean;
    double m_rank_log_mean;

    double m_priority_adjustment;

    double m_ecdf_rating_average;
    double m_max_rating;
    double m_min_rating;
    bool m_invalid_mean;

    QVector<double> m_ratings;
    QHash<QString,double> m_ecdf_ratings;
    ECDF *m_ecdf;

    void load_ecdf_data(){
        m_ecdf = new ECDF(m_ratings);
        m_max_rating = m_ecdf->sorted_data().last();
        m_min_rating = m_ecdf->sorted_data().first();
        if(m_max_rating == 0)
            m_max_rating = 1.0;
        double total = 0.0;
        foreach(double raw_rating, m_ratings){
            double val = m_ecdf->fplus(raw_rating);
            total += val;
            m_ecdf_ratings.insert(rating_key(raw_rating),val);
        }
        m_ecdf_rating_average = total / (double)m_ratings.count();
        LOGD << "Role:" << m_role_name << " ECDF Average:" << m_ecdf_rating_average;
        if(fabs(m_ecdf_rating_average-0.5) >= 0.2)
            m_invalid_mean = true;
    }

    double get_ecdf_rating(double raw_role_rating){
        if(m_ecdf == NULL)
            load_ecdf_data();

        if(m_ecdf_ratings.contains(rating_key(raw_role_rating)))
            return m_ecdf_ratings.value(rating_key(raw_role_rating));
        else
            return m_ecdf->fplus(raw_role_rating);
    }

    QString rating_key(double rating){
        return QString::number(rating,'f',8);
    }
};

#endif // ROLESTATS_H
