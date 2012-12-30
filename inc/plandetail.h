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
        max_count = 0;
        assigned_laborers = 0;
        group_ratio = 0;
    }

    int labor_id;
    QString role_name;
    float priority;
    float ratio; //ratio compared to other jobs
    bool use_skill; //set if a role isn't specified
    int max_count; //derived from the max_laborers * target population
    int assigned_laborers; //used when applying optimization
    float group_ratio;

};

#endif // PLANDETAIL_H
