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
#include "unithealth.h"
#include "dfinstance.h"
#include "unitwound.h"
#include "healthcategory.h"
#include "dwarf.h"
#include "caste.h"

QHash<int, HealthCategory*> UnitHealth::m_health_descriptions;
QList<QPair<int,QString> > UnitHealth::m_ordered_category_names;

UnitHealth::UnitHealth()
    :m_df(0x0)
    ,m_dwarf_addr(0x0)
    ,m_dwarf(0x0)
    ,m_fresh_wounds(0)
    ,m_req_diagnosis(false)
{
}
UnitHealth::UnitHealth(DFInstance *df, Dwarf *d, bool req_diagnosis)
    :m_df(df)
    ,m_dwarf_addr(d->address())
    ,m_dwarf(d)
    ,m_fresh_wounds(0)
    ,m_req_diagnosis(req_diagnosis)
{    
    read_health_info();
    read_wounds();

    //sort everything by serverity
    foreach(eHealth::H_INFO key, m_status_info.uniqueKeys()){
        QList<HealthInfo*> list = m_status_info.take(key);
        std::sort(list.begin(),list.end(),HealthInfo::less_than_severity());
        m_status_info.insert(key,list);
    }
    foreach(eHealth::H_INFO key, m_treatment_info.uniqueKeys()){
        QList<HealthInfo*> list = m_treatment_info.take(key);
        std::sort(list.begin(),list.end(),HealthInfo::less_than_severity());
        m_treatment_info.insert(key,list);
    }
    foreach(eHealth::H_INFO key, m_wounds_info.uniqueKeys()){
        QList<HealthInfo*> list = m_wounds_info.take(key);
        std::sort(list.begin(),list.end(),HealthInfo::less_than_severity());
        m_wounds_info.insert(key,list);
    }
}

UnitHealth::~UnitHealth(){
    m_wound_details.clear();

    m_wounds_info.clear();
    m_treatment_info.clear();
    m_status_info.clear();

    m_body_parts.clear();

    m_wounds.clear();

    m_df = 0;
    m_dwarf = 0;
}

void UnitHealth::add_info(eHealth::H_INFO id, bool idx0, bool idx1, bool idx2, bool idx3){
    if(!idx0 && !idx1 && !idx2 && !idx3)
        return;

    QList<short> desc_index;
    bool multiple = m_health_descriptions.value(id)->allows_multiple();
    if(multiple){
        if(idx0)
            desc_index.append(0);
        if(idx1)
            desc_index.append(1);
        if(idx2)
            desc_index.append(2);
        if(idx2)
            desc_index.append(3);
    }else{
        if(idx0)
            desc_index.append(0);
        else if(idx1)
            desc_index.append(1);
        else if(idx2)
            desc_index.append(2);
        else if(idx3)
            desc_index.append(3);
    }
    add_info(id,desc_index);
}

void UnitHealth::add_info(eHealth::H_INFO h, QList<short> indexes, bool wound_visible){
    //figure out which list we should be adding the info to
    QList<HealthInfo*> info_list;
    if(m_health_descriptions.value(h)->diff_subitem_types()){
        foreach(short idx, indexes){
            HealthInfo *hi = m_health_descriptions.value(h)->description(idx);
            if(hi){
                if(hi->is_status()){
                    info_list = m_status_info.take(h);
                    add_info(hi,info_list);
                    if(info_list.size() > 0)
                        m_status_info.insert(h,info_list);
                }
                if(hi->is_treatment()){
                    info_list = m_treatment_info.take(h);
                    add_info(hi,info_list);
                    if(info_list.size() > 0)
                        m_treatment_info.insert(h,info_list);
                }
                if(wound_visible && hi->is_wound()){
                    info_list = m_wounds_info.take(h);
                    add_info(hi,info_list);
                    if(info_list.size() > 0)
                        m_wounds_info.insert(h,info_list);
                }
            }
        }
    }else{
        if(m_health_descriptions.value(h)->is_status()){
            info_list = m_status_info.take(h);
            add_info(h,indexes,info_list);
            if(info_list.size() > 0)
                m_status_info.insert(h,info_list);
        }
        if(m_health_descriptions.value(h)->is_treatment()){
            info_list = m_treatment_info.take(h);
            add_info(h,indexes,info_list);
            if(info_list.size() > 0)
                m_treatment_info.insert(h,info_list);
        }
        if(wound_visible && m_health_descriptions.value(h)->is_wound()){
            info_list = m_wounds_info.take(h);
            add_info(h,indexes,info_list);
            if(info_list.size() > 0)
                m_wounds_info.insert(h,info_list);
        }
    }
}

