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
    uint address(const QString &key) {return m_addresses.value(key, -1);}
    uint offset(const QString &key) {return m_offsets.value(key, -1);}
    uint dwarf_offset(const QString &key) {
        return m_dwarf_offsets.value(key, -1);
    }
    uint job_flag(const QString &key) {return m_job_flags.value(key, -1);}
    QHash<uint, QString> valid_flags_1() {return m_valid_flags_1;}
    QHash<uint, QString> invalid_flags_1() {return m_invalid_flags_1;}
    QHash<uint, QString> invalid_flags_2() {return m_invalid_flags_2;}

    bool is_complete() {return m_complete;}

private:
    typedef QHash<QString, uint> AddressHash;

    QString m_filename;
    QString m_checksum;
    QString m_game_version;
    AddressHash m_addresses;
    AddressHash m_offsets;
    AddressHash m_dwarf_offsets;
    AddressHash m_job_flags;
    QHash<uint, QString> m_valid_flags_1;
    QHash<uint, QString> m_invalid_flags_1;
    QHash<uint, QString> m_invalid_flags_2;
    QSettings *m_data;
    bool m_complete;

    void load_data();
    uint read_hex(QString key);
    void read_group(const QString &group, AddressHash &map);
};

#endif
