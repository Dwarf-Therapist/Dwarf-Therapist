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
#include "gamedatareader.h"
#include "thought.h"
#include "subthought.h"
#include "emotion.h"

#include "QDebug"

UnitEmotion::UnitEmotion(QObject *parent)
    : QObject(parent)
    , m_address(0x0)
    , m_eType(EM_NONE)
    , m_thought_id(-1)
    , m_subthought_id(-1)
    , m_year(-1)
    , m_year_tick(-1)
    , m_desc("??")
    , m_desc_colored("??")
    , m_count(1)
    , m_strength(50)
    , m_effect(1)
    , m_total_effect(0)
    , m_intensifier(0)
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
{
    //emotions
    /*
    "unit_personality::anon4","1","0x0","0x4","emotion_type","type","",""
    "unit_personality::anon4","1","0x4","0x4","int32_t","unk2","",""
    "unit_personality::anon4","1","0x8","0x4","int32_t","strength","",""
    "unit_personality::anon4","1","0xc","0x4","unit_thought_type","thought","",""
    "unit_personality::anon4","1","0x10","0x4","int32_t","subthought","","for certain thoughts"
    "unit_personality::anon4","1","0x14","0x4","int32_t","severity","",""
    "unit_personality::anon4","1","0x18","0x4","int32_t","flags","",""
    "unit_personality::anon4","1","0x1c","0x4","int32_t","unk7","",""
    "unit_personality::anon4","1","0x20","0x4","int32_t","year","",""
    "unit_personality::anon4","1","0x24","0x4","int32_t","year_tick","",""
     */
    int unk2 = df->read_int(addr+0x0004);
    int unk7 = df->read_int(addr+0x001c);
    int sev = df->read_int(addr+0x0014);

    m_eType = static_cast<EMOTION_TYPE>(df->read_int(addr));
    m_strength = df->read_int(addr+0x0008);
    m_thought_id = df->read_int(addr+0x000c);
    m_subthought_id = df->read_int(addr+0x0010);
    m_year = df->read_int(addr+0x0020);
    m_year_tick = df->read_int(addr+0x0024);

    GameDataReader *gdr = GameDataReader::ptr();
    Thought *t = gdr->get_thought(m_thought_id);
    if(t){
        m_desc = t->desc();
        if(m_desc.contains("[quality"))
            qDebug() << "quality" << sev << "thought" << m_desc;
        if(m_desc.contains("unknown",Qt::CaseInsensitive) && t->id() >= 0)
            qDebug() << "unknown thought" << t->id();
        if(m_subthought_id != -1){
            SubThought *s = gdr->get_subthought(t->subtype());
            if(s){
                QString s_thought = s->get_subthought(m_subthought_id);
                if(m_desc.contains("[")){
                    m_desc.replace(s->get_placeholder(),s_thought);
                }else{
                    m_desc.append(s_thought);
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

    if(unk2 != 0 || unk7 != 0 || sev != 0)
        int z = 0;
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
    }else if(stress_vuln <= 39){
        multiplier = 0.5;
    }else if(stress_vuln <= 24){
        multiplier = 0.25;
    }else if(stress_vuln <= 9){
        multiplier = 0;
        m_intensifier = 0;
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
