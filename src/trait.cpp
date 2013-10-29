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
#include "trait.h"
#include "gamedatareader.h"
#include <QtWidgets>

Trait::Trait(int trait_id, QSettings &s, QObject *parent)
    : QObject(parent)
{
    this->trait_id = trait_id;
    name = s.value("name", "UNKNOWN").toString();
    inverted = s.value("inverted",false).toBool();

    //read in conflicting skills
    int count = s.beginReadArray("conflicts");
    int skill_id;
    for(int i = 0; i < count; i++) {
        s.setArrayIndex(i);
        skill_id = s.value("skill_id").toInt();
        conflict c;
        c.limit = s.value("limit").toInt();
        c.skill_id = abs(skill_id);
        c.gains_skill = skill_id > 0 ? true : false;
        m_conflicts.insert(c.skill_id,c);
    }
    s.endArray();

    //read in special notes
    count = s.beginReadArray("special");
    for(int i = 0; i < count; i++) {
        s.setArrayIndex(i);
        m_special.insert(s.value("msg").toString(), s.value("limit").toInt());
    }
    s.endArray();

    QString unk = tr("UNKNOWN TRAIT (%1)").arg(name);

    m_level_string[0]  = s.value("level_0", unk).toString();
    m_level_string[10] = s.value("level_1", unk).toString();
    m_level_string[25] = s.value("level_2", unk).toString();
    m_level_string[61] = s.value("level_3", unk).toString();
    m_level_string[76] = s.value("level_4", unk).toString();
    m_level_string[91] = s.value("level_5", unk).toString();

    //setup aspect types and load bins
    QList<int> raws;
    raws << -1 << 11 << 27 << 44 << 65 << 78 << 92 << 100;

    //immoderation (urge) and straightforwardness (honesty) are +
    if(trait_id==4 || trait_id==19)
        m_aspect_type = positive;
    else if(trait_id==5) //vulnerability (stress) is -
        m_aspect_type = negative;
    else //everything else
        m_aspect_type = average;

    DwarfStats::load_trait_bins(m_aspect_type, raws);
}

QString Trait::level_message(const short &val){
    QString ret_val;
    if (val >= 91)
        ret_val = m_level_string[91];
    else if (val >= 76)
        ret_val = m_level_string[76];
    else if (val >= 61)
        ret_val = m_level_string[61];
    else if (val >= 25)
        ret_val = m_level_string[25];
    else if (val >= 10)
        ret_val = m_level_string[10];
    else
        ret_val = m_level_string[0];

    return ret_val;
}


QString Trait::conflicts_messages(const short &val){
    QStringList items;
    QString msg;
    foreach(int skill_id, m_conflicts.uniqueKeys()){
        msg = conflict_message(skill_id,val);
        if(!msg.isEmpty())
            items.append(msg);
    }
    return items.join(tr(" and "));
}

QString Trait::conflict_message(const short &skill_id, const short &val){
    if(m_conflicts.contains(skill_id)){
        conflict c = m_conflicts.value(skill_id);

        if((c.limit < 0 && abs(val) < abs(c.limit)) || (c.limit > 0 && abs(val) > abs(c.limit))){
            return tr("%1 be a %2")
                    .arg(!c.gains_skill ? tr("Cannot") : tr("Can"))
                    .arg(GameDataReader::ptr()->get_skill_name(abs(skill_id)));
        }
    }
    return "";
}

QString Trait::special_messages(const short &val){
    QStringList items;
    int limit;
    bool include;
    foreach(QString msg, m_special.uniqueKeys()){
        include = false;
        limit = m_special.value(msg);

        if((limit < 0 && abs(val) < abs(limit)) || (limit > 0 && abs(val) > abs(limit)))
            include = true;

        if(include){
            items.append(msg);
        }
    }
    return items.join(tr(" and "));
}

bool Trait::default_ranges(int id, short min, short median, short max){
    //immoderation (urge) and straightforwardness (honesty) are +
    if(id==4 || id==19)
        return (min==0 && median==55 && max==100);
    else if(id==5) //vulnerability (stress) is -
        return (min==0 && median==45 && max==100);
    else //everything else
        return (min==0 && median==50 && max==100);
}
