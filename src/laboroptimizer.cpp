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
    , plan(plan)        
{    
    check_conflicts = DT->user_settings()->value("options/labor_exclusions",true).toBool();
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

    if(load_labor_map)
        adjust_ratings();

    //setup our new map
    m_current_message.clear();
    for(int i = m_dwarfs.count()-1; i >= 0; i--){
        Dwarf *d = m_dwarfs.at(i);
        if(!d || d->is_animal())
            continue;

        //exclude nobles, hospitalized dwarfs, children, babies and militia
        if(d->noble_position() != "" && plan->exclude_nobles){
            m_current_message.append(QPair<int, QString> (d->id(), tr("(Noble) %1").arg(d->nice_name())));
            m_dwarfs.removeAt(i);
        }
        else if(d->current_job_id() == 52 && plan->exclude_injured){
            m_current_message.append(QPair<int, QString> (d->id(), tr("(Hospitalized) %1").arg(d->nice_name())));
            if(load_labor_map)
                m_dwarfs.at(i)->clear_labors();
            m_dwarfs.removeAt(i);
        }
        else if(d->is_baby()){
            m_dwarfs.removeAt(i);
        }
        else if(d->active_military() && plan->exclude_military){
            m_current_message.append(QPair<int, QString> (d->id(), tr("(Active Duty) %1").arg(d->nice_name())));
            if(load_labor_map)
                m_dwarfs.at(i)->clear_labors();
            m_dwarfs.removeAt(i);
        }
        else if(d->squad_id() > -1 && plan->exclude_squads){
            m_current_message.append(QPair<int, QString> (d->id(), tr("(Squad) %1").arg(d->nice_name()).arg(d->squad_name())));
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
                foreach(PlanDetail *det, plan->plan_details){
                    //skip labor with <= 0 priority/max workers
                    if(det->priority > 0 && det->ratio > 0){
                        dwarf_labor_map dlm;
                        dlm.d = d;
                        dlm.det = det;
                        if(!det->role_name.isEmpty()){
                            //dlm.rating = d->get_role_rating(det->role_name, true);//det->priority * d->get_role_rating(det->role_name, true);
                            dlm.rating = d->get_adjusted_role_rating(det->role_name) * det->priority;
                        }
                        else{                            
                            dlm.rating = d->get_skill(GameDataReader::ptr()->get_labor(dlm.det->labor_id)->skill_id).capped_exp() / (float)MAX_CAPPED_XP * 100 * det->priority;//det->priority * (d->get_skill(GameDataReader::ptr()->get_labor(dlm.det->labor_id)->skill_id).capped_exp() / (float)MAX_CAPPED_XP * 100);
                        }
                        m_labor_map.append(dlm);
                    }
                }
            }
        }
    }

    m_target_population = (plan->pop_percent/(float)100) * (float)m_total_population;

    if(m_current_message.count() > 0){
        m_current_message.push_front(QPair<int,QString>(0,tr("%1 worker%2 excluded from optimization.")
                                                        .arg(QString::number(m_current_message.count()))
                                                        .arg(m_current_message.count() > 1 ? "s" : "")));
        if(load_labor_map)
            emit optimize_message(m_current_message);
    }
}

