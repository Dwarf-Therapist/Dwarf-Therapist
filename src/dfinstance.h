/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef DFINSTANCE_H
#define DFINSTANCE_H

#include "utils.h"
#include "global_enums.h"
#include "truncatingfilelogger.h"

#include <QDir>
#include <QPointer>
#include <memory>

#ifdef Q_OS_WIN
typedef int PID;
#else
#include <unistd.h>
typedef pid_t PID;
#endif

#ifndef VECTOR_MAX_SIZE
#define VECTOR_MAX_SIZE 100000000ul
#endif

class Dwarf;
class FortressEntity;
class ItemSubtype;
class ItemWeaponSubtype;
class Languages;
class Material;
class MemoryLayout;
class Plant;
class Race;
class Reaction;
class Squad;
class Word;
class EmotionGroup;
class Activity;
class EquipWarn;

class DFInstance : public QObject {
    Q_OBJECT
public:

    DFInstance(QObject *parent=0);
    virtual ~DFInstance();

    virtual void find_running_copy() = 0;
    virtual bool df_running() = 0;

    static quint32 ticks_per_day;
    static quint32 ticks_per_month;
    static quint32 ticks_per_season;
    static quint32 ticks_per_year;

    typedef enum{
        DFS_DISCONNECTED = -1,
        DFS_CONNECTED,
        DFS_LAYOUT_OK,
        DFS_GAME_LOADED
    } DFI_STATUS;

    // accessors
    VIRTADDR df_base_addr() const {return m_base_addr;}
    const QString df_checksum() {return m_df_checksum;}
    static QString layout_subdir();
    DFI_STATUS status() const {return m_status;}
    WORD dwarf_race_id() {return m_dwarf_race_id;}
    const std::map<QString, std::unique_ptr<MemoryLayout>> &get_layouts() const { return m_memory_layouts; }
    QDir get_df_dir() { return m_df_dir; }
    WORD current_year() {return m_current_year;}
    WORD dwarf_civ_id() {return m_dwarf_civ_id;}
    const QStringList status_err_msg();

    // memory reading
    template<typename T> T read_mem(VIRTADDR addr) {
        T buf;
        // TODO: error checking?
        read_raw(addr, sizeof(T), &buf);
        return buf;
    }
    virtual USIZE read_raw(VIRTADDR addr, USIZE bytes, void *buf) = 0;
    virtual QString read_string(VIRTADDR addr) = 0;
    USIZE read_raw(VIRTADDR addr, USIZE bytes, QByteArray &buffer);
    BYTE read_byte(VIRTADDR addr);
    WORD read_word(VIRTADDR addr);
    VIRTADDR read_addr(VIRTADDR addr);
    qint16 read_short(VIRTADDR addr);
    qint32 read_int(VIRTADDR addr);
    QVector<VIRTADDR> enumerate_vector(VIRTADDR addr);
    QVector<qint16> enumerate_vector_short(VIRTADDR addr);
    template<typename T>
    QVector<T> enum_vec(VIRTADDR addr) {
        QVector<T> out;
        VIRTADDR start = read_addr(addr);
        VIRTADDR end = read_addr(addr + m_pointer_size);
        USIZE bytes = end - start;
        USIZE count = bytes / sizeof(T);
        if (bytes % sizeof(T)) {
            LOGE << "VECTOR SIZE IS NOT A MULTIPLE OF TYPE";
        } else if (count > VECTOR_MAX_SIZE) {
            LOGE << "Vector at" << hexify(addr) << "too big:" << count;
        } else {
            out.resize(bytes / sizeof(T));
            USIZE bytes_read = read_raw(start, bytes, out.data());
            TRACE << "Found" << bytes_read / sizeof(T) << "things in vector at" << hexify(addr);
        }
        return out;
    }
    Word * read_dwarf_word(VIRTADDR addr);
    QString read_dwarf_name(VIRTADDR addr);

    QString pprint(const QByteArray &ba);

    // Memory layouts
    MemoryLayout *memory_layout() {return m_layout;}
    void set_memory_layout(QString checksum = QString());
    MemoryLayout *get_memory_layout(QString checksum);
    MemoryLayout *find_memory_layout(QString git_sha);
    bool add_new_layout(const QString & filename, const QString data, QString &result_msg);

