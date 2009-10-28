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
#ifdef _WINDOWS
    QString subdir = "windows";
#endif
#ifdef _LINUX
    QString subdir = "linux";
#endif
#ifdef _OSX
	QString subdir = "osx";
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
	int flag_count = m_data->beginReadArray("invalid_flags_1");
	for (int i = 0; i < flag_count; ++i) {
		m_data->setArrayIndex(i);
		m_invalid_flags_1.insert(read_hex("value"),
			m_data->value("name", "UNKNOWN INVALID FLAG 1").toString());
	}
	m_data->endArray();
	
	flag_count = m_data->beginReadArray("invalid_flags_2");
	for (int i = 0; i < flag_count; ++i) {
		m_data->setArrayIndex(i);
		m_invalid_flags_2.insert(read_hex("value"),
			m_data->value("name", "UNKNOWN INVALID FLAG 2").toString());
	}
	m_data->endArray();
}

uint MemoryLayout::read_hex(QString key) {
	bool ok;
	QString data = m_data->value(key, -1).toString();
    uint val = data.toUInt(&ok, 16);
	return val;
}
