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
    bool is_valid_address(uint address);
    uint offset(const QString &key) {return m_offsets.value(key, -1);}
    uint string_buffer_offset();
    uint string_length_offset();
    uint string_cap_offset();
    int dwarf_offset(const QString &key) {
        return m_dwarf_offsets.value(key, -1);
    }
    int squad_offset(const QString & key) {
        return m_squad_offsets.value(key, -1);
    }
    int word_offset(const QString & key) {
        return m_word_offsets.value(key, -1);
    }
    int race_offset(const QString & key) {
        return m_race_offsets.value(key, -1);
    }
    int caste_offset(const QString & key) {
        return m_caste_offsets.value(key, -1);
    }
    int hist_figure_offset(const QString & key){
        return m_hist_fig_offsets.value(key, -1);
    }
    int hist_entity_offset(const QString & key){
        return m_hist_entity_offsets.value(key, -1);
    }
    int weapon_subtype_offset(const QString & key){
        return m_weapon_subtype_offsets.value(key, -1);
    }
    int material_offset(const QString & key){
        return m_material_offsets.value(key, -1);
    }
    int plant_offset(const QString & key){
        return m_plant_offsets.value(key, -1);
    }
    int item_subtype_offset(const QString & key){
        return m_item_subtype_offsets.value(key, -1);
    }
    int descriptor_offset(const QString & key){
        return m_descriptor_offsets.value(key, -1);
    }
    int health_offset(const QString & key){
        return m_health_offsets.value(key, -1);
    }
    int wound_offset(const QString & key){
        return m_unit_wound_offsets.value(key,-1);
    }
    int item_offset(const QString & key){
        return m_item_offsets.value(key,-1);
    }
    int item_filter_offset(const QString & key){
        return m_item_filter_offsets.value(key,-1);
    }
    int armor_subtype_offset(const QString & key){
        return m_armor_subtype_offsets.value(key,-1);
    }
    int general_ref_offset(const QString & key){
        return m_general_ref_offsets.value(key,-1);
    }
    int syndrome_offset(const QString & key){
        return m_syndrome_offsets.value(key,-1);
    }


    QSettings * data() { return m_data; }
    uint job_detail(const QString &key) {return m_job_details.value(key, -1);}
    uint soul_detail(const QString &key) {return m_soul_details.value(key, -1);}
    QHash<uint, QString> valid_flags_1() {return m_valid_flags_1;}
    QHash<uint, QString> valid_flags_2() {return m_valid_flags_2;}
    QHash<uint, QString> invalid_flags_1() {return m_invalid_flags_1;}
    QHash<uint, QString> invalid_flags_2() {return m_invalid_flags_2;}
    QHash<uint, QString> invalid_flags_3() {return m_invalid_flags_3;}

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
    AddressHash m_race_offsets;
    AddressHash m_caste_offsets;
    AddressHash m_hist_fig_offsets;
    AddressHash m_hist_entity_offsets;
    AddressHash m_weapon_subtype_offsets;
    AddressHash m_plant_offsets;
    AddressHash m_item_subtype_offsets;
    AddressHash m_material_offsets;
    AddressHash m_descriptor_offsets;
    AddressHash m_health_offsets;
    AddressHash m_unit_wound_offsets;
    AddressHash m_armor_subtype_offsets;
    AddressHash m_item_offsets;
    AddressHash m_item_filter_offsets;
    AddressHash m_general_ref_offsets;
    AddressHash m_syndrome_offsets;

    QHash<uint, QString> m_valid_flags_1;
    QHash<uint, QString> m_valid_flags_2;
    QHash<uint, QString> m_invalid_flags_1;
    QHash<uint, QString> m_invalid_flags_2;
    QHash<uint, QString> m_invalid_flags_3;
    QSettings *m_data;
    bool m_complete;

    void load_data();
    uint read_hex(QString key);
    void read_group(const QString &group, AddressHash &map);
};
Q_DECLARE_METATYPE(MemoryLayout *)
#endif
