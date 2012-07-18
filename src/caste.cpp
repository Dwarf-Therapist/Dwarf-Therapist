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
#include "dfinstance.h"
#include "memorylayout.h"
#include "truncatingfilelogger.h"
#include <QtDebug>
#include "gamedatareader.h"


Caste::Caste(DFInstance *df, VIRTADDR address, QString race_name, QObject *parent)
    : QObject(parent)
    , m_address(address)
    , m_race_name(race_name)
    , m_tag(QString::null)
    , m_name(QString::null)
    , m_description(QString::null)
    , m_df(df)
    , m_mem(df->memory_layout())
{
    load_data();
}

Caste::~Caste() {
}

Caste* Caste::get_caste(DFInstance *df, const VIRTADDR & address, QString race_name) {
    return new Caste(df, address, race_name);
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
}

void Caste::load_attribute_info(){
    //physical attributes (seems mental attribs follow so load them all at once)
    VIRTADDR base = m_address + m_mem->caste_offset("caste_phys_att_ranges");
    for (int i=0; i<19; i++)
    {
        att_range r;
        int max = m_df->read_int(base + i*28 + 12) + 1000; //max is median + 1000

        //load the raws, add the 0-first bin
        r.raw_bins.append(0);
        for (int j=0; j<7; j++)
        {
            int val = m_df->read_int(base + i*28 + j*4);            
            r.raw_bins.append(val);
            //TRACE << m_name << " " << i << " " << j << " " << val;
        }
        //add the top range - 5000 bin
        r.raw_bins.append(5000);


        //now load the display/descriptor ranges
        for(int k=0; k<9; k++){
            //avoid the median
            if(k!=4)
                r.display_bins.prepend(max < 0 ? 0 : max);
            max -= 250; //game spaces by 250 per description bin
        }

        m_ranges.insert(i,r);
    }
}

Caste::attribute_level Caste::get_attribute_level(int id, int value)
{
    if(m_ranges.count()<=0)
        load_attribute_info();

    att_range r = m_ranges.value(id);
    attribute_level l;
    l.rating = 0;
    int idx = 0;
    Attribute *a = GameDataReader::ptr()->get_attribute(id);
    for(int i=0; i < r.display_bins.length(); i++){
        if(value <= r.display_bins.at(i)){
            if(i!=4){
                if(i<4) //the middle bin is always 0
                    idx=4;
                else if(i>4)
                    idx=5;
                l.rating = r.display_bins.at(i) - r.raw_bins.at(idx);
                if(l.rating==0)
                    l.rating=250;
                l.rating /= 66.66; //this is for our drawing rating (-15  -> +15)
            }            
            l.description = a->m_display_descriptions.at(i);
            return l;
        }
    }
    l.description = a->m_display_descriptions.at(a->m_display_descriptions.count()-1);
    l.rating = 20;

    return l;
}


int Caste::get_body_size(int index){
    if(m_body_sizes.size()>index)
        return m_body_sizes.at(index);
    else
        return 0;
}

