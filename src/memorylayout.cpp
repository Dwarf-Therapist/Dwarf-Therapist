#include <QtCore>
#include "memorylayout.h"
#include "gamedatareader.h"

MemoryLayout::MemoryLayout(uint checksum)
	: m_checksum(checksum)
	, m_data(0)
{
	m_game_version = GameDataReader::ptr()->get_string_for_key("checksum_to_version/0x" + QString::number(checksum, 16));
	if (m_game_version != "UNKNOWN") {
		QDir working_dir = QDir::current();
#ifdef Q_WS_WIN
        QString subdir = "windows";
#endif
#ifdef Q_WS_X11
        QString subdir = "linux";
#endif
        QString filename = working_dir.absoluteFilePath("etc/memory_layouts/" + subdir + "/" + m_game_version + ".ini");
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
	m_data->beginGroup("dwarf_offsets");
	foreach(QString k, m_data->childKeys()) {
		m_dwarf_offsets.insert(k, read_hex(k));
	}
	m_data->endGroup();
	
	// flags
	m_flags.insert("flags1.invalidate", read_hex("flags/flags1.invalidate"));
	m_flags.insert("flags2.invalidate", read_hex("flags/flags2.invalidate"));
}

uint MemoryLayout::read_hex(QString key) {
	bool ok;
	QString data = m_data->value(key, -1).toString();
    uint val = data.toUInt(&ok, 16);
	return val;
}
