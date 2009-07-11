#ifndef DWARF_H
#define DWARF_H

#include <QObject>
#include <QVector>
#include "skill.h"

class DFInstance;

class Dwarf : public QObject
{
	Q_OBJECT
	Dwarf(DFInstance *df, int address, QObject *parent=0); //private, use the static get_dwarf() method

public:
	static Dwarf* get_dwarf(DFInstance *df, int address);
	~Dwarf();

	QString nice_name();
	QString to_string();
	QVector<Skill> *get_skills() {return &m_skills;}

private:
	DFInstance *m_df;
	int m_address;
	int m_race_id;
    QString read_last_name(int address);
    QVector<Skill> read_skills(int address);

	int m_id;
	QString m_first_name;
	QString m_last_name;
	QString m_nick_name;
	QString m_custom_profession;
    QVector<Skill> m_skills;
	
};

#endif // DWARF_H
