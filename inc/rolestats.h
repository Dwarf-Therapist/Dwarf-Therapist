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
#include "rolecalcbase.h"
#include "gamedatareader.h"
#include "truncatingfilelogger.h"

class RoleStats{

public:
    RoleStats(const QVector<double> &unsorted, const double invalid_value = -1, const bool override = false);
    virtual ~RoleStats()
    {}

    double get_rating(double val);
    void set_list(const QVector<double> &unsorted);

private:
    double m_total_count;
    double m_null_rating;
    double m_invalid;
    bool m_override; //use a very simple range transform only
    double m_median;

    QSharedPointer<RoleCalcBase> m_calc;
    QVector<double> m_valid;
    void set_mode(const QVector<double> &unsorted);
};

#endif // ROLESTATS_H
