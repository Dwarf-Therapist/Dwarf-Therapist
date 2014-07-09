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
#ifndef DWARF_H
#define DWARF_H

#include <QtWidgets>
#include "utils.h"
#include "global_enums.h"
#include "skill.h"
#include "attribute.h"
#include "unithealth.h"
#include "item.h"
#include "itemdefuniform.h"
//#include "itemarmor.h"

class DFInstance;
class MemoryLayout;
class CustomProfession;
class Reaction;
class Role;
class Preference;
class Race;
class Caste;
class Thought;
class Syndrome;
class ItemArmor;
class Uniform;

class Dwarf : public QObject
{
    Q_OBJECT
    friend class Squad;
    Dwarf(DFInstance *df, const uint &addr, QObject *parent=0); //private, use the static get_dwarf() method

public:
    static Dwarf* get_dwarf(DFInstance *df, const VIRTADDR &address);   
    virtual ~Dwarf();

    static quint32 ticks_per_day;
    static quint32 ticks_per_month;
    static quint32 ticks_per_season;
    static quint32 ticks_per_year;

    DFInstance * get_df_instance(){return m_df;}

    // getters
    //! Return the memory address (in hex) of this creature in the remote DF process
    VIRTADDR address() {return m_address;}

    //! return the the unique id for this creature
    int id() const {return m_id;}

    //! return whether or not the dwarf is on break
    bool is_on_break() {return m_is_on_break;}

    //-1 = none, 0 = female, 1 = male
    Q_INVOKABLE GENDER_TYPE get_gender() {return m_gender;}
    Q_INVOKABLE bool is_male() {return (m_gender == SEX_M);}

    //! false if the creature is a dwarf, true if not
    Q_INVOKABLE bool is_animal() {return m_is_animal;}
    Q_INVOKABLE bool is_pet() {return m_is_pet;}
    Q_INVOKABLE TRAINED_LEVEL trained_level() {return m_animal_type;}

    Q_INVOKABLE bool is_adult() {return (!m_is_child && !m_is_baby) ? true : false;}
    Q_INVOKABLE bool is_child() {return m_is_child;}
    Q_INVOKABLE bool is_baby() {return m_is_baby;}

    //! return a text version of this dwarf's profession (will use custom profession if set)
    Q_INVOKABLE QString profession();

    //! return the raw game-set profession for a dwarf
    Q_INVOKABLE int raw_profession() {return m_raw_profession;}

    //! custom profession string (if set)
    Q_INVOKABLE QString custom_profession_name() {return m_pending_custom_profession;}

    //! return a printable name for this dwarf based on user-settings (may include nickname/firstname or both)
    Q_INVOKABLE QString nice_name() const {return m_nice_name;}

    //! return the name of the curse afflicting this dwarf
    Q_INVOKABLE QString curse_name() {return m_curse_name;}

    //! convenience function for checking the curse name
    Q_INVOKABLE bool is_cursed() {return (!m_curse_name.trimmed().isEmpty());}

    eCurse::CURSE_TYPE get_curse_type() {return m_curse_type;}

    //! return a printable name for this dwarf where each dwarven word is translated to english (not game human)
    QString translated_name() {return m_translated_name;}

    //! return the string nickname for this dwarf (if set)
    QString nickname() {return m_pending_nick_name;}

    //! return the happiness level of this dwarf
    DWARF_HAPPINESS get_happiness() {return m_happiness;}

    //! return the raw happiness score for this dwarf
    Q_INVOKABLE int get_raw_happiness() {return m_raw_happiness;}

    //! return this dwarf's strength attribute score    
    Q_INVOKABLE int strength() {return attribute((int)AT_STRENGTH);}

    //! return this dwarf's agility attribute score
    Q_INVOKABLE int agility() {return attribute((int)AT_AGILITY);}

    //! return this dwarf's toughness attribute score
    Q_INVOKABLE int toughness() {return attribute((int)AT_TOUGHNESS);}

    //! return this dwarf's endurance attribute score
    Q_INVOKABLE int endurance() {return attribute((int)AT_ENDURANCE);}

