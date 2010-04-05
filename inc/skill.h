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

#include <QString>
#include <QStringList>
#include "gamedatareader.h"

class Skill
{
public:
    Skill();
    Skill(short id, uint exp, short rating);

    short id() const {return m_id;}
    short rating() const {return m_rating;}
    uint exp() const {return m_exp;}
    uint actual_exp() const {return m_actual_exp;}
    uint exp_for_current_level() const {return m_exp_for_current_level;}
    uint exp_for_next_level() const {return m_exp_for_next_level;}
    QString exp_summary() const;

    QString to_string(bool include_level = true, bool include_exp_summary = true) const;
    QString name() {return QString("(%1) %2").arg(m_id).arg(m_name);}
    bool operator<(const Skill &s2) const;

private:
    short m_id;
    uint m_exp;
    uint m_actual_exp;
    uint m_exp_for_current_level;
    uint m_exp_for_next_level;
    short m_rating;
    QString m_name;
};

#endif // SKILL_H
