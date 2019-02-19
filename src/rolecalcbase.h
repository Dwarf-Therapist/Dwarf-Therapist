#ifndef ROLECALCBASE_H
#define ROLECALCBASE_H

#include <QVector>

class RoleCalcBase{
  public:
    RoleCalcBase(const QVector<double> &sorted);
    virtual ~RoleCalcBase();

    virtual double rating(const double val);
    double base_rating(const double val);

    static double find_median(QVector<double> v);
    static double range_transform(double val, double min, double mid, double max);

  protected:
    QVector<double> m_sorted;
    QVector<double>::const_iterator m_begin, m_end;
    void init_list();
    double m_count;
    double m_div;
  };
#endif // ROLECALCBASE_H