    //! return this dwarf's recuperation attribute score
    Q_INVOKABLE int recuperation() {return attribute((int)AT_RECUPERATION);}

    //! return this dwarf's disease resistance attribute score
    Q_INVOKABLE int disease_resistance() {return attribute((int)AT_DISEASE_RESISTANCE);}

    //! return this dwarf's willpower attribute score
    Q_INVOKABLE int willpower() {return attribute((int)AT_WILLPOWER);}

    //! return this dwarf's memory attribute score
    Q_INVOKABLE int memory() {return attribute((int)AT_MEMORY);}

    //! return this dwarf's focus attribute score
    Q_INVOKABLE int focus() {return attribute((int)AT_FOCUS);}

    //! return this dwarf's intuition attribute score
    Q_INVOKABLE int intuition() {return attribute((int)AT_INTUITION);}

    //! return this dwarf's patience attribute score
    Q_INVOKABLE int patience() {return attribute((int)AT_PATIENCE);}

    //! return this dwarf's empathy attribute score
    Q_INVOKABLE int empathy() {return attribute((int)AT_EMPATHY);}

    //! return this dwarf's social awareness attribute score
    Q_INVOKABLE int social_awareness() {return attribute((int)AT_SOCIAL_AWARENESS);}

    //! return this dwarf's creativity attribute score
    Q_INVOKABLE int creativity() {return attribute((int)AT_CREATIVITY);}

    //! return this dwarf's musicality attribute score
    Q_INVOKABLE int musicality() {return attribute((int)AT_MUSICALITY);}

    //! return this dwarf's analytical ability attribute score
    Q_INVOKABLE int analytical_ability() {return attribute((int)AT_ANALYTICAL_ABILITY);}

    //! return this dwarf's linguistic ability attribute score
    Q_INVOKABLE int linguistic_ability() {return attribute((int)AT_LINGUISTIC_ABILITY);}

    //! return this dwarf's spatial sense attribute score
    Q_INVOKABLE int spatial_sense() {return attribute((int)AT_SPATIAL_SENSE);}

    //! return this dwarf's kinesthetic sense attribute score
    Q_INVOKABLE int kinesthetic_sense() {return attribute((int)AT_KINESTHETIC_SENSE);}

    //! return this dwarf's squad reference id
    Q_INVOKABLE int squad_id(bool original = false) { return (original ? m_squad_id : m_pending_squad_id);}
    Q_INVOKABLE int squad_position(bool original = false) { return (original ? m_squad_position : m_pending_squad_position);}
    Q_INVOKABLE int historical_id() { return m_hist_id;}

    void update_squad_info(int squad_id, int position, QString name);

    //! return this dwarf's caste id
    Q_INVOKABLE short get_caste_id() { return m_caste_id; }

    //! return this creature's race id
    Q_INVOKABLE short get_race_id() { return m_race_id; }

    //! return this creature's race
    Race* get_race() { return m_race; }

    //! return this creature's age
    Q_INVOKABLE short get_age() { return m_age; }
    QString get_age_formatted();

    //! return this creature's flag1
    Q_INVOKABLE quint32 get_flag1() { return m_unit_flags.at(0); }

    //! return this creature's flag2
    Q_INVOKABLE quint32 get_flag2() { return m_unit_flags.at(1); }

    //! return this creature's flag2
    Q_INVOKABLE quint32 get_flag3() { return m_unit_flags.at(2); }

    //! return this creature's Nth bit from flags
    Q_INVOKABLE bool get_flag_value(int bit);

    static bool has_invalid_flags(const QString creature_name, QHash<uint, QString> invalid_flags, quint32 dwarf_flags);

    //! return this dwarf's highest skill
    Skill highest_skill();

    //! sum total of all skill levels in any skill
    Q_INVOKABLE int total_skill_levels();

    //! number of activated labors
    Q_INVOKABLE int total_assigned_labors(bool include_hauling = true);

    const QMap<int, ushort> get_labors() {return m_pending_labors;}

    void clear_labors();
    void assign_all_labors();
    void toggle_skilled_labors();
    void toggle_skilled_labor(int labor_id);

