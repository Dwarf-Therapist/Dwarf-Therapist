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

class RoleStats{

public:
    RoleStats(QString role_name){
        m_role_name = role_name;
        m_mean = -9999;
        m_log_mean = 0.0;
        m_ecdf = NULL;
    }

    virtual ~RoleStats(){
        m_ecdf = 0;
    }

    void calc_priority_adjustment(double log_mean_diff, double max_rank_log_mean){
        if(m_mean == -9999){
            double total = 0.0;
            foreach(double r, m_ratings)
                total += r;
            m_mean = total / m_ratings.size();
            m_log_mean = log(m_mean);
            m_rank_mean_perc = m_rank_mean / (double)GameDataReader::ptr()->get_roles().count();
            m_rank_log_mean = m_log_mean * m_rank_mean_perc;
        }

        m_priority_adjustment = (100.0f+((m_rank_log_mean / max_rank_log_mean) * log_mean_diff)-log_mean_diff) / 100.0f;
        if(isinf(m_priority_adjustment) || isnan(m_priority_adjustment))
            m_priority_adjustment = 0;
    }

    double adjusted_role_rating(double role_rating){
        return (calc_ecdf(role_rating) * m_priority_adjustment);
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

    QVector<double> m_ratings;
    ECDF *m_ecdf;

    double calc_ecdf(double role_rating){
        if(m_ecdf == NULL){
            m_ecdf = new ECDF(m_ratings);
        }
        return m_ecdf->fplus(role_rating);
    }
};

#endif // ROLESTATS_H
