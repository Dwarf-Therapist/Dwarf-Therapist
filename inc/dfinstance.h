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

#include <QDir>
#include "utils.h"
#include "global_enums.h"

class Dwarf;
class FortressEntity;
class ItemSubtype;
class ItemWeaponSubtype;
class Languages;
class Material;
class MemoryLayout;
class Plant;
class QFile;
class Race;
class Reaction;
class Squad;
class Word;

class EmotionGroup;

class DFInstance : public QObject {
    Q_OBJECT
public:

    DFInstance(QObject *parent=0);
    virtual ~DFInstance();

    // factory ctor
    virtual bool find_running_copy(bool connectUnknown = false) = 0;

    static quint32 ticks_per_day;
    static quint32 ticks_per_month;
    static quint32 ticks_per_season;
    static quint32 ticks_per_year;

    // accessors
    bool is_ok() {return m_is_ok;}
    WORD dwarf_race_id() {return m_dwarf_race_id;}
    QList<MemoryLayout*> get_layouts() { return m_memory_layouts.values(); }
    QDir get_df_dir() { return m_df_dir; }
    WORD current_year() {return m_current_year;}
    WORD dwarf_civ_id() {return m_dwarf_civ_id;}

    // revamped memory reading
    virtual USIZE read_raw(const VIRTADDR &addr, const USIZE &bytes, void *buf) = 0;
    USIZE read_raw(const VIRTADDR &addr, const USIZE &bytes, QByteArray &buffer);
    BYTE read_byte(const VIRTADDR &addr);
    WORD read_word(const VIRTADDR &addr);
    VIRTADDR read_addr(const VIRTADDR &addr);
    qint16 read_short(const VIRTADDR &addr);
    qint32 read_int(const VIRTADDR &addr);

    // memory reading
    QVector<VIRTADDR> enumerate_vector(const VIRTADDR &addr);
    QVector<qint16> enumerate_vector_short(const VIRTADDR &addr);

    virtual QString read_string(const VIRTADDR &addr) = 0;

    QString pprint(const QByteArray &ba);

    Word * read_dwarf_word(const VIRTADDR &addr);
    QString read_dwarf_name(const VIRTADDR &addr);

    // Methods for when we know how the data is layed out
    MemoryLayout *memory_layout() {return m_layout;}
    void read_raws();
    QVector<Dwarf*> load_dwarves();
    void load_reactions();
    void load_races_castes();
    void load_main_vectors();

    void load_item_defs();

    void load_fortress();
    void load_fortress_name();

    QList<Squad*> load_squads(bool refreshing = false);
    Squad * get_squad(int id);

    int get_labor_count(int id) const {return m_enabled_labor_count.value(id,0);}
    void update_labor_count(int id, int change)
    {
        m_enabled_labor_count[id] += change;
    }



    // Set layout
    void set_memory_layout(MemoryLayout * layout) { m_layout = layout; }

    // Writing
    virtual USIZE write_raw(const VIRTADDR &addr, const USIZE &bytes, const void *buffer) = 0;
    USIZE write_raw(const VIRTADDR &addr, const USIZE &bytes, const QByteArray &buffer);
    virtual USIZE write_string(const VIRTADDR &addr, const QString &str) = 0;
    USIZE write_int(const VIRTADDR &addr, const int &val);

    bool add_new_layout(const QString & version, QFile & file);
    void layout_not_found(const QString & checksum);

    bool is_attached() {return m_attach_count > 0;}
    virtual bool attach() = 0;
    virtual bool detach() = 0;

    static bool authorize();
    quint32 current_year_time() {return m_cur_year_tick;}
    quint32 current_time() {return m_cur_time;}
    static DFInstance * newInstance();

