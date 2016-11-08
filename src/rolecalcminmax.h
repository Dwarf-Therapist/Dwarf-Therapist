#ifndef ROLECALCMINMAX_H
#define ROLECALCMINMAX_H

#include <QVector>
#include "rolecalcbase.h"

class RoleCalcMinMax: public RoleCalcBase{
  public:
    RoleCalcMinMax(const QVector<double> &sorted)
        : RoleCalcBase(sorted)
        , m_min(sorted.first())
        , m_max(sorted.last())
    {
        m_diff = m_max - m_min;
        if(m_diff == 0)
            m_diff = 1;
    }

    double rating(const double val){
        return (base_rating(val) + calc_min_max(val)) * 0.25 + 0.5f;
    }

private:
    double m_min;
    double m_max;
    double m_diff;
    double calc_min_max(double val){
        return (val - m_min) / m_diff;
    }
  };
#endif // ROLECALCMINMAX_H
