#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

#include "utils.h"
#include <QSettings>
#include <QFileInfo>

class DFInstance;

class MemoryLayout {
public:
    explicit MemoryLayout(DFInstance *df, const QFileInfo &fileinfo);
    MemoryLayout(DFInstance *df, const QFileInfo &fileinfo, const QSettings &data);
    ~MemoryLayout();

    typedef enum{
        MEM_UNK = -1,
        MEM_GLOBALS,
        MEM_LANGUAGE,
        MEM_UNIT,
        MEM_SQUAD,
        MEM_WORD,
        MEM_RACE,
        MEM_CASTE,
        MEM_HIST_FIG,
        MEM_HIST_EVT,
        MEM_HIST_ENT,
        MEM_WEP_SUB,
        MEM_MAT,
        MEM_PLANT,
        MEM_ITEM_SUB,
        MEM_DESC,
        MEM_HEALTH,
        MEM_WOUND,
        MEM_ITEM,
        MEM_ITEM_FILTER,
        MEM_ARMOR_SUB,
        MEM_GEN_REF,
        MEM_SYN,
        MEM_EMOTION,
        MEM_ACTIVITY,
        MEM_JOB,
        MEM_SOUL,
        MEM_COUNT
    } MEM_SECTION;

    typedef enum{
        INVALID_FLAGS_1,
        INVALID_FLAGS_2,
        INVALID_FLAGS_3,
        FLAG_TYPE_COUNT
    } UNIT_FLAG_TYPE;

    static const QString section_name(const MEM_SECTION &section){
        QMap<MEM_SECTION,QString> m;
        m[MEM_UNK] = "UNK";
        m[MEM_GLOBALS] = "addresses";
        m[MEM_LANGUAGE] = "offsets";
        m[MEM_UNIT] = "dwarf_offsets";
        m[MEM_SQUAD] = "squad_offsets";
        m[MEM_WORD] = "word_offsets";
        m[MEM_RACE] = "race_offsets";
        m[MEM_CASTE] = "caste_offsets";
        m[MEM_HIST_FIG] = "hist_figure_offsets";
        m[MEM_HIST_EVT] = "hist_event_offsets";
        m[MEM_HIST_ENT] = "hist_entity_offsets";
        m[MEM_WEP_SUB] = "weapon_subtype_offsets";
        m[MEM_MAT] = "material_offsets";
        m[MEM_PLANT] = "plant_offsets";
        m[MEM_ITEM_SUB] = "item_subtype_offsets";
        m[MEM_DESC] = "descriptor_offsets";
        m[MEM_HEALTH] = "health_offsets";
        m[MEM_WOUND] = "unit_wound_offsets";
        m[MEM_ITEM] = "item_offsets";
        m[MEM_ITEM_FILTER] = "item_filter_offsets";
        m[MEM_ARMOR_SUB] = "armor_subtype_offsets";
        m[MEM_GEN_REF] = "general_ref_offsets";
        m[MEM_SYN] = "syndrome_offsets";
        m[MEM_EMOTION] = "emotion_offsets";
        m[MEM_ACTIVITY] = "activity_offsets";
        m[MEM_JOB] = "job_details";
        m[MEM_SOUL] = "soul_details";
        return m.value(section,m.value(MEM_UNK));
    }

    static const QString flag_type_name(const UNIT_FLAG_TYPE &flag_type){
        QMap<UNIT_FLAG_TYPE,QString> m;
        m[INVALID_FLAGS_1] = "invalid_flags_1";
        m[INVALID_FLAGS_2] = "invalid_flags_2";
        m[INVALID_FLAGS_3] = "invalid_flags_3";
        return m.value(flag_type);
    }

    QSettings &data() { return m_data; }
    QString filename() {return m_fileinfo.fileName();}
    QString filepath() {return m_fileinfo.absoluteFilePath();}
    bool is_valid();
    bool is_complete() {return m_complete;}
    QString game_version() {return m_game_version;}
    QString checksum() {return m_checksum;}
    QString git_sha() {return m_git_sha;}
    bool is_valid_address(VIRTADDR addr);
    uint string_buffer_offset();
    uint string_length_offset();
    uint string_cap_offset();

