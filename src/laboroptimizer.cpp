/*
Dwarf Therapist
Copyright (c) 2010 Justin Ehlert

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

/*
Entirely based on Thistleknot's optimization algorithm.

A list of dwarves is created for the optimization plan's jobs, multiplying the role or skill rating associated with the job by the priority specified.
Then the amount of workers to assign to each job is calculated by using the ratio's set for each job. The higher the ratio, the more workers assigned.
Finally the list of dwarves and each job's rating are sorted by the modified rating (based on priority), and is looped through,
assigning the amount of workers based on the ratio calculation.

After everything is optimized, any haulers are assigned with less than the specified amount of labors.
*/

#include "laboroptimizer.h"
#include "laboroptimizerplan.h"
#include "plandetail.h"
#include <algorithm>
#include "gamedatareader.h"
#include "dwarftherapist.h"
#include "dwarf.h"
#include "labor.h"
#include "skill.h"
#include "rolestats.h"

LaborOptimizer::LaborOptimizer(laborOptimizerPlan *plan, QObject *parent)
    :QObject(parent)
    , m_plan(plan)
    , m_ratio_sum(0)
    , m_total_jobs(0)
    , m_raw_total_jobs(0)
    , m_estimated_assigned_jobs(0)
    , m_total_population(0)
    , m_target_population(0)
    , m_labors_exceed_pop(false)
{
    m_check_conflicts = DT->user_settings()->value("options/labor_exclusions",true).toBool();
    gdr = GameDataReader::ptr();
}

LaborOptimizer::~LaborOptimizer(){
    gdr = 0;
    for(int i = 0; i < m_labor_map.count(); i++){
        m_labor_map[i].d = 0;
        m_labor_map[i].det = 0;
    }
    m_labor_map.clear();

    m_dwarfs.clear();
}

void LaborOptimizer::calc_population(bool load_labor_map){
    //clear the labor map
    m_labor_map.clear();
    m_total_population = 0;

    //setup our new map
    m_current_message.clear();
    for(int i = m_dwarfs.count()-1; i >= 0; i--){
        Dwarf *d = m_dwarfs.at(i);
        if(!d || d->is_animal())
            continue;

        //exclude nobles, hospitalized dwarfs, children, babies and militia
        if(d->noble_position() != "" && m_plan->exclude_nobles){
            m_current_message.append(QPair<int, QString> (d->id(), tr("(Noble) %1").arg(d->nice_name())));
            m_dwarfs.removeAt(i);
        }
        else if(d->current_job_id() == 52 && m_plan->exclude_injured){
            m_current_message.append(QPair<int, QString> (d->id(), tr("(Hospitalized) %1").arg(d->nice_name())));
            if(load_labor_map)
                m_dwarfs.at(i)->clear_labors();
            m_dwarfs.removeAt(i);
        }
        else if(d->is_baby()){
            m_dwarfs.removeAt(i);
        }
        else if(d->active_military() && m_plan->exclude_military){
            m_current_message.append(QPair<int, QString> (d->id(), tr("(Active Duty) %1").arg(d->nice_name())));
            if(load_labor_map)
                m_dwarfs.at(i)->clear_labors();
            m_dwarfs.removeAt(i);
        }
        else if(d->squad_id() > -1 && m_plan->exclude_squads){
            m_current_message.append(QPair<int, QString> (d->id(), tr("(Squad) %1, %2").arg(d->nice_name()).arg(d->squad_name())));
            if(load_labor_map)
                m_dwarfs.at(i)->clear_labors();
            m_dwarfs.removeAt(i);
        }
        else if(d->is_child() && !DT->labor_cheats_allowed()){
            m_current_message.append(QPair<int, QString> (d->id(), tr("(Child) %1").arg(d->nice_name())));
            m_dwarfs.removeAt(i);
        }
        else{
            m_total_population++;
            if(load_labor_map){
                //clear dwarf's existing labors
                d->clear_labors();
                d->optimized_labors = 0;
                foreach(PlanDetail *det, m_plan->plan_details){
                    //skip labor with <= 0 priority/max workers
                    if(det->priority > 0 && det->ratio > 0){
                        dwarf_labor_map dlm;
                        dlm.d = d;
                        dlm.det = det;
                        if(!det->role_name.isEmpty()){
                            dlm.rating = d->get_role_rating(det->role_name) * det->priority;
                        }else{
                            dlm.rating = d->get_skill(GameDataReader::ptr()->get_labor(dlm.det->labor_id)->skill_id).get_rating(true) * 100.0f * det->priority;
                        }
                        m_labor_map.append(dlm);
                    }
                }
            }
        }
    }

    m_target_population = (m_plan->pop_percent/(float)100) * (float)m_total_population;

    if(m_current_message.count() > 0){
        m_current_message.push_front(QPair<int,QString>(0,tr("%1 worker%2 excluded from optimization.")
                                                        .arg(QString::number(m_current_message.count()))
                                                        .arg(m_current_message.count() > 1 ? "s" : "")));
        if(load_labor_map)
            emit optimize_message(m_current_message);
    }
}