    //! return the sum total of all xp this dwarf has earned
    int total_xp() {return m_total_xp;}

    //! return the probable migration wave this dwarf arrived in (purely a guess)
    int migration_wave() {return m_migration_wave;}

    //! returns a description of birth or migration
    QString get_migration_desc();

    //! return true if the dwarf's raw_profession is a military professions
    Q_INVOKABLE bool active_military();

    //! return a hash of skill_id,Skill objects that this dwarf has experience in
    QHash<int, Skill> *get_skills() {return &m_skills;}
    QVector<Attribute> *get_attributes() {return &m_attributes;}
    QHash<int, short> *get_traits(){return &m_traits;}    
    void load_trait_values(QVector<double> &list);
    QMultiMap<int,Preference*> *get_preferences(){return &m_preferences;}    

    int get_role_pref_match_count(Role *r);

    //! return a skill object by skill_id
    Skill get_skill(int skill_id);

    //! return all labors that the user has toggled, but not comitted to DF yet
    QVector<int> get_dirty_labors(); // returns labor ids

    //! return true if the labor specified by labor_id is enabled or pending enabled
    Q_INVOKABLE bool labor_enabled(int labor_id);

    //! return true if the labor specified by labor_id has been toggled and not committed
    Q_INVOKABLE bool is_labor_state_dirty(int labor_id);

    bool is_flag_dirty(int bit_pos);

    //! return the numeric value of a preference setting (uses labor ids for offset)
    short pref_value(const int &labor_id);

    //! return this dwarf's numeric score for the trait specified by trait_id
    Q_INVOKABLE short trait(int trait_id) {
        if(m_traits.contains(trait_id))
            return m_traits[trait_id];
        else
            return -1;
    }

    bool trait_is_active(int trait_id);

    Q_INVOKABLE int attribute(int attrib_id) {return get_attribute(attrib_id).get_value();}
    Attribute get_attribute(int id);

    //! returns the numeric rating for the this dwarf in the skill specified by skill_id
    Q_INVOKABLE float skill_level(int skill_id, bool raw = false, bool precise = false);

    //! returns the numeric rating for the this dwarf in the skill associated with the labor specified by labor_id
    Q_INVOKABLE short labor_rating(int labor_id);

    //! return a hashmap of trait_id to trait score for this dwarf
    const QHash<int, short> &traits() {return m_traits;}

    //! return a hashmap of roles and ratings for this dwarf
    const QHash<QString, float> &role_ratings() {return m_role_ratings;}

    //! return a hashmap of roles and ratings for this dwarf, sorted by rating
    const QList<Role::simple_rating> &sorted_role_ratings();

    //! return the text string describing what this dwarf is currently doing ("Idle", "Construct Rock Door" etc...)
    const QString &current_job() {return m_current_job;}    

    //! return the id of the job this dwarf is currently doing
    Q_INVOKABLE short current_job_id() {return m_current_job_id;}

    //! return the id of the sub job this dwarf is currently doing
    const QString &current_sub_job_id() { return m_current_sub_job_id; }

    //! return the total number of changes to this dwarf are uncommitted
    int pending_changes();

    //! return a formatted string suitable for showing in tooltips for this dwarf
    QString tooltip_text();

    bool is_valid();
    // setters
    //! this will cause all data for this dwarf to be reset to game values (clears all pending uncomitted changes)
    void refresh_data();

    //! set the pending nickname for this dwarf (does not auto-commit)
    Q_INVOKABLE void set_nickname(const QString &nick);

    //! set the migration wave this dwarf (DwarfModel currently calls this with its best guess)
    void set_migration_wave(const int &wave_number) {m_migration_wave = wave_number;}

    /*! manually set a labor as enabled or disabled for this dwarf. This method automatically unsets
    exclusive partners of a labor, or weapon choice. It also defends against cheating by not allowing
    labors to be set on certain professions (Baby, Child, Nobles, etc...)
    */
    void set_labor(int labor_id, bool enabled, bool update_cols_realtime=true);

    //! convenience method that calls set_labor() and switches the state of the labor specified by labor_id
    bool toggle_labor(int labor_id);

