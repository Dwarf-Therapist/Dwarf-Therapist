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
#ifndef SYNDROME_H
#define SYNDROME_H

#include <QObject>
#include <QString>
#include <utils.h>
#include <dfinstance.h>
#include <memorylayout.h>
#include <gamedatareader.h>

class Syndrome{

public:    
    struct syn_att_change{
        int percent;
        int added;
        bool is_permanent;
    };

    Syndrome(){
        m_is_sickness = false;
        m_name   = "Unknown";
        m_df = 0x0;
        m_addr = 0;
    }

    Syndrome(DFInstance *df, VIRTADDR addr)
    {
        m_df = df;
        m_addr = addr;
        m_mem = m_df->memory_layout();

//        m_year = m_df->read_int(m_addr + 0x4);
//        m_time = m_df->read_int(m_addr + 0x8);

        m_id = m_df->read_int(addr);
        m_is_sickness = m_df->read_byte(m_addr + m_mem->dwarf_offset("syn_sick_flag"));     

        VIRTADDR syn_addr = m_df->get_syndrome(m_id);
        if(syn_addr > 0){
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
                int type = m_df->read_int(m_df->read_addr(vtable)+0x1);
                int end = m_df->read_int(ce_addr + m_mem->syndrome_offset("cie_end"));
                if(type ==25){
                    //physical attribute changes
                    load_attribute_changes(ce_addr,(int)AT_STRENGTH,5,m_mem->syndrome_offset("cie_phys"),end);
                }else if(type==26){
                    //mental attribute changes
                    load_attribute_changes(ce_addr,(int)AT_ANALYTICAL_ABILITY,12,m_mem->syndrome_offset("cie_ment"),end);
                }
            }
        }
    }

    QStringList class_names() {return m_class_names;}
    QString name() {return m_name;}
    int id() {return m_id;}
    bool is_sickness() {return m_is_sickness;}    

    QString display_name(bool show_name = true,bool show_class = true){
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

    void load_attribute_changes(VIRTADDR addr, int start_idx, int count, int add_offset, int end){
        int idx_offset = 0;
        int idx = 0;        
        for(idx = 0; idx <=count; idx++){
            idx_offset = 0x4 * idx;
            syn_att_change att_change;
            att_change.percent = m_df->read_int(addr+m_mem->syndrome_offset("cie_first_perc")+idx_offset);
            att_change.added = m_df->read_int(addr+add_offset+idx_offset);
            if(end < 0)
                att_change.is_permanent = true;
            else
                att_change.is_permanent = false;

            //only keep track of attribute changes that actually... change something
            if((att_change.percent != 0 && att_change.percent != 100) || att_change.added != 0){
                //m_attribute_changes.insert(static_cast<ATTRIBUTES_TYPE>(idx+start_idx),att_change);
                m_attribute_changes.insert(static_cast<ATTRIBUTES_TYPE>(idx+start_idx),att_change);
            }
        }
    }

    QString syn_effects(){
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

    QHash<ATTRIBUTES_TYPE, syn_att_change> get_attribute_changes() {return m_attribute_changes;}

    bool operator==(const Syndrome &other) const {
        if(this == &other)
            return true;
        return (this->m_id == other.m_id);
    } 

private:
    DFInstance *m_df;
    MemoryLayout *m_mem;
    VIRTADDR m_addr;
    bool m_is_sickness;
    QString m_name;
    QStringList m_class_names;
    QStringList m_syn_effects;
    int m_id;
    int m_year;
    int m_time;    
    //pairs of percent change, and flat additive changes
    QHash<ATTRIBUTES_TYPE, syn_att_change> m_attribute_changes;

};

#endif // SYNDROME_H