void LaborOptimizer::adjust_ratings(){
    QList<RoleStats*> m_role_stats;
    QHash<QString, RoleStats*> m_role_stats_by_name;
    RoleStats *rs;
    int num_roles = 0;
    double total = 0.0;

    //loop through each unique role, and load up the ratings into stat objects
    foreach(PlanDetail *det, plan->plan_details){
        if(!det->role_name.trimmed().isEmpty() && !m_role_stats_by_name.contains(det->role_name)){
            rs = new RoleStats(det->role_name);
            total = 0;
            foreach(Dwarf *d, m_dwarfs){                
                double rating = d->get_role_rating(det->role_name,true);
                total += rating;
                rs->add_rating(rating);
            }
            //since we're looping through all the ratings now anyway
            //set the mean of the raw ratings at the same time
            rs->set_mean(total / (double)m_dwarfs.count());
            m_role_stats.append(rs);
            m_role_stats_by_name.insert(det->role_name,rs);
        }
    }

    qSort(m_role_stats.begin(),m_role_stats.end(),&RoleStats::sort_means);
    num_roles = m_role_stats.count();
    int rank = 1;
    foreach(RoleStats *rs, m_role_stats){
        rs->calc_priority_adjustment(num_roles, rank);
        foreach(Dwarf *d, m_dwarfs){
            d->set_adjusted_role_rating(rs->role_name(), m_role_stats_by_name.value(rs->role_name())->adjust_role_rating(d->get_role_rating(rs->role_name(),true))*100.0f);
        }
        rank ++;
    }

    qDeleteAll(m_role_stats);
    m_role_stats.clear();
    m_role_stats_by_name.clear();
}

void LaborOptimizer::optimize_labors(QList<Dwarf*> dwarfs){
    m_dwarfs = dwarfs;
    if(m_dwarfs.count() > 0){
//        QFuture<void> f = QtConcurrent::run(this, &LaborOptimizer::optimize);
//        f.waitForFinished();
        optimize();

        m_current_message.clear();
        m_current_message.append(QPair<int,QString>(0,tr("Optimization Complete.")));
        emit optimize_message(m_current_message);
    }
}

