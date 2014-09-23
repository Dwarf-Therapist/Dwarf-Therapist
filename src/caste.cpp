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
#include "caste.h"
#include "attribute.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "truncatingfilelogger.h"
#include "gamedatareader.h"
#include "flagarray.h"
#include "races.h"
#include "bodypart.h"
#include "trait.h"

#include "dwarfstats.h"

Caste::Caste(DFInstance *df, VIRTADDR address, Race *r, QObject *parent)
    : QObject(parent)
    , m_address(address)
    , m_race(r)
    , m_tag(QString::null)
    , m_name(QString::null)
    , m_name_plural(QString::null)
    , m_description(QString::null)
    , m_baby_age(0)
    , m_child_age(0)
    , m_df(df)
    , m_mem(df->memory_layout())
    , m_flags()
    , m_body_addr(0x0)
{
    load_data();
}

Caste::~Caste() {
    qDeleteAll(m_body_parts);
    m_attrib_ranges.clear();
    m_skill_rates.clear();
    m_race = 0;
}

Caste* Caste::get_caste(DFInstance *df, const VIRTADDR & address, Race *r) {
    return new Caste(df, address, r);
}

void Caste::load_data() {
    if (!m_df || !m_df->memory_layout() || !m_df->memory_layout()->is_valid()) {
        LOGW << "load of Castes called but we're not connected";
        return;
    }
    // make sure our reference is up to date to the active memory layout
    m_mem = m_df->memory_layout();
    TRACE << "Starting refresh of Caste data at" << hexify(m_address);

    read_caste();
}

void Caste::read_caste() {
    m_tag = m_df->read_string(m_address);
    m_name = capitalizeEach(m_df->read_string(m_address + m_mem->caste_offset("caste_name")));
    m_name_plural = capitalizeEach(m_df->read_string(m_address + m_mem->word_offset("noun_plural")));
    m_description = m_df->read_string(m_address + m_mem->caste_offset("caste_descr"));

    m_flags = FlagArray(m_df, m_address + m_mem->caste_offset("flags"));

    if(m_flags.has_flag(BABY))
        m_baby_age = m_df->read_int(m_address + m_mem->caste_offset("baby_age"));
    if(m_flags.has_flag(CHILD))
        m_child_age = m_df->read_int(m_address + m_mem->caste_offset("child_age"));

    if(m_child_age < 0)
        m_child_age = 0;
    if(m_baby_age < 0)
        m_baby_age = 0;

    if(!m_flags.has_flag(NOT_BUTCHERABLE)){
        m_flags.set_flag(BUTCHERABLE,true);
    }

    m_body_addr = m_address + m_mem->caste_offset("body_info");
    m_body_parts_addr = m_df->enumerate_vector(m_body_addr - DFInstance::VECTOR_POINTER_OFFSET);

    if(m_flags.has_flag(TRAINABLE_HUNTING) || m_flags.has_flag(TRAINABLE_WAR) ||
            m_flags.has_flag(PET) || m_flags.has_flag(PET_EXOTIC)){
        m_flags.set_flag(TRAINABLE,true);
    }

    if(m_df->enumerate_vector(m_address + m_mem->caste_offset("extracts")).count() > 0){
        m_flags.set_flag(HAS_EXTRACTS,true);
    }
    int offset = m_mem->caste_offset("shearable_tissues_vector");
    if(offset != -1){
        if(m_df->enumerate_vector(m_address + offset).count() > 0){
            m_flags.set_flag(SHEARABLE,true);
        }
    }
    //raws can have both fishable and non-fishable, but for our purposes we only care that it can be fished
    //so ensure that both flags are false if the caste is not fishable
    if(m_flags.has_flag(NO_FISH)){
        m_flags.set_flag(FISHABLE,false);
    }
}

void Caste::load_skill_rates(){
    if(m_skill_rates.count() <= 0){
        VIRTADDR addr = m_address + m_mem->caste_offset("skill_rates");
        int val;
        int skill_count = GameDataReader::ptr()->get_total_skill_count();
        for(int skill_id=0; skill_id < skill_count; skill_id++){
            val = (int)m_df->read_int(addr);
            m_skill_rates.insert(skill_id, val);
            if((val-100) >= 25)
                m_bonuses.append(GameDataReader::ptr()->get_skill_name(skill_id));
            addr += 0x4;
            if(!DT->show_skill_learn_rates && val != 100)
                DT->show_skill_learn_rates = true;
        }
    }
}