void UnitHealth::add_info(eHealth::H_INFO h, QList<short> indexes, QList<HealthInfo *> &info_list){
    HealthInfo* hi;

    foreach(short idx, indexes){
        hi = get_health_info(h,idx);
        add_info(hi, info_list);
    }
}

void UnitHealth::add_info(HealthInfo *hi, QList<HealthInfo *> &info_list){
    bool replaced = false;
    bool ignore = false;
    for(int list_index = 0; list_index < info_list.count(); list_index++){
        if(!m_health_descriptions.value(hi->h_category())->allows_multiple() && hi->h_category() == info_list.at(list_index)->h_category()){
            if(info_list.at(list_index)->severity() > hi->severity()){
                info_list.replace(list_index,hi);
                replaced = true;
                break;
            }else{
                ignore = true;
            }
        }
    }
    if(!replaced && !info_list.contains(hi) && !ignore){
        info_list.append(hi);
    }
}

void UnitHealth::read_health_info(){
    MemoryLayout *mem = m_df->memory_layout();
    VIRTADDR unit_health_addr = m_df->read_addr(m_dwarf_addr + mem->dwarf_offset("unit_health_info"));
    //health flags contain the requests for treatment info
    quint32 health_flags = m_df->read_int(unit_health_addr + 0x4);
    //read bp flags for inoperable rot
    health_req_flags = m_df->enumerate_vector(unit_health_addr + 0x8 - DFInstance::VECTOR_POINTER_OFFSET);
    //1 << 2 << 4 << 8 << 16 << 32 << 64 << 128 << 256 << 512 << 1024 match with..
    //diagnosis, recovery, unk, immobilization, dressing, cleaning, surgery, suture, setting, traction, crutch
    //these only have a single description associated with them. we also want to avoid the 4th (unknown) request

    int counter = 0;
    short sh_counter = 0;
    QList<short> vals;

    bool needs_diagnosis = (has_flag(1,health_flags));
    add_info(eHealth::HI_DIAGNOSIS,false,needs_diagnosis);
    add_info(eHealth::HI_IMMOBILIZATION,(has_flag(8,health_flags)));
    add_info(eHealth::HI_DRESSING,(has_flag(16,health_flags)));
    add_info(eHealth::HI_CLEANING,(has_flag(32,health_flags)));
    add_info(eHealth::HI_SURGERY,(has_flag(64,health_flags)));
    add_info(eHealth::HI_SUTURES,(has_flag(128,health_flags)));
    add_info(eHealth::HI_SETTING,(has_flag(256,health_flags)));
    add_info(eHealth::HI_TRACTION,(has_flag(512,health_flags)));

    //stunned, webbed, dizziness
    bool unconscious = false;
//    bool stunned = false;

    VIRTADDR base_counter_addr = mem->dwarf_offset("counters1"); //starts at winded
    VIRTADDR base_counter2_addr = mem->dwarf_offset("counters2"); //starts at pain
    VIRTADDR base_counter3_addr = mem->dwarf_offset("counters3"); //starts at paralysis

    VIRTADDR base_limbs_addr = mem->dwarf_offset("limb_counters");

    //the unconscious state seems to depend on whether or not the dwarf is sleeping
    //normally the unconscious counter doesn't seem to exceed 2 for a sleeping dwarf, but this is safer
    bool sleeping = false;
    if(!m_dwarf->get_caste()->flags().has_flag(NO_SLEEP))
        sleeping = (m_dwarf->current_job_id() == 21); //direct reference to ini job id, probably a bad idea

    vals.clear();
    sh_counter = m_df->read_short(m_dwarf_addr + base_counter_addr+0x4); //unconscious
    if(!sleeping && sh_counter > 0){
        vals.append(0);
        unconscious = true;
    }
    if(!m_dwarf->get_caste()->flags().has_flag(NO_STUN)){
        sh_counter = m_df->read_short(m_dwarf_addr + base_counter_addr+0x2); //stunned
        if(sh_counter > 0){
            vals.append(1);
            //        stunned = true;
        }
    }
    if(!m_dwarf->get_caste()->flags().has_flag(WEB_IMMUNE)){
        sh_counter = m_df->read_short(m_dwarf_addr + base_counter_addr+0x8); //webbed
        if(sh_counter > 0)
            vals.append(2);
    }
    if(!m_dwarf->get_caste()->flags().has_flag(NO_DIZZINESS)){
        counter = m_df->read_short(m_dwarf_addr + base_counter2_addr + 0x8); //dizzy
        if(counter > 0)
            vals.append(3);
    }
    if(vals.size() > 0)
        add_info(eHealth::HI_MOVEMENT,vals);




    //breathing problems/drowning/winded
    if(!m_dwarf->get_caste()->flags().has_flag(NO_BREATHE)){
        //    bool drowning = false;
        vals.clear();
        sh_counter = m_df->read_short(m_dwarf_addr + base_counter_addr);
        if(sh_counter > 0)
            vals.push_front(3); //winded

        //it appears that amphibious creatures don't need breathing parts, and so can't have problems breathing?
        if(!m_dwarf->get_caste()->flags().has_flag(AMPHIBIOUS)){
            if(has_flag(0x00000020,m_dwarf->get_flag1())){
                vals.push_front(1); //drowning
                //        drowning = true;
            }
            if(!has_flag(0x10000000,m_dwarf->get_flag2()))
                vals.push_front(0); //missing breathing part (can't breathe)
            else if(has_flag(0x20000000,m_dwarf->get_flag2()))
                vals.push_front(2); //trouble breathing
        }

        if(vals.size() > 0)
            add_info(eHealth::HI_BREATHING, vals);
    }

    //seems can't stand is set when drowning, unconscious?, or laying on the ground (0x8000 flag check)
    if(!m_dwarf->get_caste()->flags().has_flag(IMMOBILE_LAND)){
        short limb_stand_max = m_df->read_short(m_dwarf_addr + base_limbs_addr);
        short limb_stand_count = m_df->read_short(m_dwarf_addr + base_limbs_addr + 0x2);

        if(limb_stand_max > 0){
            bool fallen_down = (has_flag(0x8000,m_dwarf->get_flag1()) && !sleeping);

            //having a crutch doesn't seem to count as a limb to stand on??
            bool has_crutch = has_flag(0x00000040,m_dwarf->get_flag3());
            if(has_crutch)
                limb_stand_count++;
            add_info(eHealth::HI_CRUTCH,(has_flag(1024,health_flags)),has_crutch);

            vals.clear();
            //seems if a dwarf only has a single good limb to stand on (including crutches) but has fallen, then they cannot stand
            if(limb_stand_count <= 0 || (limb_stand_count < limb_stand_max && fallen_down) || unconscious){ //if(unconscious || stunned || drowning || limb_stand_count <= 0)
                vals.append(0);
                if(needs_diagnosis && m_dwarf->current_job_id() != 52) //if already resting, then they've been recovered
                    add_info(eHealth::HI_DIAGNOSIS,(has_flag(4,health_flags))); //not diagnosed yet, but can't walk, and isn't resting yet. needs recovery
            }else if(limb_stand_count < limb_stand_max){
                if(!has_crutch)
                    vals.append(1); //stand impaired
                //        else
                //            vals.append(2); //can stand with crutch?
            }
            if(vals.size() > 0)
                add_info(eHealth::HI_STAND,vals);
        }
    }


    //check the grasp status
    vals.clear();
    short limb_grasp_max = m_df->read_short(m_dwarf_addr + base_limbs_addr + 0x4);
    short limb_grasp_count = m_df->read_short(m_dwarf_addr + base_limbs_addr + 0x6);
    if(limb_grasp_max > 0){
        if(limb_grasp_count <= 0)
            vals.append(0);
        else if(limb_grasp_count < limb_grasp_max)
            vals.append(1);
        if(vals.size() > 0)
            add_info(eHealth::HI_GRASP,vals);
    }

    //check the flight status
    if(m_dwarf->get_caste()->flags().has_flag(FLIER)){
        vals.clear();
        short limb_fly_max = m_df->read_short(m_dwarf_addr +  base_limbs_addr + 0x8);
        short limb_fly_count = m_df->read_short(m_dwarf_addr +  base_limbs_addr + 0xa);
        if(limb_fly_max > 0){
            if(limb_fly_count <= 0)
                vals.append(0);
            else if(limb_fly_count < limb_fly_max)
                vals.append(1);
            if(vals.size() > 0)
                add_info(eHealth::HI_FLY,vals);
        }
    }

    //check blood loss
    int blood_max = m_df->read_short(m_dwarf_addr + mem->dwarf_offset("blood"));
    int blood_curr = m_df->read_short(m_dwarf_addr + mem->dwarf_offset("blood")+0x4);
    float blood_perc = (float)blood_curr / (float)blood_max;
    if(blood_perc > 0)
        add_info(eHealth::HI_BLOOD_LOSS, (blood_perc < 0.25),(blood_perc < 0.50));

    //check hunger    
    if(!m_dwarf->get_caste()->flags().has_flag(NO_EAT)){
        counter = m_df->read_int(m_dwarf_addr + base_counter3_addr + 0x10);
        add_info(eHealth::HI_HUNGER, (counter >= 75000),(counter >= 50000));
    }

    //check thirst
    if(!m_dwarf->get_caste()->flags().has_flag(NO_DRINK)){
        counter = m_df->read_int(m_dwarf_addr + base_counter3_addr + 0x14);
        add_info(eHealth::HI_THIRST, (counter >= 50000),(counter >= 25000));
    }

    //check drowsiness
    if(!m_dwarf->get_caste()->flags().has_flag(NO_EXERT)){
        counter = m_df->read_int(m_dwarf_addr + base_counter3_addr + 0x18);
        add_info(eHealth::HI_SLEEPLESS,(counter >= 150000), (counter >= 57600));
    }

    //check exhaustion
    if(!m_dwarf->get_caste()->flags().has_flag(NO_EXERT)){
        counter = m_df->read_int(m_dwarf_addr + base_counter3_addr + 0xc);
        add_info(eHealth::HI_TIREDNESS, (counter >= 6000),(counter >= 4000),(counter >= 2000));
    }

    //check paralysis
    if(!m_dwarf->get_caste()->flags().has_flag(PARALYZE_IMMUNE)){
        counter = m_df->read_int(m_dwarf_addr + base_counter3_addr);
        add_info(eHealth::HI_PARALYSIS, (counter >= 100), (counter >= 50), (counter >= 1));
    }

    //check numbness
    counter = m_df->read_int(m_dwarf_addr + base_counter3_addr + 0x4);
    add_info(eHealth::HI_NUMBNESS, (counter >= 100), (counter >= 50), (counter >= 1));

    //check fever
    if(!m_dwarf->get_caste()->flags().has_flag(NO_FEVERS)){
        counter = m_df->read_int(m_dwarf_addr + base_counter3_addr + 0x8);
        add_info(eHealth::HI_FEVER, (counter >= 100), (counter >= 50), (counter >= 1));
    }

    //check nausea
    if(!m_dwarf->get_caste()->flags().has_flag(NO_NAUSEA)){
        counter = m_df->read_int(m_dwarf_addr + base_counter2_addr + 0x4);
        add_info(eHealth::HI_NAUSEOUS, (counter > 0));
    }

    //check pain
    if(!m_dwarf->get_caste()->flags().has_flag(NO_PAIN)){
        counter = m_df->read_int(m_dwarf_addr + base_counter2_addr);
        add_info(eHealth::HI_PAIN, (counter > 50), (counter > 25), (counter > 0));
    }

    //vision
    if(!m_dwarf->get_caste()->flags().has_flag(EXTRAVISION)){
        add_info(eHealth::HI_VISION,(!has_flag(0x02000000,m_dwarf->get_flag2())),
                 (has_flag(0x04000000,m_dwarf->get_flag2())), (has_flag(0x08000000,m_dwarf->get_flag2())));
    }

    //gutted
    add_info(eHealth::HI_GUTTED, (has_flag(0x00004000, m_dwarf->get_flag2())));
}

