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
#include <QtWidgets>

QHash<int,int> Skill::m_experience_levels = Skill::load_base_xp_levels();

Skill::Skill()
    : m_id(-1)
    , m_exp(0)
    , m_actual_exp(0)
    , m_capped_exp(0)
    , m_exp_for_current_level(0)
    , m_exp_for_next_level(1)
    , m_exp_progress(0)
    , m_capped_level(-1)
    , m_raw_level(-1)
    , m_name("UNKNOWN")      
    , m_rust_rating("")
    , m_skill_rate(100)
    , m_rating(-1)
    , m_balanced_level(-1)
{}

Skill::Skill(short id, uint exp, short rating, int rust, int skill_rate)    
    : m_id(id)
    , m_exp(exp)
    , m_actual_exp(exp)
    , m_exp_for_current_level(0)
    , m_exp_for_next_level(exp + 1)
    , m_exp_progress(0)
    , m_raw_level(rating)
    , m_name("UNKNOWN")     
    , m_rust_rating("")
    , m_skill_rate(skill_rate)
    , m_rust(rust)
    , m_rating(-1)
    , m_balanced_level(-1)
{    
    m_name = GameDataReader::ptr()->get_skill_name(m_id);
    //defaults
    m_rust_rating = "";    
    m_capped_level = m_raw_level > 20 ? 20 : m_raw_level;

    //current xp
    m_actual_exp = m_exp + get_xp_for_level(m_raw_level);
    //xp capped at 29000 (used in role ratings, as more than +5 legendary doesn't impact jobs)
    if(m_capped_level==20){
        m_capped_exp = MAX_CAPPED_XP;
    }else{
        m_capped_exp = m_exp + get_xp_for_level(m_capped_level);
    }

    //current xp for the level
    m_exp_for_current_level = get_xp_for_level(m_raw_level);
    //xp for next level
    m_exp_for_next_level = get_xp_for_level(m_raw_level+1);

    m_losing_xp = false;


    if(m_exp_for_next_level - m_exp_for_current_level > 0)
        m_exp_progress = (float)(m_exp / (float)(m_exp_for_next_level - m_exp_for_current_level)) * 100;

    if(m_exp_progress > 100){ //indicates losing xp
        m_exp_progress = 100;
        m_rust_rating = QObject::tr("Lost XP!");
        m_rust_color = QColor("#B7410E");
        m_losing_xp = true;
    }else{
        //check for normal rusting
        float m_raw_precise = raw_level_precise();
        if(m_raw_precise >= 4 && (m_raw_precise * 0.75) <= m_rust){
            m_rust_rating = QObject::tr("V. Rusty");
            m_rust_color = "#964B00";
        }else if(m_raw_level > 0 && (m_raw_level * 0.5) <= m_rust){
            m_rust_rating = QObject::tr("Rusty");
            m_rust_color = "#CD7F32";
        }
    }
}


QString Skill::to_string(bool include_level, bool include_exp_summary, bool use_color) const {
    GameDataReader *gdr = GameDataReader::ptr();

    bool rusted = false;
    if(!m_rust_rating.isEmpty())
        rusted = true;

    QString out;    

    if(rusted && use_color)
        out.append(QString("<font color=%1>").arg(m_rust_color.name()));

    if(include_level)
        out.append(QString("[%1] ").arg(m_raw_level));

    //df still shows the skill names based on the capped rating, not including rust?
    QString skill_level = gdr->get_skill_level_name(m_capped_level);
    QString skill_name = gdr->get_skill_name(m_id);
    if (skill_level.isEmpty())
        out.append(QString("<b>%1</b>").arg(skill_name));
    else
        out.append(QString("<b>%1 %2</b>").arg(skill_level, skill_name));
    if (include_exp_summary)
        out.append(QString(" %1").arg(exp_summary()));

    if(rusted){
        out.append(QString("<b> %1</b>").arg(rust_rating()));
        if(use_color)
            out.append("</font>");
    }

    return out;
}