    //! toggle the n_bit bit of the flag
    bool toggle_flag_bit(int n_bit);

    //! set's the pending custom profession text for this dwarf
    void set_custom_profession_text(const QString &prof_text);

    /*! sets the pending custom profession text for this dwarf as well as sets all labors for
    this dwarf to match the labor template of the CustomProfession object.
    */
    int apply_custom_profession(CustomProfession *cp); // return # of pending changes

    /*! clears the pending custom profession text for this dwarf. This will cause the dwarf to
    show up as their default profession in game
    */
    void reset_custom_profession() {m_pending_custom_profession = "";}

    QList<float> calc_role_ratings();
    float calc_role_rating(Role *);
    Q_INVOKABLE float get_role_rating(QString role_name, bool raw = false);
    Q_INVOKABLE float get_adjusted_role_rating(QString role_name){return m_adjusted_role_ratings.value(role_name);}
    void set_role_rating(QString role_name, float value);
    void set_adjusted_role_rating(QString role_name, float value);
    void update_rating_list();

    void calc_attribute_ratings();

    //! static method for mapping a numeric happiness score into a value of the enum DWARF_HAPPINESS
    static DWARF_HAPPINESS happiness_from_score(int score);

    //! static method for mapping a value in the enum DWARF_HAPPINESS to a meaningful text string
    static QString happiness_name(DWARF_HAPPINESS happiness);

    //! static method for mapping a value in the enum ATTRIBUTES_TYPE to a meaningful text string
    static QString attribute_level_name(ATTRIBUTES_TYPE attribute, short value);

    Caste *get_caste() {return m_caste;}

    //! method for mapping a caste id to a meaningful text name string
    Q_INVOKABLE QString caste_name(bool plural_name = false);

    Q_INVOKABLE QString caste_tag();

    //! static method for mapping a caste id to a meaningful text description string
    Q_INVOKABLE QString caste_desc();

    //! static method for mapping a race id to a meaningful text string
    QString race_name(bool base = false, bool plural_name = false);

    //! used for building a datamodel that shows all pending changes this dwarf has queued up
    QTreeWidgetItem *get_pending_changes_tree();

    //! convenience hack allowing Dwarf objects to know where they live in the gridview model
    QModelIndex m_name_idx;

    //! get's a list of QActions that can be activated on this dwarf, suitable for adding to Toolbars or context menus
    QList<QAction*> get_mem_actions() {return m_actions_memory;}

    //! returns true if this dwarf can have labors specified on it
    Q_INVOKABLE bool can_set_labors() {return m_can_set_labors;}

    //birth year/time invokables
    Q_INVOKABLE quint32 get_birth_time() { return m_birth_time; }
    Q_INVOKABLE quint32 get_birth_year() { return m_birth_year; }
    Q_INVOKABLE bool born_in_fortress() { return m_born_in_fortress; }
    Q_INVOKABLE void set_born_in_fortress(bool val) { m_born_in_fortress = val; }

    QString first_name() const {
        //qDebug() << "first_name called (from script?)";
        return m_first_name;
    }

//    QString squad_name() const {
//        return m_squad_name;
//    }
    QString squad_name();

    uint turn_count() const {
        return m_turn_count;
    }

    Q_INVOKABLE QString noble_position() {return m_noble_position;}

    QPixmap profession_icon() {return m_icn_prof;}
    QString gender_icon_path() {return m_icn_gender;}

    Q_INVOKABLE int body_size(bool use_default = false);

    bool has_state(short id){return m_states.contains(id);}
    int state_value(short id){return m_states.value(id,-1);}

    Reaction *get_reaction();

    void recheck_equipment();

    void find_true_ident();

    Skill highest_moodable();
    bool had_mood() {return m_had_mood;}
    QString artifact_name() {return m_artifact_name;}

    QHash<QString, QStringList*> get_grouped_preferences() {return m_grouped_preferences;}

    QHash<short, int> get_thoughts() {return m_thoughts;}

