#ifndef SKILL_H
#define SKILL_H

#include <QString>
#include <QStringList>

class Skill
{
public:
	Skill()
		: m_id(-1)
		, m_exp(-1)
		, m_rating(-1)
	{
	}
	 
    Skill(short id, short exp, short rating)
        : m_id(id)
        , m_exp(exp)
        , m_rating(rating)
    {
    }

    QString rating_name(short rating) {
		QString name_str = "Dabbling,Novice,,Competent,Skilled,Proficient,Talented,Adept,Expert,Professional,Accomplished,Great,Master,High Master,Grand Master,Legendary";
		QStringList names = name_str.split(",");
        if (rating < names.length()) {
            return names[rating];
        }
        return "Legendary";
    }

    QString to_string() {
        return QString("%1 SKILL%2 (%3)").arg(rating_name(m_rating)).arg(m_id).arg(m_exp);
    }
private:
    short m_id;
    short m_exp;
    short m_rating;
};



#endif // SKILL_H
