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
#ifndef LABOROPTIMIZER_H
#define LABOROPTIMIZER_H

#include "plandetail.h"

#include <cmath>
#include <QObject>
#include <QVector>

class Dwarf;
class GameDataReader;
class laborOptimizerPlan;

class LaborOptimizer : public QObject {
    Q_OBJECT
public:
    LaborOptimizer(laborOptimizerPlan *m_plan, QObject *parent=0);
    virtual ~LaborOptimizer();

    void optimize_labors(QList<Dwarf*> dwarfs);

    void update_population(QList<Dwarf*>);
    void update_ratios();

    //getters
    int total_jobs() const {return m_total_jobs;}
    int total_raw_jobs() const {return m_raw_total_jobs;}
    int assigned_jobs() const {return m_estimated_assigned_jobs;}
    int total_population() const {return m_total_population;}
    int targeted_population() const {return roundf(m_target_population);}
public slots:
    void calc_population(bool load_labor_map = false);

signals:
    QString optimize_message(QVector<QPair<int, QString> >,bool is_warning = false);

protected:
    GameDataReader *gdr;
    laborOptimizerPlan *m_plan;
    QList<Dwarf*> m_dwarfs;
    float m_ratio_sum;

    float m_total_jobs;
    int m_raw_total_jobs;
    int m_estimated_assigned_jobs;
    int m_total_population; //total selected dwarves - excluded dwarves
    float m_target_population; //m_total_population * % population to use

    bool m_labors_exceed_pop;
    bool m_check_conflicts;

    struct dwarf_labor_map{
        float rating;
        Dwarf * d;
        PlanDetail *det;
    };

    struct compare_rating
    {
        bool operator() (dwarf_labor_map dlm1, dwarf_labor_map dlm2)
        {
            return (dlm2.rating < dlm1.rating);
        }
    };

    struct compare_priority_then_rating
    {
        bool operator() (dwarf_labor_map dlm1, dwarf_labor_map dlm2)
        {
            if(dlm2.det->priority < dlm1.det->priority)
                return true;
            else if(dlm2.det->priority > dlm1.det->priority)
                return false;
            else
                return (dlm2.rating < dlm1.rating);
        }
    };

    QVector<dwarf_labor_map> m_labor_map;
    QVector<QPair<int, QString> > m_current_message;

    void optimize();
};

#endif // LABOROPTIMIZER_H
