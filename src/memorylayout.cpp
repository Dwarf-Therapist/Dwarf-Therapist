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
	m_data->beginGroup("addresses");
	foreach(QString k, m_data->childKeys()) {
		m_addresses.insert(k, read_hex(k));
	}
	m_data->endGroup();

	// offsets
	m_data->beginGroup("offsets");
	foreach(QString k, m_data->childKeys()) {
		m_offsets.insert(k, read_hex(k));
	}
	m_data->endGroup();
		
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
