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
#include "belief.h"
#include <QSettings>

QColor Trait::goal_color = QColor(255,153,0,255);
QColor Trait::belief_color = QColor(32,156,158,255);

//personality facets
Trait::Trait(int trait_id, QSettings &s, QObject *parent)
    : QObject(parent)
{
    this->trait_id = trait_id;
    name = s.value("name", "UNKNOWN").toString();
    inverted = s.value("inverted",false).toBool();

    //read in conflicting skills
    int count = s.beginReadArray("conflicts");
    int id;
    for(int i = 0; i < count; i++) {
        s.setArrayIndex(i);
        id = s.value("skill_id").toInt();
        skill_conflict c;
        c.limit = s.value("limit").toInt();
        c.skill_id = abs(id);
        c.gains_skill = id > 0 ? true : false;
        m_skill_conflicts.insert(c.skill_id,c);
    }
    s.endArray();

    //read in conflicting beliefs
    count = s.beginReadArray("belief_conflicts");
    int belief_id;
    for(int i = 0; i < count; i++) {
        s.setArrayIndex(i);
        belief_id = s.value("belief_id").toInt();
        m_belief_conflicts.append(belief_id);
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
    return capitalize(ret_val);
}

QString Trait::skill_conflicts_msgs(const short &val){
    if(m_skill_conflicts.size() <= 0)
        return "";
    QStringList items;
    QString msg;
    foreach(int skill_id, m_skill_conflicts.uniqueKeys()){
        msg = skill_conflict_msg(skill_id,val);
        if(!msg.isEmpty())
            items.append(msg);
    }
    return items.join(tr(" and "));
}

QString Trait::skill_conflict_msg(const short &skill_id, const short &val){
    //TODO: still unknown effects in DF2014
    if(m_skill_conflicts.contains(skill_id)){
        skill_conflict c = m_skill_conflicts.value(skill_id);

        if((c.limit < 0 && abs(val) < abs(c.limit)) || (c.limit > 0 && abs(val) > abs(c.limit))){
            return tr("%1 be a %2")
                    .arg(!c.gains_skill ? tr("Cannot") : tr("Can"))
                    .arg(GameDataReader::ptr()->get_skill_name(abs(skill_id)));
        }
    }
    return "";
}

QString Trait::belief_conflicts_names(){
    QStringList items;
    foreach(int belief_id, m_belief_conflicts){
        items.append(GameDataReader::ptr()->get_belief_name(belief_id));
    }
    return items.join(tr(" and "));
}

QString Trait::belief_conficts_msgs(short raw_value, QList<UnitBelief> conflicting_beliefs){
    if(conflicting_beliefs.size() <= 0)
        return "";

    QStringList cultural_conflicts;
    QStringList personal_conflicts;
    foreach(UnitBelief ub, conflicting_beliefs){
        short belief_id = ub.belief_id();
        Belief *b = GameDataReader::ptr()->get_belief(belief_id);
        QString msg = QString("%1 (%2)").arg(b->level_message(ub.belief_value()).toLower()).arg(b->name);
        if(ub.is_personal()){
            personal_conflicts.append(msg);
        }else{
            cultural_conflicts.append(msg);
        }
    }
    QStringList combined;
    if(cultural_conflicts.size() > 0)
        combined << (raw_value > 50 ? tr(", but") : tr(", and")) + tr(" is conflicted because their culture %1").arg(formatList(cultural_conflicts));
    if(personal_conflicts.size() > 0)
        combined << tr(" although %1").arg(formatList(personal_conflicts));
    QString msgs = formatList(combined);
    return msgs;
}

QString Trait::special_messages(const short &val){
    if(m_special.size() <=0 )
        return "";
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
