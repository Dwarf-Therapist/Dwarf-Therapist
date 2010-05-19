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

Skill::Skill()
    : m_id(-1)
    , m_exp(0)
    , m_actual_exp(0)
    , m_exp_for_current_level(0)
    , m_exp_for_next_level(1)
    , m_rating(-1)
    , m_name("UNKNOWN")
{}

Skill::Skill(short id, uint exp, short rating)
    : m_id(id)
    , m_exp(exp)
    , m_actual_exp(exp)
    , m_exp_for_current_level(0)
    , m_exp_for_next_level(exp + 1)
    , m_rating(rating > 20 ? 20 : rating)
    , m_name("UNKNOWN")
{
    // formula from http://df.magmawiki.com/index.php/40d:Attribute
    m_actual_exp = m_exp;
    for (int i = 0; i < m_rating; ++i) {
        m_actual_exp += 500 + (i * 100);
    }
    m_exp_for_current_level = 0;
    for (int i = 0; i < m_rating; ++i) {
        m_exp_for_current_level += 500 + (i * 100);
    }
    m_exp_for_next_level = 0;
    for (int i = 0; i < m_rating + 1; ++i) {
        m_exp_for_next_level += 500 + (i * 100);
    }
    m_name = GameDataReader::ptr()->get_skill_name(m_id);
}

QString Skill::to_string(bool include_level, bool include_exp_summary) const {
    GameDataReader *gdr = GameDataReader::ptr();
    QString out;
    if (include_level)
        out.append(QString("[%1] ").arg(m_rating));
    QString skill_level = gdr->get_skill_level_name(m_rating);
    QString skill_name = gdr->get_skill_name(m_id);
    if (skill_level.isEmpty())
        out.append(QString("<b>%1</b>").arg(skill_name));
    else
        out.append(QString("<b>%1 %2</b>").arg(skill_level, skill_name));
    if (include_exp_summary)
        out.append(QString(" %1").arg(exp_summary()));
    return out;
}

bool Skill::operator<(const Skill &s2) const {
    return m_rating < s2.m_rating;
}

QString Skill::exp_summary() const {
    if (m_rating >= 20) {
        return QString("TOTAL: %L1xp").arg(m_actual_exp);
    }
    float progress = 0.0f;
    if (m_exp_for_next_level && m_exp_for_current_level) {
        progress = ((float)m_exp / (float)(m_exp_for_next_level - m_exp_for_current_level)) * 100;
    }
    return QString("%L1xp / %L2xp (%L3%)")
        .arg(m_actual_exp)
        .arg(m_exp_for_next_level)
        .arg(progress, 0, 'f', 1);
}
