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
	Dwarf(DFInstance *df, int address, QObject *parent=0); //private, use the static get_dwarf() method

public:
	static Dwarf* get_dwarf(DFInstance *df, int address);
	~Dwarf();

	// attributes
	int id() {return m_id;}
	QString profession() {return m_profession;}
	QString custom_profession_name() {return m_pending_custom_profession;}
	void refresh_data();
	QString nice_name();
	

	QVector<Skill> *get_skills() {return &m_skills;}
	QVector<int> get_dirty_labors(); // returns labor ids
	bool is_labor_enabled(int labor_id) {return (char)m_pending_labors[labor_id] > 0;}
	bool is_labor_state_dirty(int labor_id) {return (char)m_labors[labor_id] != (char)m_pending_labors[labor_id];}
	void set_labor(int labor_id, bool enabled);
	bool toggle_labor(int labor_id);
	short get_rating_for_skill(int labor_id);
	int pending_changes();
	void clear_pending();
	void commit_pending();
	int apply_custom_profession(CustomProfession *cp); // return # of pending changes
	QString nickname() {return m_pending_nick_name;}
	void set_nickname(QString nick) {m_pending_nick_name = nick;}

	QTreeWidgetItem *get_pending_changes_tree();

	QModelIndex m_name_idx;

private:
	DFInstance *m_df;
	int m_address;
	int m_race_id;
	QString read_professtion(int address);
    QString read_last_name(int address);
    QVector<Skill> read_skills(int address);
	void read_labors(int address);

	int m_id;
	QString m_first_name;
	QString m_last_name;
	QString m_nick_name, m_pending_nick_name;
	QString m_custom_profession, m_pending_custom_profession;
	QString m_profession;
	int m_strength;
	int m_agility;
	int m_toughness;
    QVector<Skill> m_skills;
	uchar *m_labors;
	uchar *m_pending_labors;
	
};

#endif // DWARF_H
