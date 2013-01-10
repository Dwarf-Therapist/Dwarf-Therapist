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
#include "skill.h"
#include "gamedatareader.h"
#include <QtGui>

Skill::Skill()
    : QObject()
    , m_id(-1)
    , m_exp(0)
    , m_actual_exp(0)
    , m_capped_exp(0)
    , m_exp_for_current_level(0)
    , m_exp_for_next_level(1)
    , m_exp_progress(0)
    , m_capped_rating(-1)
    , m_raw_rating(-1)
    , m_name("UNKNOWN")      
    , m_rust_rating("")
    , m_skill_rate(100)
{}

Skill::Skill(short id, uint exp, short rating, int skill_rate)
    : QObject()
    , m_id(id)
    , m_exp(exp)
    , m_actual_exp(exp)
    , m_exp_for_current_level(0)
    , m_exp_for_next_level(exp + 1)
    , m_exp_progress(0)
    , m_raw_rating(rating)
    , m_name("UNKNOWN")     
    , m_rust_rating("")
    , m_skill_rate(skill_rate)    
{    
    m_name = GameDataReader::ptr()->get_skill_name(m_id);
    //defaults
    m_rust_rating = "";
//    QPalette tt;
//    QBrush ttb = tt.light();
//    m_rust_color = ttb.color().name(); //default to current tooltip palette text color
    m_capped_rating = m_raw_rating > 20 ? 20 : m_raw_rating;

    //current xp
    m_actual_exp = m_exp + calc_xp(m_raw_rating);
    //xp capped at 29000 (used in role ratings, as more than +5 legendary doesn't impact jobs)
    if(m_capped_rating==20){
        m_capped_exp = 29000;
    }else{
        m_capped_exp = m_exp + calc_xp(m_capped_rating);
    }

    //current xp for the level
    m_exp_for_current_level = calc_xp(m_raw_rating);
    //xp for next level
    m_exp_for_next_level = calc_xp(m_raw_rating+1);

    m_losing_xp = false;
    if (m_exp_for_next_level && m_exp_for_current_level) {
        m_exp_progress = ((float)m_exp / (float)(m_exp_for_next_level - m_exp_for_current_level)) * 100;
        if(m_exp_progress > 100){ //indicates rusted
            m_exp_progress = 100;
            m_rust_rating = tr("Rusted");
            m_rust_color = "#cc6633";
            m_losing_xp = true;
        }
    }



    //as far as i can tell, the rust stuff is bugged/broken
    //rust is capped at 6, and once it tries to pass this, it removes the rust description
    //and stops calculating rust stuff? no idea..

//    if(m_demotions <= 1){
//        m_rust_rating = "";
//        m_skill_color = "#000000";
//    }
//    else if(m_demotions < 8){
//        m_rust_rating = "Rusty";
//        m_skill_color = "#cc6633";
//    }
//    else{
//        m_rust_rating = "Very Rusty";
//        m_skill_color = "#993300";
//    }

//    if(m_rust > 0){
//        m_rust_rating = tr("Rusted");
//        m_rust_color = "#cc6633";
//    }
}

QString Skill::to_string(bool include_level, bool include_exp_summary) const {
    GameDataReader *gdr = GameDataReader::ptr();

    QString out;    
    out.append(QString("<font color=%1>").arg(m_rust_color));

//    if (include_level){
//        if(raw_rating() > 20)
//            out.append(QString("[%1|%2] ").arg(m_capped_rating).arg(m_raw_rating));
//        else
//            out.append(QString("[%1] ").arg(m_capped_rating));
//    }
    if(include_level)
        out.append(QString("[%1] ").arg(m_raw_rating));

    out.append(QString("<b>%1</b> ").arg(rust_rating()));

    //df still shows the skill names based on the capped rating, not including rust
    QString skill_level = gdr->get_skill_level_name(m_capped_rating);
    QString skill_name = gdr->get_skill_name(m_id);
    if (skill_level.isEmpty())
        out.append(QString("<b>%1</b>").arg(skill_name));
    else
        out.append(QString("<b>%1 %2</b>").arg(skill_level, skill_name));
    if (include_exp_summary)
        out.append(QString(" %1").arg(exp_summary()));

    out.append(QString("</font>"));

    return out;
}

bool Skill::operator<(const Skill *s2) const {
    return m_capped_rating < s2->m_capped_rating;
}

QString Skill::exp_summary() const {
    if (m_capped_rating >= 20) {
        return QString("TOTAL: %L1xp").arg(m_actual_exp);
    }

    return QString("%L1/%L2xp (%L3%)")
            .arg(m_actual_exp)
            .arg(m_exp_for_next_level)
            .arg(m_exp_progress, 0, 'f', 1);
}

int Skill::calc_xp(int level){
    int val = 0;
    for (int i = 0; i < level; ++i) {
        val += 500 + (i * 100);
    }
    return val;
}
