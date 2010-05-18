#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

#include <QtCore>

class MemoryLayout {
public:
    explicit MemoryLayout(const QString &filename);

    QString filename() {return m_filename;}
    bool is_valid();
    QString game_version() {return m_game_version;}
    QString checksum() {return m_checksum;}
    uint address(QString key) {return m_addresses.value(key, -1);}
    uint offset(QString key) {return m_offsets.value(key, -1);}
    uint dwarf_offset(QString key) {return m_dwarf_offsets.value(key, -1);}
    QHash<uint, QString> valid_flags_1() {return m_valid_flags_1;}
    QHash<uint, QString> invalid_flags_1() {return m_invalid_flags_1;}
    QHash<uint, QString> invalid_flags_2() {return m_invalid_flags_2;}
    bool is_complete() {return m_complete;}

private:
    QString m_filename;
    QString m_checksum;
    QString m_game_version;
    QHash<QString, uint> m_addresses;
    QHash<QString, uint> m_offsets;
    QHash<QString, uint> m_dwarf_offsets;
    QHash<uint, QString> m_valid_flags_1;
    QHash<uint, QString> m_invalid_flags_1;
    QHash<uint, QString> m_invalid_flags_2;
    QSettings *m_data;
    bool m_complete;

    void load_data();
    uint read_hex(QString key);
};

#endif
