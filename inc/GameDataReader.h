#ifndef GAME_DATA_READER_H
#define GAME_DATA_READER_H

#include <string>
#include <stdexcept>

// forward declaration
class QSettings;

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
class GameDataReader {
	
public:
	static GameDataReader *ptr() {
		if (!m_instance) {
			m_instance = new GameDataReader;
		}
		return m_instance;
	}

	int get_int_for_key(QString key, short base = 16);
	int get_address(QString key) {return get_int_for_key("addresses/" + key);}
	int get_offset(QString key) {return get_int_for_key("offsets/" + key);}
	int get_dwarf_offset(QString key) {return get_int_for_key("dwarf_offsets/" + key);}
	
	QString get_string_for_key(QString key);
	QString get_profession_name(int profession_id);
	QString get_skill_level_name(short level);
	QString get_skill_name(short skill_id);
	
	QStringList get_child_groups(QString section);
	QStringList get_keys(QString section);

protected:
	GameDataReader();
private:
	static GameDataReader *m_instance;
	QSettings *m_data_settings;


};
#endif