void LaborOptimizer::optimize(){
    //create the labor mapping
    calc_population(true);

    //sort list by the weighted rating
    std::sort(m_labor_map.begin(),m_labor_map.end(),LaborOptimizer::compare_priority_then_rating());
    std::sort(m_labor_map.begin(),m_labor_map.end(),LaborOptimizer::compare_rating());

//    foreach(dwarf_labor_map dlm, m_labor_map){
//        LOGD << "Rating:" << dlm.rating << " Priority:" << dlm.det->priority;
//    }

    update_ratios();

    QHash<int, Dwarf*> haulers;
    foreach(Dwarf *d, m_dwarfs){
        haulers.insert(d->id(),d);
    }

    //optimize
    bool has_conficting_labor = false;    
    Labor *l;        
    //QHash<int, QList<Dwarf*> > conflicts;
    //QList<Dwarf*> conflicting_dwarves;

    QList<int> m_missing_roles;
    foreach(dwarf_labor_map dlm, m_labor_map){
        l = gdr->get_labor(dlm.det->labor_id);
        //check conflicting labors
        if (check_conflicts) {
            has_conficting_labor = false;
            foreach(int excluded, l->get_excluded_labors()) {
                if(dlm.d->labor_enabled(excluded)){
                    has_conficting_labor = true;
//                    conflicting_dwarves = conflicts.value(dlm.det->labor_id);
//                    conflicting_dwarves.append(dlm.d);
//                    conflicts.insert(dlm.det->labor_id, conflicting_dwarves);
                    break;
                }
            }
            if(dlm.det->use_skill && !m_missing_roles.contains(dlm.det->labor_id))
                m_missing_roles.append(dlm.det->labor_id);
        }
        //dwarf has available labor slots? target laborers reached?
        if(!has_conficting_labor && dlm.d->optimized_labors < plan->max_jobs_per_dwarf && dlm.det->assigned_laborers < dlm.det->max_count){

            LOGD << "Job:" << GameDataReader::ptr()->get_labor(dlm.det->labor_id)->name << " Role:" << dlm.det->role_name << " Dwarf:" << dlm.d->nice_name()
                 << " Rating:" << dlm.rating << " Raw Rating:" << dlm.d->get_role_rating(dlm.det->role_name,true);

            dlm.d->set_labor(dlm.det->labor_id, true, false);
            dlm.det->assigned_laborers++;
            dlm.d->optimized_labors++;              
        }

        if(plan->auto_haulers){
            if(dlm.d->optimized_labors >= roundf((float)plan->max_jobs_per_dwarf * (plan->hauler_percent/(float)100)))
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

    //emit optimize_message(QString::number(conflicting_count) + " conflicting labors could not be assigned.");
//    if(conflicts.count() > 0){
//        foreach(int id, conflicts.uniqueKeys()){
//            m_current_message.clear();
//            m_current_message.append(QPair<int,QString>(0,tr("%1 %2 labors not assigned due to conflicts.")
//                                                        .arg(QString::number(conflicts.value(id).count())).arg(gdr->get_labor(id)->name)));
//            foreach(Dwarf *d, conflicts.value(id)){
//                m_current_message.append(QPair<int,QString>(d->id(),d->nice_name()));
//            }
//            emit optimize_message(m_current_message);
//            //        emit optimize_message(tr("%1 %2 not assigned due to conflicting labors.")
//            //                              .arg(QString::number(conflicts.value(id))).arg(gdr->get_labor(id)->name));
//        }
//    }

    //get a list of hauling labors (for now this includes burial, cleaning, recovering wounded)
    QList<Labor*> skill_less_jobs = gdr->get_ordered_labors();
    for(int i = skill_less_jobs.count()-1; i >= 0; i--){
        if(!skill_less_jobs.at(i)->is_hauling)
            skill_less_jobs.removeAt(i);
    }

    if(plan->auto_haulers){
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
    m_total_jobs = m_target_population * plan->max_jobs_per_dwarf;
    m_raw_total_jobs = m_total_jobs;

    //get ratio sum
    int count = 0;
    foreach(PlanDetail *det, plan->plan_details){
        det->max_count = 0;
        det->group_ratio = 0;
        det->assigned_laborers = 0;
        if(det->priority > 0 && det->ratio > 0)
            m_ratio_sum += det->ratio;

        count++;
    }    

    labors_exceed_pop = false;
    if(check_conflicts){
        PlanDetail *temp;
        foreach(PlanDetail *det, plan->plan_details){
            if(det->priority > 0 && det->ratio > 0 && det->group_ratio <= 0){

                if(gdr->get_labor(det->labor_id)->get_excluded_labors().count() > 0){                                                            
                    //increase this labor's group ratio
                    det->group_ratio += det->ratio;
                    foreach(int id, gdr->get_labor(det->labor_id)->get_excluded_labors()){
                        temp =  plan->job_exists(gdr->get_labor(id)->labor_id);
                        if(temp)
                            det->group_ratio += temp->ratio;
                    }
                    //set all related labors to the ratio total we just calculated
                    foreach(int id, gdr->get_labor(det->labor_id)->get_excluded_labors()){
                        temp =  plan->job_exists(gdr->get_labor(id)->labor_id);
                        if(temp)
                            temp->group_ratio = det->group_ratio;
                    }
                    if(det->group_ratio / m_ratio_sum * m_total_jobs > m_total_population){
                        m_ratio_sum -= det->group_ratio;
                        labors_exceed_pop = true;
                    }
                }
            }
        }
        if(labors_exceed_pop)
            m_total_jobs -= m_total_population;
    }


    //if sum of job's coverage + conflicting job's coverage / total coverage > target population
    //job's max count = job's coverage / sum(job's coverage + conflicting coverages) * target population
    foreach(PlanDetail *det, plan->plan_details){
        if(det->priority > 0 && det->ratio > 0){
            //det->max_count = roundf(m_pop_count * (plan->pop_percent/(float)100) * (det->max_laborers/(float)100));
            if(det->group_ratio > 0 && labors_exceed_pop){
                det->max_count = roundf(det->ratio / det->group_ratio * m_total_population);
            }else{
                det->max_count = roundf(det->ratio / m_ratio_sum * m_total_jobs);
            }
            if(det->max_count > m_total_population)
                det->max_count = m_total_population;
            m_estimated_assigned_jobs += det->max_count;
        }
    }
}

void LaborOptimizer::update_population(QList<Dwarf*> m){
    m_dwarfs = m;
}