void UnitHealth::read_wounds(){
    VIRTADDR addr = m_df->memory_layout()->dwarf_offset("body_component_info");
    body_part_status_flags = m_df->enumerate_vector(m_dwarf_addr +  addr - DFInstance::VECTOR_POINTER_OFFSET);
    layer_status_flags = m_df->enumerate_vector(m_dwarf_addr + addr + m_df->memory_layout()->dwarf_offset("layer_status_vector"));

    //add the wounds based on the wounded parts
    QVector<VIRTADDR> wounds = m_df->enumerate_vector(m_dwarf_addr + m_df->memory_layout()->dwarf_offset("wounds_vector"));
    foreach(VIRTADDR addr, wounds){
        m_wounds.append(UnitWound(m_df,addr,m_dwarf,this));
    }

    //check body parts for old wounds (specifically missing parts), these wounds haven't been made during the course
    //of this fortress, so they won't show up under the wounds section we just examined
    int idx = 0;
    foreach(int bps, body_part_status_flags){
        //filter this down to checking exact bits, since we're currently only check for motor/sensory nerve or missing part
        if(bps > 0 && (has_flag(0x2,bps) || has_flag(0x200,bps) || has_flag(0x400,bps))){
            UnitWound uw = UnitWound(m_df,idx,this);
            if(uw.get_wounded_parts().size() > 0){
                m_wounds.append(uw);
            }
        }
        idx++;
    }

    build_wounds_summary();

    //other grouping ideas:

    //group wounds by health category, to allow us to show only those wounds which are related to the column type
    //for example, if the column is for infection, then only show those bodyparts/wounds that are infected
    //the problem with this grouping is that there's no way to see ALL the wounds together

    //group the wounds by main body parts (right lower leg, head, upper body, etc..) this is primarily for showing wounds in body part columns
    //it seems the only way to accomplish this will be to read the tokens (LLL = left lower leg) and match them to appropriate pre-defined groups that we'd use as columns
    //for example, RLL and RUL would both be assigned to right leg. however the issue then is if our playable race has multiple right legs.. we'll have to use some kind of pattern matching
    //to map things appropriately RA* would match all instances of right arms
}

