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
#ifndef DWARF_H
#define DWARF_H

#include <QtGui>

#include "skill.h"

class DFInstance;
class CustomProfession;

class Dwarf : public QObject
{
	Q_OBJECT
    Dwarf(DFInstance *df, const uint &addr, QObject *parent=0); //private, use the static get_dwarf() method

public:
    static Dwarf* get_dwarf(DFInstance *df, const uint &address);
	~Dwarf();

	typedef enum {
		DH_MISERABLE,
		DH_UNHAPPY,
		DH_FINE,
		DH_CONTENT,
		DH_HAPPY,
		DH_ECSTATIC
	} DWARF_HAPPINESS;

	// attributes
    uint address() {return m_address;}
	int id() {return m_id;}
	bool is_male() {return m_is_male;}
	QString profession();
	QString custom_profession_name() {return m_pending_custom_profession;}
	void refresh_data();
	QString nice_name() {return m_nice_name;}
	QString nickname() {return m_pending_nick_name;}
	void set_nickname(QString nick) {m_pending_nick_name = nick; calc_nice_name();}
	DWARF_HAPPINESS get_happiness() {return m_happiness;}
	int get_raw_happiness() {return m_raw_happiness;}
	
	QVector<Skill> *get_skills() {return &m_skills;}
	const Skill get_skill(int skill_id);
	QVector<int> get_dirty_labors(); // returns labor ids
	bool is_labor_enabled(int labor_id);
	bool is_labor_state_dirty(int labor_id);
	void set_labor(int labor_id, bool enabled);
	bool toggle_labor(int labor_id);
	short get_rating_by_skill(int skill_id);
	short get_rating_by_labor(int labor_id);
	int pending_changes();
	void clear_pending();
	void commit_pending();
	int apply_custom_profession(CustomProfession *cp); // return # of pending changes
	void reset_custom_profession() {m_pending_custom_profession = "";}

	short get_num_weapons();
	
	static DWARF_HAPPINESS happiness_from_score(int score);
	static QString happiness_name(DWARF_HAPPINESS happiness);

	QTreeWidgetItem *get_pending_changes_tree();
	QModelIndex m_name_idx;

private:
	DFInstance *m_df;
    uint m_address;
	int m_race_id;
	DWARF_HAPPINESS m_happiness;
	int m_raw_happiness;
	int m_money;
	bool m_is_male;
	
    QString read_profession(const uint &addr);
    QString read_last_name(const uint &addr);
    QVector<Skill> read_skills(const uint &addr);
    void read_prefs(const uint &addr);
    void read_labors(const uint &addr);
	void calc_nice_name();

	int m_id;
	QString m_first_name;
	QString m_last_name;
	QString m_nick_name, m_pending_nick_name;
	QString m_nice_name; // used to cache this value
	QString m_custom_profession, m_pending_custom_profession;
	QString m_profession;
	int m_strength;
	int m_agility;
	int m_toughness;
	short m_num_weapons;
	short m_pending_num_weapons;
	QVector<Skill> m_skills;
	QMap<int, bool> m_labors;
	QMap<int, bool> m_pending_labors;
};

#endif // DWARF_H
