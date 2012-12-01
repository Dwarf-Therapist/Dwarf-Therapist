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
#include "dwarftherapist.h"

LaborOptimizer::LaborOptimizer(laborOptimizerPlan *plan, QObject *parent)    
    :QObject(parent)
    , plan(plan)        
{    
    check_conflicts = DT->user_settings()->value("options/labor_exclusions",true).toBool();
    gdr = GameDataReader::ptr();
}

void LaborOptimizer::calc_population(bool load_labor_map){
    //clear the labor map
    m_labor_map.clear();
    m_total_population = 0;

    //setup our new map
    bool labor_cheats = DT->user_settings()->value("options/allow_labor_cheats",true).toBool();

    m_current_message.clear();
    for(int i = m_dwarfs.count()-1; i >= 0; i--){
        Dwarf *d = m_dwarfs.at(i);

        //exclude nobles, hospitalized dwarfs, children, babies and militia
        if(d->noble_position() != "" && plan->exclude_nobles){
            m_current_message.append(QPair<int, QString> (d->id(), tr("Excluding %1 because they're a noble.").arg(d->nice_name())));
            m_dwarfs.removeAt(i);
        }
        else if(d->current_job_id() == 52 && plan->exclude_injured){
            m_current_message.append(QPair<int, QString> (d->id(), tr("Excluding %1 because they're hospitalized.").arg(d->nice_name())));
            if(load_labor_map)
                m_dwarfs.at(i)->clear_labors();
            m_dwarfs.removeAt(i);
        }
        else if(d->profession()=="Baby"){
            m_dwarfs.removeAt(i);
        }
        else if(d->active_military() && plan->exclude_military){
            m_current_message.append(QPair<int, QString> (d->id(), tr("Excluding %1 because they're on active duty.").arg(d->nice_name())));
            if(load_labor_map)
                m_dwarfs.at(i)->clear_labors();
            m_dwarfs.removeAt(i);
        }
        else if(d->profession()=="Child" && !labor_cheats){
            m_current_message.append(QPair<int, QString> (d->id(), tr("Excluding %1 because they're a child.").arg(d->nice_name())));
            m_dwarfs.removeAt(i);
        }
        else{
            m_total_population++;
            if(load_labor_map){
                //clear dwarf's existing labors
                d->clear_labors();
                d->optimized_labors = 0;
                foreach(laborOptimizerPlan::detail *det, plan->plan_details){
                    //skip labor with <= 0 priority/max workers
                    if(det->priority > 0 && det->ratio > 0){
                        dwarf_labor_map dlm;
                        dlm.d = d;
                        dlm.det = det;
                        if(!det->role_name.isEmpty()){
                            dlm.rating = det->priority * d->get_role_rating(det->role_name, true);
                        }
                        else{
                            dlm.rating = det->priority * (d->get_skill(GameDataReader::ptr()->get_labor(dlm.det->labor_id)->skill_id).actual_exp() / (float)29000 * 100);
                            //dlm.rating = det->priority * (d->get_skill(GameDataReader::ptr()->get_labor(dlm.det->labor_id)->skill_id).rating() / 20.0 * 100);
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

void LaborOptimizer::optimize_labors(){
    if(m_dwarfs.count() > 0)
        optimize_labors(m_dwarfs);
}

void LaborOptimizer::optimize_labors(QList<Dwarf*> dwarfs){
      m_dwarfs = dwarfs;

    //create the labor mapping
    calc_population(true);

    //sort list by the weighted rating
    std::sort(m_labor_map.begin(),m_labor_map.end(),LaborOptimizer::less_than_key());

    update_ratios();

    QHash<int, Dwarf*> haulers;
    foreach(Dwarf *d, dwarfs){
        haulers.insert(d->id(),d);
    }

    //optimize
    bool has_conficting_labor = false;    
    Labor *l;        
    //QHash<int, QList<Dwarf*> > conflicts;
    //QList<Dwarf*> conflicting_dwarves;
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
        }

        //dwarf has available labor slots? target laborers reached?
        if(!has_conficting_labor && dlm.d->optimized_labors < plan->max_jobs_per_dwarf && dlm.det->assigned_laborers < dlm.det->max_count){
            dlm.d->set_labor(dlm.det->labor_id, true, false);
            dlm.det->assigned_laborers++;
            dlm.d->optimized_labors++;
        }

        if(plan->auto_haulers){
            if(dlm.d->optimized_labors >= roundf((float)plan->max_jobs_per_dwarf * (plan->hauler_percent/(float)100)))
                haulers.remove(dlm.d->id());
        }
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
    m_total_jobs = (roundf(m_target_population * plan->max_jobs_per_dwarf));
    m_raw_total_jobs = m_total_population * plan->max_jobs_per_dwarf;

    //get ratio sum
    foreach(laborOptimizerPlan::detail *det, plan->plan_details){
        det->max_count = 0;
        det->group_ratio = 0;
        det->assigned_laborers = 0;
        if(det->priority > 0 && det->ratio > 0)
            m_ratio_sum += det->ratio;
    }

    labors_exceed_pop = false;
    if(check_conflicts){

        foreach(laborOptimizerPlan::detail *det, plan->plan_details){
            if(det->priority > 0 && det->ratio > 0 && det->group_ratio <= 0){

                if(gdr->get_labor(det->labor_id)->get_excluded_labors().count() > 0){
                    //increase this labor's group ratio
                    det->group_ratio += det->ratio;
                    foreach(int id, gdr->get_labor(det->labor_id)->get_excluded_labors()){
                        det->group_ratio += plan->job_exists(gdr->get_labor(id)->labor_id)->ratio;
                    }
                    //set all related labors to the ratio total we just calculated
                    foreach(int id, gdr->get_labor(det->labor_id)->get_excluded_labors()){
                        plan->job_exists(gdr->get_labor(id)->labor_id)->group_ratio = det->group_ratio;
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
    foreach(laborOptimizerPlan::detail *det, plan->plan_details){
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

    if(labors_exceed_pop)
        m_estimated_assigned_jobs -= m_total_population;

}

void LaborOptimizer::update_population(QList<Dwarf*> m){
    m_dwarfs = m;
}