    Q_INVOKABLE bool has_preference(QString pref_name, QString category = "", bool exactMatch = true);
    Q_INVOKABLE bool has_thought(short id) {return m_thoughts.contains(id);}
    Q_INVOKABLE bool has_health_issue(int id, int idx = -1);

    Q_INVOKABLE bool is_buffed();
    Q_INVOKABLE QString buffs();

    Q_INVOKABLE bool has_syndrome(QString name);
    Q_INVOKABLE QString syndromes();

    QString get_syndrome_names(bool include_buffs, bool include_sick);

    QString get_thought_desc() {return m_thought_desc;}

    UnitHealth get_unit_health() {return m_unit_health;}    

    //! returns a list of items, grouped by body part name
    QHash<QString,QList<Item*> > get_inventory_grouped(){return m_inventory_grouped;}

    //! returns a percentage of the crucial clothing (shirt, shoes, pants)
    Q_INVOKABLE float get_coverage_rating(ITEM_TYPE itype = NONE);
    //! returns the number of items missing
    Q_INVOKABLE int get_missing_equip_count(ITEM_TYPE itype = NONE);
    //! returns a percentage of the inventory items / uniform items
    Q_INVOKABLE float get_uniform_rating(ITEM_TYPE itype = NONE);
    //! returns a percentage of how worn out items a dwarf is wearing
    Q_INVOKABLE int get_inventory_wear(ITEM_TYPE itype = NONE);
    int optimized_labors;


    public slots:
        //! called when global user settings change
        void read_settings();
        //! show a dialog with a memory dump for this dwarf...
        void dump_memory();
        //! dump dwarf memory to a file using the dwarf's name
        void dump_memory_to_file();
        //! show details for this dwarf in a new window...
        void show_details();
        //! copy this dwarf's memory address to the clipboard (debugging)
        void copy_address_to_clipboard();

        //! undo any uncomitted changes to this dwarf (reset back to game-state)
        void clear_pending();
        //! write all uncommitted pending changes back to the game (DANGEROUS METHOD)
        void commit_pending(bool single=false);

        //sorts ratings
        static bool sort_ratings(const Role::simple_rating &r1, const Role::simple_rating &r2){return r1.rating > r2.rating;}
        static bool sort_ratings_custom(const Role::simple_rating &r1, const Role::simple_rating &r2){
            if(r1.is_custom)
                return true;
            else if(r2.is_custom)
                return false;
            else
                return r1.rating > r2.rating;
        }

private:
    int m_id; // each creature in the game has a unique serial ID
    DFInstance *m_df;
    MemoryLayout *m_mem;
    VIRTADDR m_address; // start of the structure in DF's memory space
    VIRTADDR m_first_soul; // start of 1st soul for this creature
    int m_race_id; // each creature has racial ID
    DWARF_HAPPINESS m_happiness; // enum value of happiness
    int m_raw_happiness; // raw score before being turned into an enum
    GENDER_TYPE m_gender;

    int m_mood_id;
    bool m_had_mood;
    QString m_artifact_name;
    QString m_curse_name;
    short m_caste_id;
    bool m_show_full_name;
    int m_total_xp;
    int m_migration_wave;
    int m_body_size;
    TRAINED_LEVEL m_animal_type;