void LaborOptimizer::optimize_labors(QList<Dwarf*> dwarfs){
    m_dwarfs = dwarfs;
    if(m_dwarfs.count() > 0){
        optimize();

        m_current_message.clear();
        m_current_message.append(QPair<int,QString>(0,tr("Optimization Complete.")));
        emit optimize_message(m_current_message);
    }
}

void LaborOptimizer::optimize(){
    //create the labor mapping
    calc_population(true);
    std::sort(m_labor_map.begin(),m_labor_map.end(),LaborOptimizer::compare_rating());

    update_ratios();

    QHash<int, Dwarf*> haulers;
    foreach(Dwarf *d, m_dwarfs){
        haulers.insert(d->id(),d);
    }

    //optimize
    bool has_conficting_labor = false;
    Labor *l;

    QList<int> m_missing_roles;
    foreach(dwarf_labor_map dlm, m_labor_map){
        l = gdr->get_labor(dlm.det->labor_id);
        //check conflicting labors
        if (m_check_conflicts) {
            has_conficting_labor = false;
            foreach(int excluded, l->get_excluded_labors()) {
                if(dlm.d->labor_enabled(excluded)){
                    has_conficting_labor = true;
                    break;
                }
            }
            if(dlm.det->use_skill && !m_missing_roles.contains(dlm.det->labor_id))
                m_missing_roles.append(dlm.det->labor_id);
        }
        //dwarf has available labor slots? target laborers reached?
        if(!has_conficting_labor && dlm.d->optimized_labors < plan->max_jobs_per_dwarf && dlm.det->assigned_laborers < dlm.det->get_max_count()){

            LOGD << "Job:" << GameDataReader::ptr()->get_labor(dlm.det->labor_id)->name << " Role:" << dlm.det->role_name << " Dwarf:" << dlm.d->nice_name()
                 << " Rating:" << dlm.rating << " Raw Rating:" << dlm.d->get_raw_role_rating(dlm.det->role_name);

            dlm.d->set_labor(dlm.det->labor_id, true, false);
            dlm.det->assigned_laborers++;
            dlm.d->optimized_labors++;
        }

        if(m_plan->auto_haulers){
            if(dlm.d->optimized_labors >= roundf((float)m_plan->max_jobs_per_dwarf * (m_plan->hauler_percent/(float)100)))
                haulers.remove(dlm.d->id());
        }
    }

    if(m_missing_roles.count() > 0){
        m_current_message.clear();
        m_current_message.append(QPair<int,QString>(0,tr("%1 labors are missing roles and may not have accurate ratings!").arg(QString::number(m_missing_roles.count()))));
        foreach(int id, m_missing_roles){
            m_current_message.append(qMakePair(id,gdr->ptr()->get_labor(id)->name));
        }
        emit optimize_message(m_current_message,true);
    }

    //get a list of skill-less labors
    QList<Labor*> skill_less_jobs = gdr->get_ordered_labors();
    for(int i = skill_less_jobs.count()-1; i >= 0; i--){
        if(!skill_less_jobs.at(i)->is_hauling)
            skill_less_jobs.removeAt(i);
    }

    if(m_plan->auto_haulers){
        foreach(int id, haulers.uniqueKeys()){
            Dwarf *d = haulers.value(id);
            foreach(l, skill_less_jobs){
                d->set_labor(l->labor_id, true, false);
            }
        }
        m_current_message.clear();
        m_current_message.append(QPair<int,QString>(0,QString::number(haulers.count()) + " haulers have been assigned."));
        emit optimize_message(m_current_message);
    }
}