    USIZE pointer_size() const { return m_pointer_size; }

    // Writing
    virtual USIZE write_raw(VIRTADDR addr, USIZE bytes, const void *buffer) = 0;
    USIZE write_raw(VIRTADDR addr, USIZE bytes, const QByteArray &buffer);
    virtual USIZE write_string(VIRTADDR addr, const QString &str) = 0;
    USIZE write_int(VIRTADDR addr, int val);

    bool is_attached() {return m_attach_count > 0;}
    virtual bool attach() = 0;
    virtual bool detach() = 0;
    virtual int VM_TYPE_OFFSET() {return 0x1;}

    static bool authorize();
    quint32 current_year_time() {return m_cur_year_tick;}
    quint32 current_time() {return m_cur_time;}
    static DFInstance * newInstance();

    // Methods for when we know how the data is layed out
    void load_game_data();
    void read_raws();

    QVector<Dwarf*> load_dwarves();
    void load_reactions();
    void load_races_castes();
    void load_main_vectors();

    void load_item_defs();
    void load_items();

    void load_fortress();
    void load_fortress_name();

    void load_activities();

    void refresh_data();

    QList<Squad*> load_squads(bool show_progress);
    Squad * get_squad(int id);

    int get_labor_count(int id) const {return m_enabled_labor_count.value(id,0);}
    void update_labor_count(int id, int change)
    {
        m_enabled_labor_count[id] += change;
    }

    QString get_language_word(VIRTADDR addr);
    QString get_translated_word(VIRTADDR addr);
    QString get_name(VIRTADDR addr, bool translate);

    Reaction * get_reaction(QString tag) { return m_reactions.value(tag, 0); }
    Race * get_race(const uint & offset) { return m_races.value(offset, NULL); }
    QVector<Race *> get_races() {return m_races;}

    VIRTADDR find_historical_figure(int hist_id);
    VIRTADDR find_identity(int id);
    VIRTADDR find_event(int id);
    QPair<int, QString> find_activity(int histfig_id);
    VIRTADDR find_occupation(int histfig_id);

    FortressEntity * fortress() {return m_fortress;}

    struct pref_stat{
        QStringList names_likes;
        QStringList names_dislikes;
        QString pref_category;
    };

    VIRTADDR get_syndrome(int idx) {
        return m_all_syndromes.value(idx);
    }
    VIRTADDR get_material_template(QString temp_id) {return m_material_templates.value(temp_id);}
    QVector<Material *> get_inorganic_materials() {return m_inorganics_vector;}
    QHash<ITEM_TYPE, QVector<VIRTADDR> > get_all_item_defs() {return m_itemdef_vectors;}
    QVector<VIRTADDR>  get_colors() {return m_color_vector;}
    QVector<VIRTADDR> get_shapes() {return m_shape_vector;}
    QVector<Plant *> get_plants() {return m_plants_vector;}
    QVector<Material *> get_base_materials() {return m_base_materials;}

    QString get_building_name(int id);

    ItemWeaponSubtype* find_weapon_subtype(QString name);
    QMap<QString, ItemWeaponSubtype *> get_ordered_weapon_defs() {return m_ordered_weapon_defs;}

    QList<ItemSubtype*> get_item_subtypes(ITEM_TYPE itype){return m_item_subtypes.value(itype);}
    ItemSubtype* get_item_subtype(ITEM_TYPE itype, int sub_type);

    Material * find_material(int mat_index, short mat_type);

    QVector<VIRTADDR> get_itemdef_vector(ITEM_TYPE i);
    VIRTADDR get_item_address(ITEM_TYPE itype, int item_id);

    QString get_preference_item_name(int index, int subtype);
    QString get_preference_other_name(int index, PREF_TYPES p_type);
    QString get_artifact_name(ITEM_TYPE itype,int item_id);

