#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

#include <QtCore>

class MemoryLayout {
public:
    explicit MemoryLayout(const QString &filename);
    MemoryLayout(const QString & filename, QSettings * data);

    QString filename() {return m_filename;}
    bool is_valid();
    QString game_version() {return m_game_version;}
    QString checksum() {return m_checksum;}
    uint address(const QString &key) {return m_addresses.value(key, -1);}
    uint offset(const QString &key) {return m_offsets.value(key, -1);}
    uint string_buffer_offset();
    uint string_length_offset();
    uint string_cap_offset();
    uint dwarf_offset(const QString &key) {
        return m_dwarf_offsets.value(key, -1);
    }
    uint squad_offset(const QString & key) {
        return m_squad_offsets.value(key, -1);
    }
    uint word_offset(const QString & key) {
        return m_word_offsets.value(key, -1);
    }

    QSettings * data() { return m_data; }
    uint job_detail(const QString &key) {return m_job_details.value(key, -1);}
    uint soul_detail(const QString &key) {return m_soul_details.value(key, -1);}
    QHash<uint, QString> valid_flags_1() {return m_valid_flags_1;}
    QHash<uint, QString> valid_flags_2() {return m_valid_flags_2;}
    QHash<uint, QString> invalid_flags_1() {return m_invalid_flags_1;}
    QHash<uint, QString> invalid_flags_2() {return m_invalid_flags_2;}

    bool is_complete() {return m_complete;}

    //Setters
    void set_address(const QString & key, uint value);
    void set_game_version(const QString & value);
    void set_checksum(const QString & checksum);
    void save_data();
    void set_complete();

    bool operator<(const MemoryLayout & rhs) const {
        return m_game_version < rhs.m_game_version;
    }

private:
    typedef QHash<QString, uint> AddressHash;

    QString m_filename;
    QString m_checksum;
    QString m_game_version;
    AddressHash m_addresses;
    AddressHash m_offsets;
    AddressHash m_dwarf_offsets;
    AddressHash m_job_details;
    AddressHash m_soul_details;
    AddressHash m_squad_offsets;
    AddressHash m_word_offsets;
    QHash<uint, QString> m_valid_flags_1;
    QHash<uint, QString> m_valid_flags_2;
    QHash<uint, QString> m_invalid_flags_1;
    QHash<uint, QString> m_invalid_flags_2;
    QSettings *m_data;
    bool m_complete;

    void load_data();
    uint read_hex(QString key);
    void read_group(const QString &group, AddressHash &map);
};
Q_DECLARE_METATYPE(MemoryLayout *)
#endif