void LaborOptimizer::update_ratios(){
    m_ratio_sum = 0;
    m_estimated_assigned_jobs = 0;
    m_total_jobs = m_target_population * m_plan->max_jobs_per_dwarf;
    m_raw_total_jobs = m_total_jobs;

    //get ratio sum
    int count = 0;
    foreach(PlanDetail *det, plan->plan_details){
        if(!det->is_overridden()) //don't clear overridden counts
            det->set_max_count(0,false);
        det->group_ratio = 0;
        det->assigned_laborers = 0;
        if(det->priority > 0 && det->ratio > 0)
            m_ratio_sum += det->ratio;

        count++;
    }

    m_labors_exceed_pop = false;
    if(m_check_conflicts){
        PlanDetail *temp;
        foreach(PlanDetail *det, m_plan->plan_details){
            if(det->priority > 0 && det->ratio > 0 && det->group_ratio <= 0){

                if(gdr->get_labor(det->labor_id)->get_excluded_labors().count() > 0){
                    //increase this labor's group ratio
                    det->group_ratio += det->ratio;
                    foreach(int id, gdr->get_labor(det->labor_id)->get_excluded_labors()){
                        temp =  m_plan->job_exists(gdr->get_labor(id)->labor_id);
                        if(temp)
                            det->group_ratio += temp->ratio;
                    }
                    //set all related labors to the ratio total we just calculated
                    foreach(int id, gdr->get_labor(det->labor_id)->get_excluded_labors()){
                        temp =  m_plan->job_exists(gdr->get_labor(id)->labor_id);
                        if(temp)
                            temp->group_ratio = det->group_ratio;
                    }
                    if(det->group_ratio / m_ratio_sum * m_total_jobs > m_total_population){
                        m_ratio_sum -= det->group_ratio;
                        m_labors_exceed_pop = true;
                    }
                }
            }
        }
        if(m_labors_exceed_pop)
            m_total_jobs -= m_total_population;
    }


    //if sum of job's coverage + conflicting job's coverage / total coverage > target population
    //job's max count = job's coverage / sum(job's coverage + conflicting coverages) * target population
    foreach(PlanDetail *det, m_plan->plan_details){
        if(det->priority > 0 && det->ratio > 0){
            if(!det->is_overridden()){
                if(det->group_ratio > 0 && labors_exceed_pop){
                    det->set_max_count(roundf(det->ratio / det->group_ratio * m_total_population),false);
                }else{
                    det->set_max_count(roundf(det->ratio / m_ratio_sum * m_total_jobs),false);
                }
                if(det->get_max_count() > m_total_population)
                    det->set_max_count(m_total_population,false);
            }else{
                float new_ratio = 0.0;
                if(det->group_ratio > 0 && labors_exceed_pop){
                    new_ratio = det->get_max_count() * det->group_ratio / m_total_population;
                }else{
                    new_ratio = det->get_max_count()  * m_ratio_sum / m_total_jobs;
                }
                if(new_ratio > 100.0)
                    new_ratio = 100.0;
                if(new_ratio <= 0)
                    new_ratio = 0.01;
                det->ratio = new_ratio;
            }
        }
        m_estimated_assigned_jobs += det->get_max_count();
    }
}

void LaborOptimizer::update_population(QList<Dwarf*> m){
    m_dwarfs = m;
}