int Caste::get_skill_rate(int skill_id){
    if(m_skill_rates.count() <= 0)
        load_skill_rates();

    return(m_skill_rates.value(skill_id,100));
}

void Caste::load_attribute_info(){
    //physical attributes (seems mental attribs follow so load them all at once)
    VIRTADDR base = m_address + m_mem->caste_offset("caste_phys_att_ranges");
    VIRTADDR base_caps = m_address + m_mem->caste_offset("caste_att_caps");
    VIRTADDR base_rates = m_address + m_mem->caste_offset("caste_att_rates");
    int perc = 200; //the percentage of improvement default is 200
    int limit = 5000; //absolute maximum any dwarf can achieve (last raw bin * perc)
    int median = 0;
    int display_max = 0; //maximum display descriptor value, seems to be the median + 1000?
    int cost_to_improve = 500; //cost to improve default is 500
    for (int i=0; i<19; i++)
    {
        att_range r;
        for (int j=0; j<7; j++){
            int val = m_df->read_int(base + i*28 + j*4);
            r.raw_bins.append(val);
        }
        median = r.raw_bins.at(3);
        display_max = median + 1000; //maybe this is based on the perc below?

        //add a bin between the max raw value, and the 5000 limit, based on the max %
        perc = m_df->read_int(base_caps + i*4);
        limit = r.raw_bins.at(6) * (perc/100);
        r.raw_bins.append(limit);

        //add the absolute max
        r.raw_bins.append(5000);

        //also save the cost to improve for this attribute for the caste
        cost_to_improve = m_df->read_int(base_rates + i*16);
        m_attrib_costs.insert(i,cost_to_improve);

        //now load the display/descriptor ranges
        for(int k=0; k<9; k++){
            //avoid the median
            if(k!=4)
                r.display_bins.prepend(display_max < 0 ? 0 : display_max);
            display_max -= 250; //game spaces by 250 per descriptor bin
        }
        m_attrib_ranges.insert(i,r);
    }
}

QPair<int, QString> Caste::get_attribute_descriptor_info(ATTRIBUTES_TYPE id, int value){
    if(m_attrib_ranges.count()<=0)
        load_attribute_info();

    QPair<int, QString> ret;
    att_range r = m_attrib_ranges.value((int)id);
    if(value >= r.display_bins.at(r.display_bins.length()-1)){
        ret.first = r.display_bins.length();
        ret.second = Attribute::find_descriptor(id,ret.first);
    }else{
        for(int i=0; i < r.display_bins.length(); i++){
            if(value <= r.display_bins.at(i)){
                ret.first = i;
                ret.second = Attribute::find_descriptor(id,i);
                break;
            }
        }
    }

    //only append the caste's name to our playable race (don't do this for tame animals in the fort)
    if(DT->multiple_castes && m_race->race_id() == m_df->dwarf_race_id()){
        ret.second == "" ? ret.second = QObject::tr("Average") : ret.second;
        ret.second = QObject::tr("%1 for a %2.").arg(ret.second).arg(m_name);
    }

    return ret;
}

int Caste::get_attribute_cost_to_improve(int id){
    if(m_attrib_ranges.count()<=0)
        load_attribute_info();
     return m_attrib_costs.value(id);
}

BodyPart* Caste::get_body_part(int body_part_id){
    if(body_part_id >= 0 && body_part_id < m_body_parts_addr.size()){
        if(!m_body_parts.keys().contains(body_part_id)){
            BodyPart *bp = new BodyPart(m_df,m_race,m_body_parts_addr.at(body_part_id),body_part_id);
            m_body_parts.insert(body_part_id,bp);
        }
        return m_body_parts.value(body_part_id);
    }else{
        return new BodyPart();
    }
}

QString Caste::description(){
    if(m_bonuses.count() > 0){
        QString list = m_bonuses.join(", ");
        list = list.replace(list.lastIndexOf(","),2," and ");

        return tr("%1 These %2 gain a significant xp bonus for <b>%3</b>.")
                .arg(m_description)
                .arg(m_race->plural_name())
                .arg(list);
    }else{
        return m_description;
    }

}
