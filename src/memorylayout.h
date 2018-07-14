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
        MEM_ART,
        MEM_JOB,
        MEM_SOUL,
        MEM_VIEWSCR,
        MEM_COUNT
    } MEM_SECTION;

    typedef enum{
        INVALID_FLAGS_1,
        INVALID_FLAGS_2,
        INVALID_FLAGS_3,
        FLAG_TYPE_COUNT
    } UNIT_FLAG_TYPE;

    static const QString section_name(const MEM_SECTION &section){
        switch (section) {
        case MEM_GLOBALS: return "addresses";
        case MEM_LANGUAGE: return "offsets";
        case MEM_UNIT: return "dwarf_offsets";
        case MEM_SQUAD: return "squad_offsets";
        case MEM_WORD: return "word_offsets";
        case MEM_RACE: return "race_offsets";
        case MEM_CASTE: return "caste_offsets";
        case MEM_HIST_FIG: return "hist_figure_offsets";
        case MEM_HIST_EVT: return "hist_event_offsets";
        case MEM_HIST_ENT: return "hist_entity_offsets";
        case MEM_WEP_SUB: return "weapon_subtype_offsets";
        case MEM_MAT: return "material_offsets";
        case MEM_PLANT: return "plant_offsets";
        case MEM_ITEM_SUB: return "item_subtype_offsets";
        case MEM_DESC: return "descriptor_offsets";
        case MEM_HEALTH: return "health_offsets";
        case MEM_WOUND: return "unit_wound_offsets";
        case MEM_ITEM: return "item_offsets";
        case MEM_ITEM_FILTER: return "item_filter_offsets";
        case MEM_ARMOR_SUB: return "armor_subtype_offsets";
        case MEM_GEN_REF: return "general_ref_offsets";
        case MEM_SYN: return "syndrome_offsets";
        case MEM_EMOTION: return "emotion_offsets";
        case MEM_ACTIVITY: return "activity_offsets";
        case MEM_ART: return "art_offsets";
        case MEM_JOB: return "job_details";
        case MEM_SOUL: return "soul_details";
        case MEM_VIEWSCR: return "viewscreen_offsets";
        case MEM_UNK:
        default:
                       return "UNK";
        }
    }

    static const QString flag_type_name(const UNIT_FLAG_TYPE &flag_type){
        switch (flag_type) {
        case INVALID_FLAGS_1: return "invalid_flags_1";
        case INVALID_FLAGS_2: return "invalid_flags_2";
        case INVALID_FLAGS_3: return "invalid_flags_3";
        default: return QString();
        }
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
    qint16 art_offset(const QString & key) const {return offset(MEM_ART,key);}
    qint16 job_detail(const QString &key) const {return offset(MEM_JOB,key);}
    qint16 soul_detail(const QString &key) const {return offset(MEM_SOUL,key);}
    qint16 viewscreen_offset(const QString &key) const {return offset(MEM_VIEWSCR,key);}

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

    unsigned long long read_hex(QString key);
    void read_group(const MEM_SECTION &section);
    void read_flags(const UNIT_FLAG_TYPE &flag_type);
};
Q_DECLARE_METATYPE(MemoryLayout *)
#endif
