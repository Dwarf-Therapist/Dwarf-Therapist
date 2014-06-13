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
    RoleStats(QString role_name);
    virtual ~RoleStats();

    void calc_priority_adjustment(int total_roles, int rank);
    double adjust_role_rating(double raw_role_rating);

    double priority_adjustment(){return m_priority_adjustment;}
    void add_rating(double val){m_ratings.append(val);}

    void set_mean(double val){m_mean = val;}
    double get_mean(){return m_mean;}

    QString role_name(){return m_role_name;}

    static bool sort_means(const RoleStats *r1, const RoleStats *r2){return r1->m_mean > r2->m_mean;}

private:
    QString m_role_name;
    double m_mean;
    double m_priority_adjustment;
    double m_ecdf_rating_average;
    double m_max_rating;
    double m_min_rating;
    bool m_use_alt_conversion;

    QVector<double> m_ratings;
    QHash<QString,double> m_ecdf_ratings;
    ECDF *m_ecdf;

    void load_ecdf_data();
    double get_ecdf_rating(double raw_role_rating);
    QString rating_key(double rating);
};

#endif // ROLESTATS_H
