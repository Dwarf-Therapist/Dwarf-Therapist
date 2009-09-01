#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

#include <QtCore>

class MemoryLayout {
public:
    MemoryLayout(uint checksum);
	
	bool is_valid() {return m_data != 0;}
	QString game_version() {return m_game_version;}
    uint address(QString key) {return m_addresses.value(key, -1);}
    uint offset(QString key) {return m_offsets.value(key, -1);}
    uint dwarf_offset(QString key) {return m_dwarf_offsets.value(key, -1);}
	int flags(QString key) {return m_flags.value(key, -1);}

private:
    uint m_checksum;
    QMap<QString, uint> m_addresses;
    QMap<QString, uint> m_offsets;
    QMap<QString, uint> m_dwarf_offsets;
    QMap<QString, uint> m_flags;
	QSettings *m_data;

	void load_data();
    uint read_hex(QString key);
	QString m_game_version;
};

#endif
