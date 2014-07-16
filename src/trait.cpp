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

//personality facets
Trait::Trait(int trait_id, QSettings &s, QObject *parent)
    : QObject(parent)
{
    this->trait_id = trait_id;
    name = s.value("name", "UNKNOWN").toString();
    inverted = s.value("inverted",false).toBool();

    //read in conflicting beliefs
    int count = s.beginReadArray("conflicts");
    int belief_id;
    for(int i = 0; i < count; i++) {
        s.setArrayIndex(i);
        belief_id = s.value("belief_id").toInt();
        conflict c;
        c.limit = s.value("limit").toInt();
        c.beliefl_id = abs(belief_id);
        c.gains_skill = belief_id > 0 ? true : false;
        m_conflicts.insert(c.beliefl_id,c);
    }
    s.endArray();

    //read in special notes
    count = s.beginReadArray("special");
    for(int i = 0; i < count; i++) {
        s.setArrayIndex(i);
        m_special.insert(s.value("msg").toString(), s.value("limit").toInt());
    }
    s.endArray();

    //read in custom limits (these are the minimum values for each range)
    count = s.beginReadArray("limits");
    for(int i = 0; i < count; i++) {
        s.setArrayIndex(i);
        m_limits.append(s.value("min").toInt());
    }
    s.endArray();

    if(m_limits.count() <= 0){
        m_level_string[0]  = s.value("level_0", "").toString();
        m_level_string[10] = s.value("level_1", "").toString();
        m_level_string[25] = s.value("level_2", "").toString();
        m_level_string[61] = s.value("level_3", "").toString();
        m_level_string[76] = s.value("level_4", "").toString();
        m_level_string[91] = s.value("level_5", "").toString();
    }else{
        for(int i = 0; i < m_limits.length(); i++){
            m_level_string[m_limits.at(i)] = s.value(QString("level_%1").arg(QString::number(i))).toString();
        }
    }
}

QString Trait::level_message(const short &val){
    QString ret_val;
    QMapIterator<int,QString> i(m_level_string);
    i.toBack();
    while(i.hasPrevious()){
        i.previous();
        if(val >= i.key()){
            ret_val = i.value();
            break;
        }
    }
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
    Q_UNUSED(skill_id);
    Q_UNUSED(val);
    //TODO: still unknown effects in DF2014
//    if(m_conflicts.contains(skill_id)){
//        conflict c = m_conflicts.value(skill_id);

//        if((c.limit < 0 && abs(val) < abs(c.limit)) || (c.limit > 0 && abs(val) > abs(c.limit))){
//            return tr("%1 be a %2")
//                    .arg(!c.gains_skill ? tr("Cannot") : tr("Can"))
//                    .arg(GameDataReader::ptr()->get_skill_name(abs(skill_id)));
//        }
//    }
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
