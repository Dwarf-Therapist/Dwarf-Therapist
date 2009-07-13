#ifndef SKILL_H
#define SKILL_H

#include <QString>
#include <QStringList>
#include "GameDataReader.h"

class Skill
{
public:
	Skill()
		: m_id(-1)
		, m_exp(-1)
		, m_rating(-1)
	{
	}
	 
    Skill(short id, ushort exp, short rating)
        : m_id(id)
        , m_exp(exp)
		, m_rating(rating > 15 ? 15 : rating)
    {
    }

	short id() {return m_id;}
	short rating() {return m_rating;}

    QString to_string() {
		GameDataReader *gdr = GameDataReader::ptr();
		QString out;
		QString level = gdr->get_skill_level_name(m_rating);
		
		if (!level.isEmpty()) {
			out += level;
			out += " ";
		}
		out += gdr->get_skill_name(m_id);
        return out + QString("(%3exp)").arg(m_exp);
    }
private:
    short m_id;
    ushort m_exp;
    short m_rating;
};



#endif // SKILL_H
