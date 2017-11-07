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

#include "unitemotion.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "gamedatareader.h"
#include "thought.h"
#include "subthoughttypes.h"
#include "emotion.h"

#include <QRegExp>

UnitEmotion::UnitEmotion(QObject *parent)
    : QObject(parent)
    , m_address(0x0)
    , m_eType(EM_NONE)
    , m_thought_id(-1)
    , m_sub_id(-1)
    , m_year(-1)
    , m_year_tick(-1)
    , m_desc("??")
    , m_desc_colored("??")
    , m_count(1)
    , m_strength(50)
    , m_effect(1)
    , m_total_effect(0)
    , m_intensifier(0)
    , m_optional_level(-1)
    , m_compare_id("")
    , m_date_in_ticks(-1)
{
}

UnitEmotion::UnitEmotion(VIRTADDR addr, DFInstance *df, QObject *parent)
    : QObject(parent)
    , m_address(addr)
    , m_desc("??")
    , m_desc_colored("??")
    , m_count(1)
    , m_effect(0)
    , m_total_effect(0)
    , m_intensifier(0)
    , m_optional_level(-1)
    , m_compare_id("")
{
    MemoryLayout *m_mem = df->memory_layout();
    m_eType = static_cast<EMOTION_TYPE>(df->read_int(addr+m_mem->emotion_offset("emotion_type")));
    m_strength = df->read_int(addr+m_mem->emotion_offset("strength"));
    m_thought_id = df->read_int(addr+m_mem->emotion_offset("thought_id"));
    m_sub_id = df->read_int(addr+m_mem->emotion_offset("sub_id"));
    m_optional_level = df->read_int(addr+m_mem->emotion_offset("level"));
    m_year = df->read_int(addr+m_mem->emotion_offset("year"));
    m_year_tick = df->read_int(addr+m_mem->emotion_offset("year_tick"));
    m_date_in_ticks = m_year * df->ticks_per_year + m_year_tick;

    GameDataReader *gdr = GameDataReader::ptr();
    Thought *t = gdr->get_thought(m_thought_id);
    if(t){
        m_desc = t->desc();

        //for some thoughts (witnessed death) the sub id should be referencing a historical figure
        //but it's unknown what the sub id is referencing. it appears to be an index, but to an unknown vector

        if(m_sub_id != -1 || m_optional_level != -1){
            SubThoughtTypes *s_types = gdr->get_subthought_types(t->subtype());
            if(s_types){
                int id = m_sub_id;
                if(m_sub_id == -1)
                    id = m_optional_level;
                QString s_thought = s_types->get_subthought(id);
                if(s_types->has_placeholder()){
                    m_desc.replace(s_types->get_placeholder(),s_thought);
                    m_compare_id = id;
                }else{
                    m_desc.append(s_thought);
                }
            }else{
                QRegExp re(".*(\\[.*\\]).*");
                int idx = re.indexIn(m_desc);
                if(idx > -1){
                    m_compare_id = m_sub_id; //don't consider thoughts with skills, syndromes, poems, etc. the same
                    QString key = re.cap(1);
                    QString replace = "";
                    if(key=="[skill]"){
                        replace = gdr->get_skill_name(m_sub_id,(m_thought_id == 11));
                    }else if(key=="[building]"){
                        replace = gdr->get_building_name(static_cast<BUILDING_TYPE>(m_sub_id),m_optional_level);
                    }else if(key=="[syndrome]"){
                        VIRTADDR syn_addr = df->get_syndrome(m_sub_id);
                        if(syn_addr){
                            replace = df->read_string(syn_addr);
                            m_compare_id = replace;
                        }
                    }else if(key=="[poetry]"){
                        replace = df->get_preference_other_name(m_sub_id,LIKE_POETRY);
                    }else if(key=="[music]"){
                        replace = df->get_preference_other_name(m_sub_id,LIKE_MUSIC);
                    }else if(key=="[dance]"){
                        replace = df->get_preference_other_name(m_sub_id,LIKE_DANCE);
                    }
                    if(replace != "")
                        m_desc.replace(key,replace);
                }
            }
        }
    }

    Emotion *e = gdr->get_emotion(m_eType);
    if(e){
        m_desc_colored = QString("<font color=%1>%2</font> ").arg(e->get_color().name()).arg(e->get_name()) + m_desc;
        m_desc = e->get_name() + " " + m_desc;

        m_intensifier = e->get_divider();
        if(m_intensifier != 0){
            m_effect = m_strength / m_intensifier;
        }
    }
}

QString UnitEmotion::get_desc(bool colored){
    if(colored){
        return m_desc_colored;
    }else{
        return m_desc;
    }
}

int UnitEmotion::set_effect(int stress_vuln){
    //modify the base effect by the unit's vulnerability to stress
    float multiplier = 1.0;
    if(stress_vuln >= 91){
        multiplier = 5.0;
    }else if(stress_vuln >= 76){
        multiplier = 3.0;
    }else if(stress_vuln >= 61){
        multiplier = 2.0;
    }else if(stress_vuln <= 9){
        multiplier = 0;
        m_intensifier = 0;
    }else if(stress_vuln <= 24){
        multiplier = 0.25;
    }else if(stress_vuln <= 39){
        multiplier = 0.5;
    }
    m_total_effect = m_effect * multiplier;
    return m_total_effect;
}

short UnitEmotion::get_stress_effect() const{
    if(m_total_effect > 0 || m_intensifier > 0)
        return 1;
    else if(m_total_effect < 0 || m_intensifier < 0)
        return -1;
    else
        return 0;
}

bool UnitEmotion::equals(const UnitEmotion &other){
    return (this->m_thought_id == other.m_thought_id && this->m_eType == other.m_eType &&
            this->m_compare_id == other.m_compare_id);
}