    inline Material * get_inorganic_material(int index) {
        return m_inorganics_vector.value(index);
    }
    inline Material * get_raw_material(int index) {
        return m_base_materials.value(index);
    }
    inline Plant * get_plant(int index) {
        return m_plants_vector.value(index);
    }
    QString find_material_name(int mat_index, short mat_type, ITEM_TYPE itype, MATERIAL_STATES mat_state = SOLID);
    const QHash<QPair<QString,QString>,pref_stat*> get_preference_stats() {return m_pref_counts;}
    const QHash<int, EmotionGroup*> get_emotion_stats() {return m_emotion_counts;}
    const QHash<ITEM_TYPE,EquipWarn*> get_equip_warnings(){return m_equip_warning_counts;}

    const QString fortress_name();
    QList<Squad*> squads() {return m_squads;}

protected:
    VIRTADDR m_base_addr;
    QString m_df_checksum;
    USIZE m_pointer_size;
    MemoryLayout *m_layout;
    int m_attach_count;
    QTimer *m_heartbeat_timer;
    short m_dwarf_race_id;
    int m_dwarf_civ_id;
    WORD m_current_year;
    QDir m_df_dir;
    QVector<Dwarf*> m_actual_dwarves;
    QVector<Dwarf*> m_labor_capable_dwarves;
    quint32 m_cur_year_tick;
    quint32 m_cur_time;
    QHash<int,int> m_enabled_labor_count;
    DFI_STATUS m_status;

    virtual bool set_pid() = 0;

    void load_population_data();
    void load_role_ratings();
    bool check_vector(VIRTADDR start, VIRTADDR end, VIRTADDR addr);

    /*! this map will hold all loaded and valid memory layouts found
        on disk, the key is a QString of the checksum since other OSs will use
        an MD5 of the binary instead of a PE timestamp */
    std::map<QString, std::unique_ptr<MemoryLayout>> m_memory_layouts; // checksum->layout

    static PID select_pid(QSet<PID> pids);

private slots:
    void heartbeat();

signals:
    // methods for sending progress information to QWidgets
    void connection_interrupted();
    void progress_message(const QString &message);
    void progress_range(int min, int max);
    void progress_value(int value);

private:
    Languages* m_languages;
    FortressEntity* m_fortress;
    QHash<QString, Reaction *> m_reactions;
    QVector<Race *> m_races;

    QMap<QString, ItemWeaponSubtype *> m_ordered_weapon_defs;
    QHash<ITEM_TYPE,QList<ItemSubtype *> > m_item_subtypes;
    QVector<Plant *> m_plants_vector;
    QVector<Material *> m_inorganics_vector;
    QVector<Material *> m_base_materials;

    QVector<VIRTADDR> get_creatures(bool report_progress = true);

    QHash<int,VIRTADDR> m_hist_figures;
    QVector<VIRTADDR> m_fake_identities;
    QHash<int,VIRTADDR> m_occupations;
    QHash<int,VIRTADDR> m_events;
    QMap<int,QPointer<Activity> > m_activities;

    QHash<ITEM_TYPE, QVector<VIRTADDR> > m_itemdef_vectors;
    QHash<ITEM_TYPE, QVector<VIRTADDR> > m_items_vectors;
    QHash<ITEM_TYPE, QHash<int,VIRTADDR> >  m_mapped_items;

    QVector<VIRTADDR> m_color_vector;
    QVector<VIRTADDR> m_shape_vector;
    QVector<VIRTADDR> m_poetic_vector;
    QVector<VIRTADDR> m_music_vector;
    QVector<VIRTADDR> m_dance_vector;

    QHash<QString, VIRTADDR> m_material_templates;

    QVector<VIRTADDR> m_all_syndromes;

    QHash<ITEM_TYPE,EquipWarn*> m_equip_warning_counts;
    QHash<QPair<QString,QString>, pref_stat*> m_pref_counts;
    QHash<int, EmotionGroup*> m_emotion_counts;

    QString m_fortress_name;
    QString m_fortress_name_translated;

    VIRTADDR m_squad_vector;
    QList<Squad*> m_squads;

    void load_hist_figures();
    void load_occupations();
    void index_item_vector(ITEM_TYPE itype);
    void send_connection_interrupted();
};

// specializations for VIRTADDR using pointer size
template<> VIRTADDR DFInstance::read_mem<VIRTADDR>(VIRTADDR addr);
template<> QVector<VIRTADDR> DFInstance::enum_vec<VIRTADDR>(VIRTADDR addr);

#endif // DFINSTANCE_H
