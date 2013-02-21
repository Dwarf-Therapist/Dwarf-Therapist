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
#include <QtDebug>
#include "caste.h"
#include "attribute.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "truncatingfilelogger.h"
#include "gamedatareader.h"
#include "flagarray.h"

#include "dwarfstats.h"

Caste::Caste(DFInstance *df, VIRTADDR address, int race_id, QString race_name, QObject *parent)
    : QObject(parent)
    , m_address(address)
    , m_race_id(race_id)
    , m_race_name(race_name)
    , m_tag(QString::null)
    , m_name(QString::null)
    , m_name_plural(QString::null)
    , m_description(QString::null)
    , m_df(df)
    , m_mem(df->memory_layout())
    , m_flags()
    , m_has_extracts(false)    
{
    load_data();
}

Caste::~Caste() {
    m_body_sizes.clear();
    m_attrib_ranges.clear();
    m_skill_rates.clear();    
}

Caste* Caste::get_caste(DFInstance *df, const VIRTADDR & address, int race_id, QString race_name) {
    return new Caste(df, address, race_id, race_name);
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
//    //needed for mods with caste's name not specified

//    !!!!doesn't work with mods that require hidden castes. mod creators shouldn't be creating castes with no names unless they want them hidden...

//    if (m_name.toLower()==m_race_name.toLower())
//    {
//        if ((m_tag.toLower()!="male")&&(m_tag.toLower()!="female"))
//        {
//            m_name = capitalizeEach(m_tag.remove("_MALE").remove("_FEMALE").replace("_"," ").toLower());
//        }
//    }
    m_description = m_df->read_string(m_address + m_mem->caste_offset("caste_descr"));

//    could update this to read the child/baby sizes instead of just the adult size
//    QVector<VIRTADDR> body_sizes = m_df->enumerate_vector(m_address + m_mem->caste_offset("body_sizes_vector"));
//    foreach(VIRTADDR size, body_sizes){
//        m_body_sizes.prepend((int)size);
//    }
    m_body_sizes.append(m_df->read_int(m_address + m_mem->caste_offset("adult_size")));
    m_flags = FlagArray(m_df, m_address + m_mem->caste_offset("flags"));

    QVector<uint> extracts = m_df->enumerate_vector(m_address + m_mem->caste_offset("extracts"));
    if(extracts.count() > 0)
        m_has_extracts = true;

}

bool Caste::is_trainable(){
    if(m_flags.has_flag(TRAINABLE_HUNTING) || m_flags.has_flag(TRAINABLE_WAR) ||
            m_flags.has_flag(PET) || m_flags.has_flag(PET_EXOTIC)){
        return true;
    }else{
        return false;
    }
}

bool Caste::is_milkable(){
    return m_flags.has_flag(MILKABLE);
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
        }
    }
}

int Caste::get_skill_rate(int skill_id){
    if(m_skill_rates.count() <= 0)
        load_skill_rates();

    return(m_skill_rates.value(skill_id,100));
}

void Caste::load_attribute_info(float ratio){
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
        if(ratio > -1)
            DwarfStats::load_att_caste_bins(i,ratio,r.raw_bins);
    }    
}

QPair<int, QString> Caste::get_attribute_descriptor_info(ATTRIBUTES_TYPE id, int value){
    if(m_attrib_ranges.count()<=0)
        load_attribute_info();

    QPair<int, QString> ret;
    att_range r = m_attrib_ranges.value((int)id);
    if(value >= r.display_bins.at(r.display_bins.length()-1)){
        ret.first = r.display_bins.length()-1;
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
    if(DT->multiple_castes && m_race_id == m_df->dwarf_race_id()){
        ret.second == "" ? ret.second = QObject::tr("Average") : ret.second;
        ret.second = QObject::tr("%1 for a %2.").arg(ret.second).arg(m_name);
    }

    return ret;
}

void Caste::load_trait_info(){
//    if(m_trait_ranges.count() <= 0){
//        VIRTADDR base = m_address + 0x4ec;
//        for (int i=0; i<30; i++)
//        {
//            QList<short> ranges;
//            ranges.append(m_df->read_short(base));//min
//            ranges.append(m_df->read_short(base + 0x003c));//median
//            ranges.append(m_df->read_short(base + 0x0078));//max
//            m_trait_ranges.insert(i,ranges);
//            base += 0x2;
//        }
//    }
}



int Caste::get_body_size(int index){
    if(m_body_sizes.size()>index)
        return m_body_sizes.at(index);
    else
        return 0;
}

QString Caste::description(){
    if(m_bonuses.count() > 0){
        QString list = m_bonuses.join(", ");
        list = list.replace(list.lastIndexOf(","),2," and ");

        return tr("%1 These %2 gain a significant xp bonus for <b>%3</b>.")
                .arg(m_description)
                .arg(m_race_name)
                .arg(list);
    }else{
        return m_description;
    }

}
