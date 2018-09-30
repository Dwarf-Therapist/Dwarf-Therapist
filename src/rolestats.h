/*
Dwarf Therapist
Copyright (c) 2018 Clement Vuchener

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

#include <QVector>
#include <vector>

// Abstract base class for normalization methods.
class RoleStats
{
public:
    RoleStats(double invalid_value = -1);
    virtual ~RoleStats();

    virtual double get_rating(double val) const = 0;

    virtual void set_list(const QVector<double> &unsorted);

    void log_stats(const QVector<double> &unsorted) const;

protected:
    double m_invalid;
    int m_total_count;
    std::vector<double> m_valid;
    double m_median;
};

// Use the average of ECDF rank and Stratified MAD.
class RoleStatsRank: public RoleStats
{
public:
    RoleStatsRank(double invalid_value = -1);

    double get_rating(double val) const override;

    void set_list(const QVector<double> &unsorted) override;

private:
    double m_stratified_mad;
};

// Same as RoleStasRank but valid values ratings are transformed between 50%
// and 100%. Invalid value have ratings below 50%.
class RoleStatsRankSkewed: public RoleStatsRank
{
public:
    RoleStatsRankSkewed(double invalid_value);

    double get_rating(double val) const override;

    void set_list(const QVector<double> &unsorted) override;

private:
    double m_invalid_rating;
};

// Simply transform ratings centering around the median.
class RoleStatsTransform: public RoleStats
{
public:
    RoleStatsTransform(double invalid_value = -1);

    double get_rating(double val) const override;
};

#endif