    Q_PROPERTY(QString first_name READ first_name) // no setters (read-only)
    QString m_first_name; // set by game
    QString m_nick_name; // set by user
    QString m_pending_nick_name; // used when not committed yet
    QString m_last_name; // last name in dwarven
    QString m_translated_last_name; // last name in human english
    QString m_nice_name; // full name (depends on settings)
    QString m_translated_name; // full name using human english last name
    QString m_custom_profession; // set by user
    QString m_pending_custom_profession; // uncommitted
    QString m_profession; // name of profession set by game
    QPixmap m_icn_prof;
    QString m_icn_gender;
    int m_raw_profession; // id of profession set by game
    bool m_can_set_labors; // used to prevent cheating
    short m_current_job_id;
    QString m_current_job;
    QString m_current_sub_job_id;    
    QHash<int,Skill> m_skills;
    QMultiMap<float, int> m_sorted_skills; //level, skill_id
    QHash<int, short> m_traits;    
    QVector<Attribute> m_attributes;
    QMap<int, ushort> m_labors;
    QMap<int, ushort> m_pending_labors;
    QList<QAction*> m_actions_memory; // actions suitable for context menus    
    int m_hist_id;
    int m_squad_id;
    int m_squad_position;
    int m_pending_squad_id;
    int m_pending_squad_position;
    QString m_pending_squad_name; //The name of the squad that the dwarf belongs to (if any)
    QList<quint32> m_unit_flags;
    quint32 m_caged;
    quint32 m_butcher;
    short m_age;
    short m_age_in_months;
    uint m_turn_count; // Dwarf turn count from start of fortress (as best we know)
    bool m_is_on_break;
    QHash<QString, float> m_adjusted_role_ratings;
    QHash<QString, float> m_role_ratings;
    QHash<QString, float> m_raw_role_ratings;
    QList<Role::simple_rating> m_sorted_role_ratings;
    QList<QPair<QString,float> > m_sorted_custom_role_ratings;
    QHash<short, int> m_states;
    bool m_born_in_fortress;
    quint32 m_birth_year;
    quint32 m_birth_time;
    VIRTADDR m_hist_nickname;
    VIRTADDR m_fake_nickname;
    QString m_noble_position;
    bool m_is_pet;
    int m_highest_moodable_skill;
    Race* m_race;
    Caste* m_caste;
    QMultiMap<int, Preference*> m_preferences;
    QHash<QString, QStringList*> m_grouped_preferences;
    QHash<short, int> m_thoughts;
    QString m_thought_desc;
    bool m_is_child;
    bool m_is_baby;
    bool m_is_animal;
    QString m_true_name; //used for vampires
    int m_true_birth_year; //used for vampires
    UnitHealth m_unit_health;
    bool m_validated;
    bool m_is_valid;
    QList<Syndrome> m_syndromes;
    quint32 m_curse_flags;    
    Uniform* m_uniform;

    //! inventory grouped by body part /category
    QHash<QString,QList<Item*> > m_inventory_grouped;
    //! inventory coverage ratings
    QHash<ITEM_TYPE,int> m_coverage_ratings;
    //! inventory wear levels
    QHash<ITEM_TYPE,int> m_inventory_wear;
    QHash<ITEM_TYPE,int> m_missing_counts;

    //quint32 m_curse_flags2;
    eCurse::CURSE_TYPE m_curse_type;
    QHash<ATTRIBUTES_TYPE,QPair<float,int> > m_attribute_mods_perm; //permanent modification of attributes via syndromes (and curses?)
    QHash<ATTRIBUTES_TYPE,QPair<float,int> > m_attribute_mods_temp; //temporary modification of attributes via syndromes (and curses?)
    QHash<ATTRIBUTES_TYPE,QStringList> m_attribute_syndromes;

    // these methods read data from raw memory
    void read_id();
    void read_sex();
    void read_mood();
    void read_curse();
    void read_caste();
    void read_race();
    void read_body_size();
    void read_first_name();
    void read_last_name(VIRTADDR name_offset);
    void read_nick_name();
    void read_states();
    void read_profession();
    void read_labors();
    void read_happiness();
    void read_current_job();
    void read_soul();
    void read_soul_aspects();
    void read_skills();
    void read_attributes();
    void load_attribute(VIRTADDR &addr, int id);
    void read_traits();    
    void read_flags();
    void read_turn_count();
    void read_animal_type();
    void read_noble_position();
    void read_preferences();    
    void read_syndromes();
    void read_squad_info();
    void read_inventory();
    void read_uniform();

    void process_inv_item(QString category, Item *item, bool is_contained_item=false);

    void set_age_and_migration(VIRTADDR birth_year_offset, VIRTADDR birth_time_offset);

    // utility methods to assist with reading names made up of several words
    // from the language tables
    QString word_chunk(uint word, bool use_generic=false);
    QString read_chunked_name(const VIRTADDR &addr, bool use_generic=false);

    // assembles component names into a nicely formatted single string
    void build_names();

signals:
    void name_changed();
};

#endif // DWARF_H