bool Skill::operator<(const Skill *s2) const {
    return m_capped_level < s2->m_capped_level;
}

QString Skill::exp_summary() const {
    if (m_capped_level >= 20) {
        return QString("%L1xp").arg(m_actual_exp);
    }

    return QString("%L1/%L2xp (%L3%)")
            .arg(m_actual_exp)
            .arg(m_exp_for_next_level)
            .arg(m_exp_progress, 0, 'f', 1);
}

//used to initialize the standard xp levels
QHash<int,int> Skill::load_base_xp_levels(){
    QHash<int,int> exp;
    for(int level = 0; level <= 20; level++){
        exp.insert(level, xp_for_level(level));
    }
    return exp;
}

int Skill::xp_for_level(int level){
    if(level < 0)
        return 0;
    else
        return ((50 * level) * (level + 9));

}

float Skill::level_from_xp(int xp){
    return (xp / (225.0f + (5.0f*sqrt(2025.0f + (2.0f*xp)))));
}

int Skill::get_xp_for_level(int level){
    if(!m_experience_levels.contains(level))
        m_experience_levels.insert(level, xp_for_level(level));

    return m_experience_levels.value(level);
}

float Skill::capped_level_precise() const{
    if(m_capped_level >= 20){
        return (float)m_capped_level;
    }else{
        return (float)m_capped_level + (float)(m_exp_progress / 100.0f);
    }
}

float Skill::raw_level_precise() const{
    return (float)m_raw_level + (float)(m_exp_progress / 100.0f);

}

//simulates xp gain based on skill rate to calculate a rating between 0 and 1, courtesy of Maklak
double Skill::get_simulated_rating(){
    int curr_xp = m_capped_exp;
    int curr_level = m_capped_level;
    int rate = m_skill_rate;

    if (curr_xp >= MAX_CAPPED_XP)
        return 1.0;

    if (rate == 0)
        return curr_level / 20.0;

    double rating = get_simulated_level() / 20.0f;
    return rating;

}

double Skill::get_simulated_level(){
    int curr_xp = m_capped_exp;
    int curr_level = m_capped_level;
    int rate = m_skill_rate;

    int sim_xp = MAX_CAPPED_XP;
    sim_xp = (sim_xp / 100.0) * rate; //This is how much XP will go towards skill learning.
    double sim_level = 0.0;
    int xp_gap = 0;

    while ((sim_xp > 0) && (curr_level < 20))
    {
        xp_gap = get_xp_for_level(curr_level+1) - curr_xp;//xp to next level
        if (xp_gap > sim_xp)
            xp_gap = sim_xp;
        sim_xp -= xp_gap;

        sim_level += xp_gap * curr_level;

        curr_level++;
        curr_xp = get_xp_for_level(curr_level);
    }

    if (sim_xp > 0)
        sim_level += 20 * sim_xp;
    sim_level /= MAX_CAPPED_XP;
    return sim_level;
}

//returns a weighted average of the current level and the simulated level with skill rate
double Skill::get_balanced_level(){
    if(m_balanced_level < 0){
        float curr_level = capped_level_precise();
        if(curr_level < 0)
            curr_level = 0;
        float skill_rate_weight = DwarfStats::get_skill_rate_weight();
        if(!DT->show_skill_learn_rates){
            skill_rate_weight = 0;
        }
        double simulated_level = get_simulated_level();
        m_balanced_level = (curr_level * (1.0f-skill_rate_weight)) + (simulated_level * skill_rate_weight);
        if(m_balanced_level < 0)
            m_balanced_level = 0;
    }
    return m_balanced_level;
}

double Skill::get_rating(bool ensure_non_zero){
    if(m_rating < 0){
        m_rating = DwarfStats::get_skill_ecdf(get_balanced_level());
        if(m_rating < 0)
            m_rating = 0;
    }
    //this is just for optimization to ensure that this rating will match the lowest possible role rating (0.0001) for comparison
    if(ensure_non_zero && m_rating == 0)
        return 0.0001;
    else
        return m_rating;
}