void UnitHealth::build_wounds_summary(){
    foreach(UnitWound w, m_wounds){
        //wound details by body part
        QHash<QString,QList<HealthInfo*> > wounds_info = w.get_wound_details();
        foreach(QString bp_name, wounds_info.uniqueKeys()){
            QList<HealthInfo*> wnd_details = wounds_info.take(bp_name);

            //keep a list of body parts and their related health info stuff
            QList<HealthInfo*> info_summary = m_wound_details.take(bp_name);
            foreach(HealthInfo* hi, wnd_details){
                add_info(hi,info_summary);
            }
            if(info_summary.size() > 0){
                m_wound_details.insert(bp_name,info_summary);
                if(w.is_recent())
                    m_fresh_wounds++;
            }
        }
    }
}

BodyPartDamage UnitHealth::get_body_part(int body_part_id){
    if(!m_body_parts.keys().contains(body_part_id)){
        quint32 bp_status = 0;
        quint32 bp_req = 0;
        if(body_part_id < body_part_status_flags.size())
            bp_status = body_part_status_flags.at(body_part_id);
        if(body_part_id < health_req_flags.size())
            bp_req = health_req_flags.at(body_part_id);

        BodyPart *bp = m_dwarf->get_caste()->get_body_part(body_part_id);
        BodyPartDamage bpd = BodyPartDamage(bp,bp_status,bp_req);

        m_body_parts.insert(body_part_id,bpd);
    }
    return m_body_parts.value(body_part_id);
}


