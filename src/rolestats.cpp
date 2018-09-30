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

#include "rolestats.h"

#include "truncatingfilelogger.h"

#include <algorithm>
#include <cmath>
#include <array>
#include <numeric>

// Value for M_SQRT2 (sqrt(2)) in GNU libc header.
static constexpr double Sqrt2 = 1.41421356237309504880;

// Cumulative Distribution Function
static double cdf(double x, double mean = 0.0, double sdev = 1.0)
{
    return (1.0 + std::erf((x - mean) / (sdev * Sqrt2))) / 2.0;
}

static double percentile(const std::vector<double> &vec, double k)
{
    // assume:
    //  - vec is sorted
    //  - 0.0 <= k <= 1.0
    auto index = (vec.size()-1) * k;
    auto index_floor = static_cast<unsigned int>(std::floor(index));
    auto dec_part = index - index_floor;
    if (dec_part == 0.0)
        return vec[index_floor];
    else
        return vec[index_floor] * (1.0-dec_part) + vec[index_floor+1] * dec_part;
}

// Empirical Cumulative Distribution Function
static double ecdf(const std::vector<double> &vec, double x)
{
    auto range = std::equal_range(vec.begin(), vec.end(), x);
    auto rank = [&vec] (auto it) { return static_cast<double>(std::distance(vec.begin(), it))/(vec.size()-1); };
    if (range.second == vec.begin())
        return 0.0;
    else if (range.first == vec.end())
        return 1.0;
    else
        return (rank(range.first) + rank(std::prev(range.second))) / 2.0;
}

static double range_transform(double val, double min, double mid, double max)
{
    if(val <= mid) {
        if(mid-min == 0)
            return 0;
        else
            return (val-min)/(mid-min)*0.5f;
    }
    else {
        if(max-mid == 0)
            return mid;
        else
            return ((val-mid)/(max-mid)*0.5f)+0.5f;
    }
}

RoleStats::RoleStats(double invalid_value)
    : m_invalid(invalid_value)
{
}

RoleStats::~RoleStats()
{
}

void RoleStats::set_list(const QVector<double> &unsorted)
{
    m_total_count = unsorted.size();
    m_valid.clear();
    if (m_invalid == -1) {
        m_valid.reserve(m_total_count);
        std::copy(unsorted.begin(), unsorted.end(), std::back_inserter(m_valid));
    }
    else {
        for (auto val: unsorted)
            if (val > m_invalid)
                m_valid.push_back(val);
    }
    std::sort(m_valid.begin(), m_valid.end());
    m_median = percentile(m_valid, 0.5);

    bool print_debug_info = (DT->get_log_manager()->get_appender("core")->minimum_level() <= LL_VERBOSE);
    if (print_debug_info) {
        auto average = std::accumulate(m_valid.begin(), m_valid.end(), 0.0) / m_valid.size();
        auto min = m_valid.front();
        auto max = m_valid.back();
        LOGV << "     - total raw values:" << m_total_count;
        LOGV << "     - valid:" << m_valid.size() << "/" << m_total_count;
        LOGV << "     - median of valid raw values:" << m_median;
        LOGV << "     - average of valid raw values:" << average;
        LOGV << "     - min raw valid value:" << min << "max raw valid value:" << max;
    }
}

void RoleStats::log_stats(const QVector<double> &unsorted) const
{
    auto min = +std::numeric_limits<double>::infinity(),
         max = -std::numeric_limits<double>::infinity(),
         average = 0.0;
    for (auto val: unsorted) {
        auto r = get_rating(val);
        if (r < min)
            min = r;
        if (r > max)
            max = r;
        average += r;
    }
    average /= unsorted.size();
    auto min_valid = get_rating(m_valid.front());
    LOGV << "     - min rating:" << min << "min valid rating:" << min_valid  << "max rating:" << max;
    LOGV << "     - average of final ratings:" << average;
    LOGV << "     ------------------------------";
}

RoleStatsRank::RoleStatsRank(double invalid_value)
    : RoleStats(invalid_value)
{
}

double RoleStatsRank::RoleStatsRank::get_rating(double val) const
{
    return (ecdf(m_valid, val) + cdf(val, m_median, m_stratified_mad)) / 2.0;
}

void RoleStatsRank::set_list(const QVector<double> &unsorted)
{
    RoleStats::set_list(unsorted);

    // Compute Stratified MAD (dark magic from Thistleknot)
    std::array<double, 7> k = { 0.0, 0.1, 0.25, 0.5, 0.75, 0.9, 1.0}, p;
    std::transform(
            k.begin(), k.end(),
            p.begin(),
            [this] (double k) {
                auto p = percentile(m_valid, k) - m_median;
                return p * std::fabs(p);
            });
    m_stratified_mad = 0.0;
    for (auto i = 0u; i < k.size()-1; ++i)
        m_stratified_mad += (k[i+1]-k[i])*(p[i+1]-p[i]);
    m_stratified_mad = std::sqrt(m_stratified_mad);
    LOGV << "     - stratified MAD:" << m_stratified_mad;
}

RoleStatsRankSkewed::RoleStatsRankSkewed(double invalid_value)
    : RoleStatsRank(invalid_value)
{
}

double RoleStatsRankSkewed::get_rating(double val) const
{
    if (val <= m_invalid)
        return m_invalid_rating;
    else
        return 0.5 + RoleStatsRank::get_rating(val) / 2.0;
}

void RoleStatsRankSkewed::set_list(const QVector<double> &unsorted)
{
    RoleStatsRank::set_list(unsorted);

    // Set invalid rating so the average rating is 50%
    double valid_total = std::accumulate(
            m_valid.begin(), m_valid.end(), 0.0,
            [this] (double total, double val) {
                return total + get_rating(val);
            });
    m_invalid_rating = ((m_total_count * 0.5) - valid_total) / (m_total_count - m_valid.size());
    if (m_invalid_rating < 0.0)
        m_invalid_rating = 0.0;
    LOGV << "     - invalid rating:" << m_invalid_rating;
}

RoleStatsTransform::RoleStatsTransform(double invalid_value)
    : RoleStats(invalid_value)
{
}

double RoleStatsTransform::get_rating(double val) const
{
    return range_transform(val, m_valid.front(), m_median, m_valid.back());
}
