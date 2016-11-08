#ifndef ROLECALCRECENTER_H
#define ROLECALCRECENTER_H

#include <QVector>
#include "rolecalcbase.h"
#include <numeric>

class RoleCalcRecenter: public RoleCalcBase{
public:
    RoleCalcRecenter(const QVector<double> &sorted)
        : RoleCalcBase(sorted)
    {
        m_avg = std::accumulate(m_begin,m_end,0.0) /  (double)m_count;
        recenter_list();
    }

    double rating(const double val){
        double adjusted_val = range_transform(val,m_sorted.first(),m_avg,m_sorted.last());
        adjusted_val = range_transform(adjusted_val,0,m_adj_median,1.0f);
        return (base_rating(val) + adjusted_val) * 0.5f;
    }

private:
    double m_adj_median;
    double m_avg;

    void recenter_list(){
        QVector<double> m_adjusted;
        double min = m_sorted.first();
        double max = m_sorted.last();
        foreach(double val, m_sorted){
            m_adjusted.append(range_transform(val,min,m_avg,max));
        }
        m_adj_median = find_median(m_adjusted);
    }
};
#endif // ROLECALCRECENTER_H
