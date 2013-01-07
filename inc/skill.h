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
#ifndef SKILL_H
#define SKILL_H

#include "qstring.h"
#include "qstringlist.h"

class GameDataReader;

class Skill
{
public:
    Skill();
    Skill(short id, uint exp, short rating, int demotions, int skill_rate = 100);

    short id() const {return m_id;}
    short rating() const {return m_rating;}
    uint exp() const {return m_exp;}
    uint actual_exp() const {return m_actual_exp;}
    uint exp_for_current_level() const {return m_exp_for_current_level;}
    uint exp_for_next_level() const {return m_exp_for_next_level;}
    QString exp_summary() const;
    QString rust_rating() const {return m_rust_rating;}
    QString skill_color() const {return m_skill_color;}
    int skill_rate() const {return m_skill_rate;}

    QString to_string(bool include_level = true, bool include_exp_summary = true) const;
    //QString name() {return QString("(%1) %2").arg(m_id).arg(m_name);}
    QString name() {return m_name;}
    bool operator<(const Skill *s2) const;

    struct less_than_key
    {
        bool operator() (Skill* const& s1, Skill* const& s2)
        {
            return s1->rating() < s2->rating();
        }
    };

private:
    short m_id;
    uint m_exp;
    uint m_actual_exp;
    uint m_exp_for_current_level;
    uint m_exp_for_next_level;
    float m_exp_progress;
    short m_rating;
    QString m_name;
    int m_demotions;
    QString m_skill_color;
    QString m_rust_rating;    
    int m_skill_rate;
};

#endif // SKILL_H
