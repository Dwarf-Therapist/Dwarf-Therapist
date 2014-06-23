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
#include "defines.h"
#include "qcolor.h"
#include "dwarftherapist.h"
#include "dwarfstats.h"

class GameDataReader;

class Skill
{

public:
    Skill();
    Skill(short id, uint exp, short rating, int rust, int skill_rate = 100);    

    short id() const {return m_id;}
    short capped_level() const {return m_capped_level;}
    float capped_level_precise() const;
    short raw_level() const {return m_raw_level;}
    float raw_level_precise() const;
    uint exp() const {return m_exp;}
    uint actual_exp() const {return m_actual_exp;}
    uint capped_exp() const {return m_capped_exp;}
    uint exp_for_current_level() const {return m_exp_for_current_level;}
    uint exp_for_next_level() const {return m_exp_for_next_level;}
    bool is_losing_xp() const {return m_losing_xp;}
    QString exp_summary() const;
    QString rust_rating() const {return m_rust_rating;}
    //QString rust_color() const {return m_rust_color;}
    QColor rust_color() const {return m_rust_color;}
    int skill_rate() const {return m_skill_rate;}    

    QString to_string(bool include_level = true, bool include_exp_summary = true, bool use_color = true) const;
    QString name() {return m_name;}
    bool operator<(const Skill *s2) const;

    struct less_than_key
    {
        bool operator() (Skill const& s1, Skill const& s2)
        {
            return s1.raw_level() < s2.raw_level();
        }
    };

    static int get_xp_for_level(int level);
    double get_simulated_rating();
    double get_simulated_level();
    double get_role_rating();

private:
    short m_id;
    uint m_exp;
    uint m_actual_exp;
    uint m_capped_exp;
    uint m_exp_for_current_level;
    uint m_exp_for_next_level;
    float m_exp_progress;
    short m_capped_level;
    short m_raw_level;
    QString m_name;
    int m_demotions;    
    QColor m_rust_color;
    QString m_rust_rating;
    int m_skill_rate;
    int m_rust;
    bool m_losing_xp;
    double m_role_rating;
    //skill level, experience
    static QHash<int,int> m_experience_levels;

    static QHash<int,int> load_base_xp_levels();
    static int xp_for_level(int level);
    static float level_from_xp(int xp);
};

#endif // SKILL_H
