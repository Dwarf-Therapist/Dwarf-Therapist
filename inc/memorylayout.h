#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

#include <QtCore>

class MemoryLayout {
public:
	MemoryLayout(int checksum);
	
	bool is_valid() {return m_data != 0;}
	QString game_version() {return m_game_version;}
	int address(QString key) {return m_addresses.value(key, -1);}
	int offset(QString key) {return m_offsets.value(key, -1);}
	int dwarf_offset(QString key) {return m_dwarf_offsets.value(key, -1);}
	int flags(QString key) {return m_flags.value(key, -1);}

private:
	int m_checksum;
	QMap<QString, int> m_addresses;
	QMap<QString, int> m_offsets;
	QMap<QString, int> m_dwarf_offsets;
	QMap<QString, int> m_flags;
	QSettings *m_data;

	void load_data();
	int read_hex(QString key);
	QString m_game_version;
};

#endif