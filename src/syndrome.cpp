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

#include "syndrome.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "gamedatareader.h"
#include "truncatingfilelogger.h"

Syndrome::Syndrome()
    : m_df(0x0)
    , m_mem(0x0)
    , m_addr(0)
    , m_transform_race(-1)
    , m_has_transform(false)
    , m_is_sickness(false)
    , m_name("Unknown")
    , m_id(-1)
{
}

Syndrome::Syndrome(DFInstance *df, VIRTADDR addr)
    : m_df(df)
    , m_mem(m_df->memory_layout())
    , m_addr(addr)
    , m_transform_race(-1)
    , m_has_transform(false)
{
    m_id = m_df->read_int(addr);
    m_is_sickness = m_df->read_byte(m_addr + m_mem->dwarf_offset("syn_sick_flag"));

    VIRTADDR syn_addr = m_df->get_syndrome(m_id);
    if(syn_addr){
        m_name = capitalizeEach(m_df->read_string(syn_addr));
        //SYN_CLASS tokens
        QRegExp rx = QRegExp("[-_\\*~@#\\^]");
        QVector<VIRTADDR> classes_addr = m_df->enumerate_vector(syn_addr + m_mem->syndrome_offset("syn_classes_vector"));
        foreach(VIRTADDR class_addr, classes_addr){
            QString class_name = m_df->read_string(class_addr);
            class_name = class_name.replace(rx," ");
            if(!class_name.trimmed().isEmpty())
                m_class_names.append(capitalizeEach(class_name.trimmed().toLower()));
        }

        QVector<VIRTADDR> effects = m_df->enumerate_vector(syn_addr + m_mem->syndrome_offset("cie_effects"));
        foreach(VIRTADDR ce_addr, effects){
            VIRTADDR vtable = m_df->read_addr(ce_addr);
            int type = m_df->read_int(m_df->read_addr(vtable)+m_df->VM_TYPE_OFFSET());
            int end = m_df->read_int(ce_addr + m_mem->syndrome_offset("cie_end"));
            if(type==25){
                //physical attribute changes
                LOGD << "reading syndrome type" << type << "(phys_att_change)";
                load_attribute_changes(ce_addr,(int)AT_STRENGTH,5,m_mem->syndrome_offset("cie_phys"),end);
            }else if(type==26){
                //mental attribute changes
                LOGD << "reading syndrome type" << type << "(ment_att_change)";
                load_attribute_changes(ce_addr,(int)AT_ANALYTICAL_ABILITY,12,m_mem->syndrome_offset("cie_ment"),end);
            }else if(type==24){
                //transformation
                LOGD << "reading syndrome type" << type << "(transformation)";
                m_has_transform = true;
                if(m_mem->is_valid_address(m_mem->syndrome_offset("trans_race_id")))
                    m_transform_race = m_df->read_int(ce_addr + m_mem->syndrome_offset("trans_race_id"));
            }
        }
    }else{
        m_name = "??";
    }
}

Syndrome::~Syndrome(){
    m_df = 0;
}

QString Syndrome::display_name(bool show_name,bool show_class){
    QString c_names = "???";
    if(m_class_names.count() > 0)
        c_names = m_class_names.join(", ");

    if(show_name && show_class){
        if(m_name.isEmpty()){
            return c_names;
        }else{
            if(m_class_names.count() > 0)
                return QString("%1 (%2)").arg(m_name).arg(c_names);
            else
                return m_name;
        }
    }
    //name only, but no name, show the class
    if(show_name){
        if(m_name.isEmpty())
            return c_names;
        else
            return m_name;
    }
    //class only, but no classes, show the name
    if(show_class){
        if(m_class_names.count() > 0)
            return c_names;
        else if(!m_name.isEmpty())
            return m_name;
    }

    return "???";
}

void Syndrome::load_attribute_changes(VIRTADDR addr, int start_idx, int count, int add_offset, int end){
    int idx_offset = 0;
    int idx = 0;
    for(idx = 0; idx <=count; idx++){
        idx_offset = sizeof(VIRTADDR) * idx;
        syn_att_change att_change;
        att_change.percent = m_df->read_int(addr+m_mem->syndrome_offset("cie_first_perc")+idx_offset);
        att_change.added = m_df->read_int(addr+add_offset+idx_offset);
        if(end < 0)
            att_change.is_permanent = true;
        else
            att_change.is_permanent = false;

        //only keep track of attribute changes that actually... change something
        if((att_change.percent != 0 && att_change.percent != 100) || att_change.added != 0){
            m_attribute_changes.insert(static_cast<ATTRIBUTES_TYPE>(idx+start_idx),att_change);
        }
    }
}

QString Syndrome::syn_effects(){
    if(m_syn_effects.count() <= 0 && m_attribute_changes.count() > 0){
        foreach(ATTRIBUTES_TYPE a_type,m_attribute_changes.keys()){
            QStringList att_effects;
            QString att_desc = GameDataReader::ptr()->get_attribute_name(a_type).left(3);
            int perc = m_attribute_changes.value(a_type).percent;
            int add = m_attribute_changes.value(a_type).added;
            if(perc != 100 && perc != 0)
                att_effects.append(QString((perc > 0) ? "+" : "-").append(QString::number(perc)).append("%"));
            if(add != 0)
                att_effects.append(QString(add > 0 ? "+" : "-").append(QString::number(add)));
            att_desc.append(" " + att_effects.join("|"));
            m_syn_effects.append(att_desc);
        }
    }
    return m_syn_effects.join(", ");
}
