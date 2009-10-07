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
		DH_MISERABLE = 0,
		DH_VERY_UNHAPPY,
		DH_UNHAPPY,
		DH_FINE,
		DH_CONTENT,
		DH_HAPPY,
		DH_ECSTATIC,
		DH_TOTAL_LEVELS
	} DWARF_HAPPINESS;

	// attributes
    uint address() {return m_address;}
	int id() {return m_id;}
	bool is_male() {return m_is_male;}
	QString profession();
	int raw_profession() {return m_raw_profession;}
	QString custom_profession_name() {return m_pending_custom_profession;}
	void refresh_data();
	QString nice_name() {return m_nice_name;}
	QString translated_name() {return m_translated_name;}
	QString nickname() {return m_pending_nick_name;}
	void set_nickname(QString nick) {m_pending_nick_name = nick; calc_names();}
	DWARF_HAPPINESS get_happiness() {return m_happiness;}
	int get_raw_happiness() {return m_raw_happiness;}
	int strength() {return m_strength;}
	int agility() {return m_agility;}
	int toughness() {return m_toughness;}
	int total_xp() {return m_total_xp;}
	int migration_wave() {return m_migration_wave;}
	void set_migration_wave(const int &wave_number) {m_migration_wave = wave_number;}
	
	QVector<Skill> *get_skills() {return &m_skills;}
	const Skill get_skill(int skill_id);
	QVector<int> get_dirty_labors(); // returns labor ids
	bool is_labor_enabled(int labor_id);
	bool is_labor_state_dirty(int labor_id);
	void set_labor(int labor_id, bool enabled);
	bool toggle_labor(int labor_id);
	short get_rating_by_skill(int skill_id);
	short get_rating_by_labor(int labor_id);
	short trait(int trait_id) {return m_traits.value(trait_id, -1);}
    const QHash<int, short> &traits() {return m_traits;}
    const QString &current_job() {return m_current_job;}
    const short &current_job_id() {return m_current_job_id;}

	int pending_changes();
	void clear_pending();
	void commit_pending();
	int apply_custom_profession(CustomProfession *cp); // return # of pending changes
	void reset_custom_profession() {m_pending_custom_profession = "";}
	QString tooltip_text();

	short get_num_weapons();
	
	static DWARF_HAPPINESS happiness_from_score(int score);
	static QString happiness_name(DWARF_HAPPINESS happiness);

	QTreeWidgetItem *get_pending_changes_tree();
	QModelIndex m_name_idx;

	QList<QAction*> get_actions() {return m_actions;}

	public slots:
		void read_settings();
		//! show a dialog with a memory dump for this dwarf...
		void dump_memory();
        //! show details for this dwarf in a new window...
        void show_details();

private:
	DFInstance *m_df;
    uint m_address;
	int m_race_id;
	DWARF_HAPPINESS m_happiness;
	int m_raw_happiness;
	int m_money;
	bool m_is_male;
	bool m_show_full_name;
	int m_total_xp;
	int m_migration_wave;
	
    QString read_profession(const uint &addr);
    QString read_last_name(const uint &addr, bool use_generic=false);
    QVector<Skill> read_skills(const uint &addr);
    void read_prefs(const uint &addr);
    void read_labors(const uint &addr);
	void calc_names();
    void read_traits(const uint &addr);
    void read_current_job(const uint &addr);

	int m_id;
	QString m_first_name;
	QString m_last_name, m_translated_last_name;
	QString m_nick_name, m_pending_nick_name;
	QString m_nice_name, m_translated_name; // used to cache this value
	QString m_custom_profession, m_pending_custom_profession;
	QString m_profession;
	int m_raw_profession;
	bool m_can_set_labors;
	int m_strength;
	int m_agility;
	int m_toughness;
	short m_num_weapons;
	short m_pending_num_weapons;
    short m_current_job_id;
    QString m_current_job;
	QVector<Skill> m_skills;
	QHash<int, short> m_traits;
	QMap<int, bool> m_labors;
	QMap<int, bool> m_pending_labors;
	QList<QAction*> m_actions; // actions suitable for context menus

signals:
	void name_changed();
};

#endif // DWARF_H