    QHash<QString,VIRTADDR> get_section_offsets(const MEM_SECTION &section) {
        return m_offsets.value(section);
    }
    VIRTADDR offset(const MEM_SECTION &section, const QString &name) const{
        return m_offsets.value(section).value(name,-1);
    }
    QHash<uint,QString> get_flags(const UNIT_FLAG_TYPE &flag_type){
        return m_flags.value(flag_type);
    }

    QHash<QString, VIRTADDR> globals() {return get_section_offsets(MEM_GLOBALS);}

    VIRTADDR address(const QString &key, const bool is_global = true);

    qint16 language_offset(const QString &key) const {return offset(MEM_LANGUAGE,key);}
    qint16 dwarf_offset(const QString &key) const {return offset(MEM_UNIT,key);}
    qint16 squad_offset(const QString & key) const {return offset(MEM_SQUAD,key);}
    qint16 word_offset(const QString & key) const {return offset(MEM_WORD,key);}
    qint16 race_offset(const QString & key) const {return offset(MEM_RACE,key);}
    qint16 caste_offset(const QString & key) const {return offset(MEM_CASTE,key);}
    qint16 hist_figure_offset(const QString & key) const {return offset(MEM_HIST_FIG,key);}
    qint16 hist_event_offset(const QString & key) const {return offset(MEM_HIST_EVT,key);}
    qint16 hist_entity_offset(const QString & key) const {return offset(MEM_HIST_ENT,key);}
    qint16 weapon_subtype_offset(const QString & key) const {return offset(MEM_WEP_SUB,key);}
    qint16 material_offset(const QString & key) const {return offset(MEM_MAT,key);}
    qint16 plant_offset(const QString & key) const {return offset(MEM_PLANT,key);}
    qint16 item_subtype_offset(const QString & key) const {return offset(MEM_ITEM_SUB,key);}
    qint16 descriptor_offset(const QString & key) const {return offset(MEM_DESC,key);}
    qint16 health_offset(const QString & key) const {return offset(MEM_HEALTH,key);}
    qint16 wound_offset(const QString & key) const {return offset(MEM_WOUND,key);}
    qint16 item_offset(const QString & key) const {return offset(MEM_ITEM,key);}
    qint16 item_filter_offset(const QString & key) const {return offset(MEM_ITEM_FILTER,key);}
    qint16 armor_subtype_offset(const QString & key) const {return offset(MEM_ARMOR_SUB,key);}
    qint16 general_ref_offset(const QString & key) const {return offset(MEM_GEN_REF,key);}
    qint16 syndrome_offset(const QString & key) const {return offset(MEM_SYN,key);}
    qint16 emotion_offset(const QString & key) const {return offset(MEM_EMOTION,key);}
    qint16 activity_offset(const QString & key) const {return offset(MEM_ACTIVITY,key);}
    qint16 job_detail(const QString &key) const {return offset(MEM_JOB,key);}
    qint16 soul_detail(const QString &key) const {return offset(MEM_SOUL,key);}

    QHash<uint, QString> invalid_flags_1() {return get_flags(INVALID_FLAGS_1) ;}
    QHash<uint, QString> invalid_flags_2() {return get_flags(INVALID_FLAGS_2);}
    QHash<uint, QString> invalid_flags_3() {return get_flags(INVALID_FLAGS_3);}

    //Setters
    void set_address(const QString & key, uint value);
    void set_game_version(const QString & value);
    void set_checksum(const QString & checksum);
    void save_data();
    void set_complete();

    bool operator<(const MemoryLayout & rhs) const {
        return m_game_version < rhs.m_game_version;
    }

    void load_data();

private:
    DFInstance *m_df;
    typedef QHash<QString, VIRTADDR> AddressHash;

    QHash<MEM_SECTION,AddressHash> m_offsets;
    QHash<UNIT_FLAG_TYPE, QHash<uint,QString> > m_flags;

    QFileInfo m_fileinfo;
    QString m_checksum;
    QString m_git_sha;
    QString m_game_version;
    QSettings m_data;
    bool m_complete;

    uint read_hex(QString key);
    void read_group(const MEM_SECTION &section);
    void read_flags(const UNIT_FLAG_TYPE &flag_type);
};
Q_DECLARE_METATYPE(MemoryLayout *)
#endif