    // Windows string offsets
#ifdef Q_OS_WIN
    static const int STRING_BUFFER_OFFSET = 4;  // Default value for older windows releases
    static const int STRING_LENGTH_OFFSET = 16; // Relative to STRING_BUFFER_OFFSET
    static const int STRING_CAP_OFFSET = 20;    // Relative to STRING_BUFFER_OFFSET
    static const int VECTOR_POINTER_OFFSET = 0;
#elif defined(Q_OS_LINUX)
    static const int STRING_BUFFER_OFFSET = 0;
    static const int STRING_LENGTH_OFFSET = 0; // Dummy value
    static const int STRING_CAP_OFFSET = 0;    // Dummy value
    static const int VECTOR_POINTER_OFFSET = 0;
#elif defined(Q_OS_MAC)
    static const int STRING_BUFFER_OFFSET = 0;
    static const int STRING_LENGTH_OFFSET = 0; // Dummy value
    static const int STRING_CAP_OFFSET = 0;    // Dummy value
    static const int VECTOR_POINTER_OFFSET = 0;
#endif

    MemoryLayout *get_memory_layout(QString checksum, bool warn = true);

    void load_game_data();
    QString get_language_word(VIRTADDR addr);
    QString get_translated_word(VIRTADDR addr);
    Reaction * get_reaction(QString tag) { return m_reactions.value(tag, 0); }
    Race * get_race(const uint & offset) { return m_races.value(offset, NULL); }
    QVector<Race *> get_races() {return m_races;}

    VIRTADDR find_historical_figure(int hist_id);
    VIRTADDR find_identity(int id);
    VIRTADDR find_event(int id);

    FortressEntity * fortress() {return m_fortress;}

    void index_item_vector(ITEM_TYPE itype);

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

    QVector<VIRTADDR> get_item_vector(ITEM_TYPE i);
    VIRTADDR get_item_address(ITEM_TYPE itype, int item_id);

    QString get_preference_item_name(int index, int subtype);
    QString get_artifact_name(ITEM_TYPE itype,int item_id);

    QString get_color(int index);
    QString get_shape(int index);
    inline Material * get_inorganic_material(int index) {
        return m_inorganics_vector.value(index);
    }
    inline Material * get_raw_material(int index) {
        return m_base_materials.value(index);
    }
    inline Plant * get_plant(int index) {
        return m_plants_vector.value(index);
    }
    QString find_material_name(int mat_index, short mat_type, ITEM_TYPE itype);
    const QHash<QPair<QString,QString>,pref_stat*> get_preference_stats() {return m_pref_counts;}
    const QHash<int, EmotionGroup*> get_emotion_stats() {return m_emotion_counts;}
    const QHash<QPair<QString,int>,int> get_equip_warnings(){return m_equip_warning_counts;}

    const QString fortress_name();
    QList<Squad*> squads() {return m_squads;}

    public slots:
        // if a menu cancels our scan, we need to know how to stop
        void cancel_scan() {m_stop_scan = true;}
protected:
    bool m_stop_scan; // flag that gets set to stop scan loops
    bool m_is_ok;
    int m_bytes_scanned;
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

    void load_population_data();
    void load_role_ratings();
    bool check_vector(const VIRTADDR start, const VIRTADDR end, const VIRTADDR addr);

    template<typename T>
    QVector<T> enum_vec(const VIRTADDR &addr);

    /*! this hash will hold a map of all loaded and valid memory layouts found
        on disk, the key is a QString of the checksum since other OSs will use
        an MD5 of the binary instead of a PE timestamp */
    QHash<QString, MemoryLayout*> m_memory_layouts; // checksum->layout

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
    QHash<int,VIRTADDR> m_events;

    QHash<ITEM_TYPE, QVector<VIRTADDR> > m_itemdef_vectors;
    QHash<ITEM_TYPE, QVector<VIRTADDR> > m_items_vectors;
    QHash<ITEM_TYPE, QHash<int,VIRTADDR> >  m_mapped_items;

    QVector<VIRTADDR> m_color_vector;
    QVector<VIRTADDR> m_shape_vector;

    QHash<QString, VIRTADDR> m_material_templates;

    QVector<VIRTADDR> m_all_syndromes;

    void load_hist_figures();

    QHash<QPair<QString,int>, int> m_equip_warning_counts;
    QHash<QPair<QString,QString>, pref_stat*> m_pref_counts;
    QHash<int, EmotionGroup*> m_emotion_counts;

    QString m_fortress_name;
    QString m_fortress_name_translated;

    VIRTADDR m_squad_vector;

    QList<Squad*> m_squads;
};

#endif // DFINSTANCE_H
