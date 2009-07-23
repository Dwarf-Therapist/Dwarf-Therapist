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
#ifndef GAME_DATA_READER_H
#define GAME_DATA_READER_H

#include <string>
#include <stdexcept>
#include <QtCore>

// forward declaration
class QSettings;
class Labor;

// exceptions
class MissingValueException : public std::runtime_error {
public:
	MissingValueException(const std::string &msg) : runtime_error(msg) {}
};

class CorruptedValueException : public std::runtime_error {
public:
	CorruptedValueException(const std::string &msg) : runtime_error(msg) {}
};

//singleton reader of game data
class GameDataReader : public QObject {
	Q_OBJECT
public:
	static GameDataReader *ptr() {
		if (!m_instance) {
			m_instance = new GameDataReader(0);
		}
		return m_instance;
	}

	void set_game_checksum(int checksum) {m_game_checksum = checksum;}

	int get_int_for_key(QString key, short base = 16);
	int get_address(QString key) {return get_int_for_key("addresses/" + key);}
	int get_offset(QString key) {return get_int_for_key("offsets/" + key);}
	int get_dwarf_offset(QString key) {return get_int_for_key("dwarf_offsets/" + key);}

	QMap<int, Labor*> get_ordered_labors() {return m_ordered_labors;}
	Labor *get_labor(int labor_id);
	
	QString get_string_for_key(QString key);
	QString get_profession_name(int profession_id);
	QString get_skill_level_name(short level);
	QString get_skill_name(short skill_id);
	QColor get_color(QString key);
	
	QStringList get_child_groups(QString section);
	QStringList get_keys(QString section);

protected:
	GameDataReader(QObject *parent = 0);
private:
	static GameDataReader *m_instance;
	QSettings *m_data_settings;
	QMap<int, Labor*> m_labors;
	QMap<int, Labor*> m_ordered_labors;
	int m_game_checksum;


};
#endif