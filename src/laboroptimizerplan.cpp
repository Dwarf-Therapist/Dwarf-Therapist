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

#include "laboroptimizerplan.h"
#include "plandetail.h"

#include <QSettings>

laborOptimizerPlan::laborOptimizerPlan()
    :name("UNKNOWN")
{
    exclude_nobles = true;
    exclude_military = true;
    exclude_injured = true;
    exclude_squads = true;
    max_jobs_per_dwarf = 20;
    auto_haulers = true;
    pop_percent = 80.0f;
    hauler_percent = 50.0f;
}

laborOptimizerPlan::laborOptimizerPlan(QSettings &s, QObject *parent)
    : QObject(parent)
    , name(s.value("name", "UNKNOWN ROLE").toString())
    , exclude_nobles(s.value("nobles",true).toBool())
    , exclude_military(s.value("military",true).toBool())
    , exclude_injured(s.value("injured",true).toBool())
    , exclude_squads(s.value("squads",true).toBool())
    , max_jobs_per_dwarf(s.value("max_jobs_per_dwarf",20).toInt())
    , pop_percent(s.value("pop_percent",100).toFloat())
    , auto_haulers(s.value("auto_haulers",true).toBool())
    , hauler_percent(s.value("hauler_percent",50.0f).toFloat())
{
    read_details(s);
}

laborOptimizerPlan::laborOptimizerPlan(const laborOptimizerPlan &lop)
    :QObject(lop.parent())
{
    exclude_nobles = lop.exclude_nobles;
    exclude_military = lop.exclude_military;
    exclude_squads = lop.exclude_squads;
    exclude_injured = lop.exclude_injured;
    max_jobs_per_dwarf = lop.max_jobs_per_dwarf;
    auto_haulers = lop.auto_haulers;
    pop_percent = lop.pop_percent;
    hauler_percent = lop.hauler_percent;
    name = lop.name;
    foreach(PlanDetail *pd, lop.plan_details){
        PlanDetail *tmp = new PlanDetail(*pd);
        plan_details.append(tmp);
    }
}

laborOptimizerPlan::~laborOptimizerPlan(){
    qDeleteAll(plan_details);
}

void laborOptimizerPlan::read_details(QSettings &s){
    qDeleteAll(plan_details);
    plan_details.clear();

    int count = s.beginReadArray("jobs");
    for(int i=0; i<count; i++){
        PlanDetail *d = new PlanDetail();
        s.setArrayIndex(i);
        d->labor_id = s.value("labor_id").toInt();
        QString role_name = s.value("role_name").toString();
        d->role_name = role_name;

        if(role_name=="")
            d->use_skill = true;
        else
            d->use_skill = false;
        d->priority = s.value("priority").toFloat();
        if(s.contains("ratio")){
            d->ratio = s.value("ratio").toFloat();
            if(s.contains("max_laborers"))
                d->set_max_count(s.value("max_laborers").toFloat(),true);
        }else{
            d->ratio = s.value("max_laborers").toFloat(); //old version, ratio is max laborers
            d->set_max_count(-1,false);
        }
        d->group_ratio = 0.0;
        plan_details.append(d);
    }

    s.endArray();
}


void laborOptimizerPlan::write_to_ini(QSettings &s){
    //name
    s.setValue("name",name);
    s.setValue("nobles", exclude_nobles);
    s.setValue("military", exclude_military);
    s.setValue("squads", exclude_squads);
    s.setValue("injured", exclude_injured);
    s.setValue("max_jobs_per_dwarf", max_jobs_per_dwarf);
    s.setValue("auto_haulers", auto_haulers);
    s.setValue("pop_percent", QString::number(pop_percent,'g',2));
    s.setValue("hauler_percent", QString::number(hauler_percent,'g',2));

    if(plan_details.count() > 0){
        int count = 0;
        s.beginWriteArray("jobs", plan_details.count());
        foreach(PlanDetail *d, plan_details){
            s.setArrayIndex(count);

            s.setValue("labor_id", d->labor_id);
            s.setValue("role_name", d->role_name);
            s.setValue("priority", QString::number(d->priority,'g',6));
            s.setValue("ratio", QString::number(d->ratio, 'g', 4));
            if(d->is_overridden())
                s.setValue("max_laborers", QString::number(d->get_max_count(), 'g', 4));
            count ++;
        }
        s.endArray();
    }
}

PlanDetail *laborOptimizerPlan::job_exists(int labor_id){
    foreach(PlanDetail *d, plan_details){
        if(d->labor_id == labor_id)
            return d;
    }
    return 0;
}

void laborOptimizerPlan::remove_job(int labor_id){
    for(int i=0; i<plan_details.count(); i++){
        PlanDetail *d = plan_details.at(i);
        if(d->labor_id == labor_id){
            plan_details.remove(i);
            return;
        }
    }
}
