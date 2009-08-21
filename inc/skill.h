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
	Skill()
		: m_id(-1)
		, m_exp(0)
		, m_rating(-1)
	{
	}
	 
	Skill(short id, uint exp, short rating)
		: m_id(id)
		, m_exp(exp)
		, m_rating(rating > 15 ? 15 : rating)
	{
	}

	short id() const {return m_id;}
	short rating() const {return m_rating;}
	uint exp() const {return m_exp;}

	QString to_string() const {
		GameDataReader *gdr = GameDataReader::ptr();
		QString out;
		QString level = gdr->get_skill_level_name(m_rating);
		
		if (!level.isEmpty()) {
			out += level;
			out += " ";
		}
		out += gdr->get_skill_name(m_id);
		return out;// + QString("(%3exp)").arg(m_exp);TODO: fix the exp reading
	}

	bool operator<(const Skill &s2) const {
		return m_rating < s2.rating();
	}

private:
	short m_id;
	uint m_exp;
	short m_rating;
};

#endif // SKILL_H
