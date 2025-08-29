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

#include "dwarfstats.h"
#include "rolestats.h"
#include "truncatingfilelogger.h"

#include <QSharedPointer>

float DwarfStats::m_att_pot_weight;
float DwarfStats::m_skill_rate_weight;
int DwarfStats::m_max_unit_kills;

DwarfStats DwarfStats::attributes(use_method_t<RoleStatsStratifiedMAD>{});
DwarfStats DwarfStats::attributes_raw(use_method_t<RoleStatsStratifiedMAD>{});
DwarfStats DwarfStats::skills(use_method_t<RoleStatsSkewed<RoleStatsStratifiedMAD>>{}, 0);
DwarfStats DwarfStats::facets(use_method_t<RoleStatsStratifiedMAD>{});
DwarfStats DwarfStats::beliefs(use_method_t<RoleStatsStratifiedMAD>{});
DwarfStats DwarfStats::needs(use_method_t<RoleStatsSkewed<RoleStatsStratifiedMAD>>{}, 0);
DwarfStats DwarfStats::preferences(use_method_t<RoleStatsSkewed<RoleStatsRankECDF>>{}, 0);
DwarfStats DwarfStats::roles(use_method_t<RoleStatsTransform>{});

template<typename T>
DwarfStats::DwarfStats(use_method_t<T>, double invalid_value)
    : m_stats(std::make_unique<T>(invalid_value))
{
}

double DwarfStats::calc_att_potential_value(int value, float max, float cti){
    double potential_value = 0.0;
    double diff = max - value;
    if(cti <= 0)
        cti = 1.0;
    double gap = diff * 500 / cti;

    //uses the cost to improve and the attributes maximum to apply a bonus
    //based on the difference between the attribute's current value and the maximum possible
    if(value >= max){
        potential_value = value;
    }else{
        potential_value = 0.5f * (1-(gap/diff));
        if(potential_value >= 0)
            potential_value += 0.5f;
        else
            potential_value = -0.5f / (potential_value - 1.0f);
        potential_value = value + (potential_value * gap);
    }
    return potential_value;
}

void DwarfStats::init(const QVector<double> &values)
{
    m_stats->set_list(values);
    if (DT->get_log_manager()->get_appender("core")->minimum_level() <= LL_VERBOSE)
        m_stats->log_stats(values);
}

double DwarfStats::rating(double val) const
{
    if (m_stats)
        return m_stats->get_rating(val);
    else
        return 0.5;
}