HealthInfo *UnitHealth::get_most_severe(eHealth::H_INFO h){
    if(m_treatment_info.contains(h)){
        return m_treatment_info.value(h).at(0);
    }
    if(m_status_info.contains(h)){
        return m_status_info.value(h).at(0);
    }
    if(m_wounds_info.contains(h)){
        return m_wounds_info.value(h).at(0);
    }
    return 0x0;
}

bool UnitHealth::has_info_detail(eHealth::H_INFO h, int idx){
    if(m_treatment_info.contains(h)){
        if(m_treatment_info.value(h).contains(get_health_info(h,idx)))
            return true;
    }
    if(m_status_info.contains(h)){
        if(m_status_info.value(h).contains(get_health_info(h,idx)))
            return true;
    }
    if(m_wounds_info.contains(h)){
        if(m_wounds_info.value(h).contains(get_health_info(h,idx)))
            return true;
    }
    return false;
}

QStringList UnitHealth::get_all_category_desc(eHealth::H_INFO hs, bool symbol_only, bool colored){
    return m_health_descriptions.value(hs)->get_all_descriptions(symbol_only,colored);
}

QMap<QString, QStringList> UnitHealth::get_wound_summary(bool colored, bool symbols){
    int key = ((int)colored << 1) | (int)symbols;
    if(m_wound_summary.value(key).isEmpty()){
        QMap<QString, QStringList> summary = m_wound_summary.value(key);
        foreach(QString bp, m_wound_details.uniqueKeys()){
            QList<HealthInfo*> infos = m_wound_details.value(bp);
            QStringList bp_summary = summary.value(bp);
            foreach(HealthInfo* hi, infos){
                QString desc = hi->formatted_value(colored,symbols);
                if(!bp_summary.contains(desc))
                    bp_summary.append(desc);
            }
            if(bp_summary.size() > 0){
                qSort(bp_summary);
                summary.insert(bp,bp_summary);
            }
        }        
        m_wound_summary.insert(key, summary);
    }
    return m_wound_summary.value(key);
}


