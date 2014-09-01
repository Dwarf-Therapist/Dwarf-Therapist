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
#include "belief.h"
#include "gamedatareader.h"

//personality facets
Belief::Belief(int id, QSettings &s, QObject *parent)
    : QObject(parent)
{
    this->m_id = id;
    name = s.value("name", "UNKNOWN").toString();

    m_level_string[-50] = s.value("level_0", "").toString();
    m_level_string[-40] = s.value("level_1", "").toString();
    m_level_string[-25] = s.value("level_2", "").toString();
    m_level_string[-10] = s.value("level_3", "").toString();
    m_level_string[11] = s.value("level_4", "").toString();
    m_level_string[26] = s.value("level_5", "").toString();
    m_level_string[41] = s.value("level_6", "").toString();
}

bool Belief::is_active(const short &personal_val){
    //if the cultural value is between the neutral ranges (-10 to 10) it will be active outside that range
    //otherwise, use the range where the cultural value was found as the limits for active or not
    //eg. cultural norm is 15, so anything outside the 11 to 26 range is active
    int entity_val = DT->get_DFInstance()->fortress()->get_belief_value(m_id);
    if(abs(entity_val) <= 10){
        return (abs(personal_val) > 10);
    }else{
        QMapIterator<int,QString> i(m_level_string);
        i.toBack();
        while(i.hasPrevious()){
            i.previous();
            if(entity_val >= i.key()){
                return (personal_val < i.key() || (i.hasPrevious() && personal_val >= i.peekPrevious().key()));
            }
        }
        return false;
    }
}

QString Belief::level_message(const short &val){
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

void Belief::add_conflict(int trait_id){
    if(!m_trait_conflicts.contains(trait_id))
        m_trait_conflicts.append(trait_id);
}

QString Belief::trait_conflict_names(){
    QStringList items;
    foreach(int trait_id, m_trait_conflicts){
        items.append(GameDataReader::ptr()->get_trait_name(trait_id));
    }
    return nice_list(items);
}
