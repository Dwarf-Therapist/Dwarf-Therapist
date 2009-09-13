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

Skill::Skill()
	: m_id(-1)
	, m_exp(0)
	, m_rating(-1)
	, m_actual_exp(0)
	, m_exp_for_next_level(1)
{}

Skill::Skill(short id, uint exp, short rating)
	: m_id(id)
	, m_exp(exp)
	, m_rating(rating > 20 ? 20 : rating)
	, m_actual_exp(exp)
	, m_exp_for_next_level(exp + 1)
{
	// formula from http://dwarf.lendemaindeveille.com/index.php/Experience
	m_actual_exp = m_exp;
	for (int i = 0; i < m_rating; ++i) {
		m_actual_exp += 500 + (i * 100);
	}
	m_exp_for_next_level = 0;
	for (int i = 0; i < m_rating + 1; ++i) {
		m_exp_for_next_level += 500 + (i * 100);
	}
}

QString Skill::to_string() const {
	GameDataReader *gdr = GameDataReader::ptr();
	QString out = QString("[%1] ").arg(m_rating);
	QString skill_level = gdr->get_skill_level_name(m_rating);
	QString skill_name = gdr->get_skill_name(m_id);
	if (skill_level.isEmpty())
		out.append(skill_name);
	else
		out.append(QString("%1 %2").arg(skill_level, skill_name));
	out.append(QString(" (%1xp)").arg(m_exp));
	return out;
}

bool Skill::operator<(const Skill &s2) const {
	return m_rating < s2.rating();
}

QString Skill::exp_summary() const {
	float progress = 0.0f;
	if (m_exp_for_next_level)
		progress = ((float)m_actual_exp / (float)m_exp_for_next_level) * 100;
	return QString("%1/%2 (%L3%)")
		.arg(m_actual_exp)
		.arg(m_exp_for_next_level)
		.arg(progress, 0, 'f', 1);
}
