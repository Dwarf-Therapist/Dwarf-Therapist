#ifndef PLANDETAIL_H
#define PLANDETAIL_H

#include <QObject>
#include <QString>

class PlanDetail : QObject  {
    Q_OBJECT
public:
    PlanDetail()
    {
        labor_id = -1;
        role_name = "";
        priority = 0.0;
        ratio = 0.0;
        use_skill = false;
        assigned_laborers = 0;
        group_ratio = 0;
        m_max_count = 0;
        m_manual_count = false;
    }
    PlanDetail(const PlanDetail &pd)
        :QObject(pd.parent())
        , labor_id(pd.labor_id)
        , role_name(pd.role_name)
        , priority(pd.priority)
        , ratio(pd.ratio)
        , use_skill(pd.use_skill)
        , assigned_laborers(pd.assigned_laborers)
        , group_ratio(pd.group_ratio)
        , m_max_count(pd.m_max_count)
        , m_manual_count(pd.m_manual_count)
    {
    }

    int labor_id;
    QString role_name;
    float priority;
    float ratio; //ratio compared to other jobs
    bool use_skill; //set if a role isn't specified
    int assigned_laborers; //used when applying optimization
    float group_ratio;

    void set_max_count(int count, bool override){
        m_max_count = count;
        m_manual_count = override;
    }
    int get_max_count(){return m_max_count;}

    bool is_overridden(){return m_manual_count;}
    void is_overridden(bool val){m_manual_count = val;}

private:
    int m_max_count; //derived from the max_laborers * target population (if not manual)
    bool m_manual_count; //user set the count

};

#endif // PLANDETAIL_H
