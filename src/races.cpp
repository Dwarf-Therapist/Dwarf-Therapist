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
#include "races.h"

#include "dfinstance.h"
#include "caste.h"
#include "memorylayout.h"
#include "truncatingfilelogger.h"
#include "material.h"

Race::Race(DFInstance *df, VIRTADDR address, int id, QObject *parent)
    : QObject(parent)
    , m_address(address)
    , m_id(id)
    , m_name(QString::null)
    , m_description(QString::null)
    , m_name_plural(QString::null)
    , m_adjective(QString::null)
    , m_baby_name(QString::null)
    , m_baby_name_plural(QString::null)
    , m_child_name(QString::null)
    , m_child_name_plural(QString::null)
    , m_castes_vector(0)
    , m_df(df)
    , m_mem(df->memory_layout())
    , m_flags()
    , loaded_stats(false)
{
    load_data();
}

Race::~Race() {
    qDeleteAll(m_castes);
    m_castes.clear();

    qDeleteAll(m_creature_mats);
    m_creature_mats.clear();
}

Race* Race::get_race(DFInstance *df, const VIRTADDR & address, int id) {
    return new Race(df, address, id);
}

void Race::load_data() {
    if (!m_df || !m_df->memory_layout() || !m_df->memory_layout()->is_valid()) {
        LOGW << "load of Races called but we're not connected";
        return;
    }
    // make sure our reference is up to date to the active memory layout
    m_mem = m_df->memory_layout();
    TRACE << "Starting refresh of Race data at" << hexify(m_address);

    read_race();
}

void Race::read_race() {
    m_df->attach();
    //m_id = m_df->read_int(m_address);
    m_name = capitalize(m_df->read_string(m_address + m_mem->race_offset("name_singular")));
    TRACE << "RACE " << m_name << " at " << hexify(m_address);
    m_name_plural = capitalize(m_df->read_string(m_address + m_mem->race_offset("name_plural")));
    m_adjective = capitalize(m_df->read_string(m_address + m_mem->race_offset("adjective")));

    m_child_name = capitalize(m_df->read_string(m_address + m_mem->race_offset("child_name_singular")));
    m_child_name_plural = capitalize(m_df->read_string(m_address + m_mem->race_offset("child_name_plural")));

    m_baby_name = capitalize(m_df->read_string(m_address + m_mem->race_offset("baby_name_singular")));
    m_baby_name_plural = capitalize(m_df->read_string(m_address + m_mem->race_offset("baby_name_plural")));

    if(m_baby_name == "" && m_child_name != "")
        m_baby_name = m_child_name;
    if(m_baby_name_plural == "" && m_child_name_plural != "")
        m_baby_name_plural = m_child_name_plural;

    if(m_child_name == "" && m_baby_name != "")
        m_child_name = m_baby_name;
    if(m_child_name_plural == "" && m_baby_name_plural  != "")
        m_child_name_plural = m_baby_name_plural;

    if(m_baby_name == "")
        m_baby_name = m_name + tr(" Baby");
    if(m_baby_name_plural == "")
        m_baby_name_plural = m_name_plural + tr(" Babies");

    if(m_child_name == "")
        m_child_name = m_name + tr(" Offspring");
    if(m_child_name_plural == "")
        m_child_name_plural = m_name + tr(" Offsprings");

    m_child_name = capitalizeEach(m_child_name);
    m_child_name_plural = capitalizeEach(m_child_name_plural);
    m_baby_name = capitalizeEach(m_baby_name);
    m_baby_name_plural = capitalizeEach(m_baby_name_plural);

    m_pref_string_vector = m_address + m_mem->race_offset("pref_string_vector");
    m_pop_ratio_vector = m_address + m_mem->race_offset("pop_ratio_vector");
    m_castes_vector = m_address + m_mem->race_offset("castes_vector");
    m_materials_addr = m_df->enumerate_vector(m_address + m_mem->race_offset("materials_vector"));
    m_tissues_addr = m_df->enumerate_vector(m_address + m_mem->race_offset("tissues_vector"));

    //m_description = m_df->read_string(m_address + m_mem->caste_offset("caste_descr"));
    QVector<VIRTADDR> castes = m_df->enumerate_vector(m_castes_vector);
    //LOGD << "RACE " << m_name << " (index:" << m_id << ") with " << castes.size() << "castes";

    if (!castes.empty()) {
        foreach (VIRTADDR caste_addr, castes) {
            m_castes.append(new Caste(m_df, caste_addr, this));
        }
    }

    //if this is the race that we're currently playing as, we need to load some extra data and set some flags
    if(m_id == m_df->dwarf_race_id()){
        load_caste_ratios();
    }

    m_flags = FlagArray(m_df, m_address + m_mem->race_offset("flags"));
    m_df->detach();
}

void Race::load_caste_ratios(){
    if(!loaded_stats){
        QVector<int> ratios = m_df->enum_vec<int>(m_pop_ratio_vector);

        if(ratios.count() > 0){
            int sum = 0;
            int valid_castes = 0;
            for(int i=0; i < ratios.count(); i++){
                sum += ratios.at(i);
            }
            if(sum<=0)
                sum = 1;

            float commonality = 0.0;
            for(int idx=0; idx < m_castes.count();idx++){
                commonality = (float)ratios.at(idx) / (float)sum;
                if(commonality > 0.0001){
                    valid_castes++;
                }
            }
            //castes usually come in male/female pairs, vanilla only has 2 castes (dwarf male/female)
            //if we have more castes then that, assume it's a mod with castes, with different skill rates, attribute and trait bins
            //this could be split into a more granular check for different skill rates and different attributes
            if(valid_castes > 2)
                DT->multiple_castes(true);
        }
        loaded_stats = true;
    }
}

void Race::load_materials(int idx){
    if(m_materials_addr.isEmpty())
        return;
    //load creature's material list
    if(idx >= 0 && idx < m_materials_addr.size()){
        Material *m = Material::get_material(m_df, m_materials_addr.at(idx) ,idx, false, this);
        m_creature_mats.insert(idx,m);
    }else{
        for(int idx = 0; idx < m_materials_addr.size(); idx++){
            load_materials(idx);
        }
    }
}

QHash<int,Material*> Race::get_creature_materials(){
    if(m_creature_mats.empty()){
        load_materials();
    }
    return m_creature_mats;
}

Material * Race::get_creature_material(int index){
    if(!m_creature_mats.contains(index)){
        load_materials(index);
    }
    if(m_creature_mats.contains(index)){
        return m_creature_mats.value(index);
    }else{
        return new Material(this);
    }
}

bool Race::caste_flag(CASTE_FLAGS cf) const {
    if(m_castes.empty()){
        return false;
    }else{
        if(auto c = m_castes.value(0))
            return c->flags().has_flag(cf);
        else
            return false;
    }
}

int Race::adult_size(){
    if(m_castes.empty()){
        return -1;
    }else{
        Caste *c = m_castes.at(0);
        if(c){
            return c->adult_size();
        }else{
            return -1;
        }
    }
}

VIRTADDR Race::get_tissue_address(int index){
    if(index > -1 && index < m_tissues_addr.size())
        return m_tissues_addr.at(index);
    else
        return -1;
}
