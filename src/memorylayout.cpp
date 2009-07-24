#include <QtCore>
#include "memorylayout.h"
#include "GameDataReader.h"

MemoryLayout::MemoryLayout(int checksum)
	: m_checksum(checksum)
	, m_data(0)
{
	m_game_version = GameDataReader::ptr()->get_string_for_key("checksum_to_version/0x" + QString::number(checksum, 16));
	if (m_game_version != "UNKNOWN") {
		QDir working_dir = QDir::current();
		QString filename = working_dir.absoluteFilePath("etc/" + m_game_version + ".ini");
		m_data = new QSettings(filename, QSettings::IniFormat);
		load_data();
	}
}

void MemoryLayout::load_data() {
	// addresses
	m_addresses.insert("language_vector", read_hex("addresses/language_vector"));
	m_addresses.insert("translation_vector", read_hex("addresses/translation_vector"));
	m_addresses.insert("creature_vector", read_hex("addresses/creature_vector"));
	m_addresses.insert("dwarf_race_index", read_hex("addresses/dwarf_race_index"));

	// offsets
	m_offsets.insert("word_table", read_hex("offsets/word_table"));
		
	// dwarf offsets
	m_dwarf_offsets.insert("first_name", read_hex("dwarf_offsets/first_name"));
	m_dwarf_offsets.insert("nick_name", read_hex("dwarf_offsets/nick_name"));
	m_dwarf_offsets.insert("last_name", read_hex("dwarf_offsets/last_name"));
	m_dwarf_offsets.insert("custom_profession", read_hex("dwarf_offsets/custom_profession"));
	m_dwarf_offsets.insert("profession", read_hex("dwarf_offsets/profession"));
	m_dwarf_offsets.insert("race", read_hex("dwarf_offsets/race"));
	m_dwarf_offsets.insert("flags1", read_hex("dwarf_offsets/flags1"));
	m_dwarf_offsets.insert("flags2", read_hex("dwarf_offsets/flags2"));
	m_dwarf_offsets.insert("id", read_hex("dwarf_offsets/id"));
	m_dwarf_offsets.insert("strength", read_hex("dwarf_offsets/strength"));
	m_dwarf_offsets.insert("agility", read_hex("dwarf_offsets/agility"));
	m_dwarf_offsets.insert("toughness", read_hex("dwarf_offsets/toughness"));
	m_dwarf_offsets.insert("skills", read_hex("dwarf_offsets/skills"));
	m_dwarf_offsets.insert("labors", read_hex("dwarf_offsets/labors"));

	// flags
	m_flags.insert("flags1.invalidate", read_hex("flags/flags1.invalidate"));
	m_flags.insert("flags2.invalidate", read_hex("flags/flags2.invalidate"));
}

int MemoryLayout::read_hex(QString key) {
	bool ok;
	QString data = m_data->value(key, -1).toString();
	int val = data.toInt(&ok, 16);
	return val;
}