QStringList UnitHealth::get_treatment_summary(bool colored, bool symbols){
    int key = ((int)colored << 1) | (int)symbols;
    if(m_treatment_summary.value(key).isEmpty()){
        QStringList summary = m_treatment_summary.value(key);
        foreach(eHealth::H_INFO h, m_treatment_info.uniqueKeys()){
            foreach(HealthInfo* hi, m_treatment_info.value(h)){
                summary.append(hi->formatted_value(colored,symbols));
            }
        }
        qSort(summary);
        m_treatment_summary.insert(key,summary);
    }
    return m_treatment_summary.value(key);
}

QStringList UnitHealth::get_status_summary(bool colored, bool symbols){
    int key = ((int)colored << 1) | (int)symbols;
    if(m_status_summary.value(key).isEmpty()){
        QStringList summary = m_status_summary.value(key);
        foreach(eHealth::H_INFO h, m_status_info.uniqueKeys()){
            if(m_status_info.value(h).count() > 0){
                if(m_health_descriptions.value(h)->allows_multiple()){ //m_display_descriptions.value(h).at(0)->can_have_multiple()){
                    foreach(HealthInfo *hi,m_status_info.value(h)){
                        summary.append(hi->formatted_value(colored,symbols));
                    }
                }else{
                    summary.append(m_status_info.value(h).at(0)->formatted_value(colored,symbols));
                }
            }
        }
        qSort(summary);
        m_status_summary.insert(key,summary);
    }
    return m_status_summary.value(key);
}

HealthInfo* UnitHealth::get_health_info(eHealth::H_INFO hs, short idx){
    return m_health_descriptions.value((int)hs)->description(idx);
}

void UnitHealth::load_health_descriptors(QSettings &s){
    if(m_health_descriptions.count() <= 0){

        int categories = s.beginReadArray("health_info");
        QStringList cat_names;
        for(int i = 0; i < categories; ++i) {
            s.setArrayIndex(i);
            HealthCategory *hc = new HealthCategory(s);
            m_health_descriptions.insert(hc->id(),hc);
            cat_names.append(hc->name());
        }
        s.endArray();

        qSort(cat_names);
        foreach(QString name, cat_names) {
            foreach(int id, m_health_descriptions.uniqueKeys()) {
                if (m_health_descriptions.value(id)->name() == name) {
                    m_ordered_category_names << QPair<int, QString>(id, name);
                    break;
                }
            }
        }
    }
}

void UnitHealth::cleanup(){
    qDeleteAll(m_health_descriptions);
    m_health_descriptions.clear();
}
