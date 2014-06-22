/*
  Copyright (C) 2007 Steven L. Scott

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/
#ifndef ECDF_H
#define ECDF_H

#include <QVector>

  class ECDF{
    // empirical CDF
  public:
    ECDF(const QVector<double> &unsorted);
    ECDF();
    void set_list(const QVector<double> &unsorted);
    double fplus(double x)const;  // fraction of data <= x
    double fplus_deskew(double)const;
    double fminus(double x)const; // fraction of data < x;
    double operator()(double x, bool leq = true)const{
      return leq ? fplus(x) : fminus(x);}
    const QVector<double> & sorted_data()const{return m_sorted;}
  private:
    QVector<double> m_sorted;
    QVector<double>::const_iterator b, e;
    void init_list();
    double n;
    int m_zero_counts;
    int m_non_zero_counts;
  };

#endif // ECDF_H
