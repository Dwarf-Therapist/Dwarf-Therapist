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

#include "utils.h"
#include "global_enums.h"
#include "skill.h"
#include "attribute.h"
#include "unithealth.h"
#include "unitbelief.h"
#include "role.h"
#include "syndrome.h"
#include "equipwarn.h"
#include "dftime.h"
#include <QModelIndex>
#include <memory>

class QAction;
class DFInstance;
class MemoryLayout;
class CustomProfession;
class Profession;
class Reaction;
class Preference;
class RolePreference;
class Race;
class Caste;
class Uniform;
class HistFigure;
class UnitEmotion;
class UnitNeed;

class Dwarf : public QObject
{
    Q_OBJECT
    friend class Squad;

public:
    Dwarf(DFInstance *df, VIRTADDR addr, QObject *parent=0);
    virtual ~Dwarf();

    DFInstance * get_df_instance(){return m_df;}

    //! Return the memory address (in hex) of this creature in the remote DF process
    VIRTADDR address() {return m_address;}

    //! return the the unique id for this creature
    Q_INVOKABLE int id() const {return m_id;}

    bool is_valid() {return m_is_valid;}

    enum GENDER_TYPE {
        SEX_UNK = -1,
        SEX_F = 0,
        SEX_M = 1
    };
    Q_ENUMS(GENDER_TYPE)

    enum SEX_ORIENT_TYPE {
        ORIENT_ASEXUAL,
        ORIENT_BISEXUAL,
        ORIENT_HOMO,
        ORIENT_HETERO
    };
    Q_ENUMS(SEX_ORIENT_TYPE)

    enum SEX_COMMITMENT {
        COMMIT_UNINTERESTED = 0,
        COMMIT_LOVER = 1,
        COMMIT_MARRIAGE = 2,
    };
    Q_ENUMS(SEX_COMMITMENT)

    enum MISC_STATES {
        STATE_MIGRANT = 7,
        STATE_CAVE_ADAPT = 16
    };

    enum TRAINED_LEVEL {
        none=-1, //custom
        semi_wild=0,
        trained=1,
        well_trained=2,
        skillfully_trained=3,
        expertly_trained=4,
        exceptionally_trained=5,
        masterfully_trained=6,
        domesticated=7,
        unknown_trained=8,
        wild_untamed=9,
        hostile=10 //custom
    };
    Q_ENUMS(TRAINED_LEVEL)

    enum UNIT_OCCUPATION {
        OCC_NONE = -1,
        OCC_TAVERN,
        OCC_PERFORMER,
        OCC_SCHOLAR,
        OCC_MERC,
        OCC_MONSTER,
        OCC_SCRIBE,
        OCC_MESSENGER,
        OCC_DOCTOR,
        OCC_DIAGNOSTICIAN,
        OCC_SURGEON,
        OCC_BONE_DOCTOR,
    };

    static inline QString get_animal_trained_descriptor(const TRAINED_LEVEL &type) {
        switch (type) {
        case semi_wild: return tr("Semi-wild");
        case trained: return tr("Trained");
        case well_trained: return tr("Well-trained");
        case skillfully_trained: return tr("Skillfully Trained");
        case expertly_trained: return tr("Expertly Trained");
        case exceptionally_trained: return tr("Exceptionally Trained");
        case masterfully_trained: return tr("Masterfully Trained");
        case domesticated: return tr("Tame");
        case wild_untamed: return tr("Wild");
        case hostile: return tr("Hostile");
        default:
            return tr("Unknown");
        }
    }


    static QString get_gender_desc(const GENDER_TYPE &type) {
        switch (type) {
        case SEX_UNK: return tr("Other");
        case SEX_F: return tr("Female");
        case SEX_M: return tr("Male");
        default:
            return tr("Unknown");
        }
    }

    static QString get_orientation_desc(const SEX_ORIENT_TYPE &type) {
        switch (type) {
        case ORIENT_ASEXUAL: return tr("Asexual");
        case ORIENT_HOMO: return tr("Homosexual");
        case ORIENT_HETERO: return tr("Heterosexual");
        case ORIENT_BISEXUAL: return tr("Bisexual");
        default:
            return "";
        }
        return "";
    }

    static QString get_occupation_desc(const UNIT_OCCUPATION &type) {
        switch (type) {
        case OCC_NONE: return tr("None");
        case OCC_TAVERN: return tr("Tavern Keeper");
        case OCC_PERFORMER: return tr("Performer");
        case OCC_SCHOLAR: return tr("Scholar");
        case OCC_MERC: return tr("Mercenary");
        case OCC_MONSTER: return tr("Monster Slayer");
        case OCC_SCRIBE: return tr("Scribe");
        case OCC_MESSENGER: return tr("Messenger");
        case OCC_DOCTOR: return tr("Doctor");
        case OCC_DIAGNOSTICIAN: return tr("Diagnostician");
        case OCC_SURGEON: return tr("Surgeon");
        case OCC_BONE_DOCTOR: return tr("Bone Doctor");
        default:
            return tr("Unknown");
        }
    }

    struct unit_gender{
        GENDER_TYPE gender;
        SEX_ORIENT_TYPE orientation;
        SEX_COMMITMENT male, female;
        QString full_desc;
    };

    enum GenderInfoOption
    {
        Option_SexOnly = 0,
        Option_ShowOrientation,
        Option_ShowCommitment,
    };

    Q_INVOKABLE GENDER_TYPE get_gender() const {return m_gender_info.gender;}
    Q_INVOKABLE SEX_ORIENT_TYPE get_orientation() const {return m_gender_info.orientation;}
    QString get_gender_orient_desc() const {return m_gender_info.full_desc;}
    Q_INVOKABLE SEX_COMMITMENT same_sex_commitment() const { return m_gender_info.gender == SEX_M ? m_gender_info.male : m_gender_info.female; }
    Q_INVOKABLE SEX_COMMITMENT other_sex_commitment() const { return m_gender_info.gender == SEX_M ? m_gender_info.female : m_gender_info.male; }
    Q_INVOKABLE bool is_male() const {return (m_gender_info.gender == SEX_M);}
    Q_INVOKABLE bool is_female() const {return (m_gender_info.gender == SEX_F);}

    Q_INVOKABLE bool is_animal() {return m_is_animal;}
    Q_INVOKABLE bool is_pet() {return m_is_pet;}
    Q_INVOKABLE TRAINED_LEVEL trained_level() {return m_animal_type;}

    Q_INVOKABLE bool is_adult() {return !is_child() && !is_baby();}
    Q_INVOKABLE bool is_child() {return m_raw_prof_id == 103;}
    Q_INVOKABLE bool is_baby() {return m_raw_prof_id == 104;}

    Q_INVOKABLE bool is_citizen() {return m_is_citizen;}

    Q_INVOKABLE QString occupation() {return get_occupation_desc(m_occ_type);}
    UNIT_OCCUPATION get_occupation() {return m_occ_type;}

    //! return a text version of this dwarf's profession (will use custom profession if set)
    Q_INVOKABLE QString profession();

    //! return the raw game-set profession for a dwarf
    Q_INVOKABLE int raw_profession() {return m_raw_prof_id;}

    //! custom profession string (if set)
    Q_INVOKABLE QString custom_profession_name() {return m_pending_custom_profession;}

    bool is_custom_profession_dirty(QString name);

    Q_INVOKABLE bool name_matches(QString val) const {return m_nice_name.contains(val,Qt::CaseInsensitive);}
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
    Q_INVOKABLE QString nickname() {return m_pending_nick_name;}

    //! return the happiness level of this dwarf
    DWARF_HAPPINESS get_happiness() const {return m_happiness;}
    QString get_happiness_desc() const {return m_happiness_desc;}

    //! return the raw happiness score for this dwarf
    Q_INVOKABLE int get_raw_happiness() const {return m_stress_level;}

    // focus and need values
    enum FOCUS_DEGREE {
        FOCUS_BADLY_DISTRACTED = 0,
        FOCUS_DISTRACTED,
        FOCUS_UNFOCUSED,
        FOCUS_UNTROUBLED,
        FOCUS_SOMEWHAT_FOCUSED,
        FOCUS_QUITE_FOCUSED,
        FOCUS_VERY_FOCUSED,
        FOCUS_DEGREE_COUNT
    };
    Q_INVOKABLE int get_need_type_focus(int need_id) const;
    //! return 0 if the dwarf does not have this need
    Q_INVOKABLE int get_need_type_level(int need_id) const;
    Q_INVOKABLE int get_need_focus(int need_id, int deity_id) const;
    Q_INVOKABLE int get_need_level(int need_id, int deity_id) const;
    Q_INVOKABLE int get_need_focus_degree(int need_id, int deity_id) const;
    Q_INVOKABLE int get_current_focus() const { return m_current_focus; }
    Q_INVOKABLE int get_undistracted_focus() const { return m_undistracted_focus; }
    Q_INVOKABLE int get_focus_degree() const { return m_current_focus_degree; }
    static QString get_focus_adjective(int degree);
    QString get_focus_adjective() const;
    // set tooltip to true to use colors adapted to tooltip instead of window text.
    static QColor get_focus_color(int degree, bool tooltip = false, bool background = false);
    QString get_focus_desc(bool color = true) const;
    const auto &get_needs() const { return m_needs; }
    auto get_needs(int need_id) const { return m_needs.equal_range(need_id); }

    //! return specific attribute values
    Q_INVOKABLE int strength() {return attribute(AT_STRENGTH);}
    Q_INVOKABLE int agility() {return attribute(AT_AGILITY);}
    Q_INVOKABLE int toughness() {return attribute(AT_TOUGHNESS);}
    Q_INVOKABLE int endurance() {return attribute(AT_ENDURANCE);}
    Q_INVOKABLE int recuperation() {return attribute(AT_RECUPERATION);}
    Q_INVOKABLE int disease_resistance() {return attribute(AT_DISEASE_RESISTANCE);}
    Q_INVOKABLE int willpower() {return attribute(AT_WILLPOWER);}
    Q_INVOKABLE int memory() {return attribute(AT_MEMORY);}
    Q_INVOKABLE int focus() {return attribute(AT_FOCUS);}
    Q_INVOKABLE int intuition() {return attribute(AT_INTUITION);}
    Q_INVOKABLE int patience() {return attribute(AT_PATIENCE);}
    Q_INVOKABLE int empathy() {return attribute(AT_EMPATHY);}
    Q_INVOKABLE int social_awareness() {return attribute(AT_SOCIAL_AWARENESS);}
    Q_INVOKABLE int creativity() {return attribute(AT_CREATIVITY);}
    Q_INVOKABLE int musicality() {return attribute(AT_MUSICALITY);}
    Q_INVOKABLE int analytical_ability() {return attribute(AT_ANALYTICAL_ABILITY);}
    Q_INVOKABLE int linguistic_ability() {return attribute(AT_LINGUISTIC_ABILITY);}
    Q_INVOKABLE int spatial_sense() {return attribute(AT_SPATIAL_SENSE);}
    Q_INVOKABLE int kinesthetic_sense() {return attribute(AT_KINESTHETIC_SENSE);}
    //! attribute value from id
    Q_INVOKABLE int attribute(int attrib_id) {return get_attribute(static_cast<ATTRIBUTES_TYPE>(attrib_id)).get_value();}
    Q_INVOKABLE int attribute_maximum(int attrib_id) {return (int)get_attribute(static_cast<ATTRIBUTES_TYPE>(attrib_id)).max();}
    Attribute get_attribute(ATTRIBUTES_TYPE id);

    //! return this dwarf's squad reference id
    Q_INVOKABLE int squad_id(bool original = false) { return (original ? m_squad_id : m_pending_squad_id);}
    Q_INVOKABLE int squad_position(bool original = false) { return (original ? m_squad_position : m_pending_squad_position);}
    Q_INVOKABLE int historical_id();
    HistFigure *hist_figure();

    void update_squad_info(int squad_id, int position, QString name);

    //! return this dwarf's caste id
    Q_INVOKABLE short get_caste_id() { return m_caste_id; }

    //! return this creature's race id
    Q_INVOKABLE short get_race_id() { return m_race_id; }

    //! return this creature's race
    Race* get_race() { return m_race; }

    //! return this creature's age
    Q_INVOKABLE int get_age() { return std::chrono::duration_cast<df_year>(m_age).count(); }
    Q_INVOKABLE int get_age_in_ticks() { return std::chrono::duration_cast<df_tick>(m_age).count(); }
    QString get_age_formatted();

    //! return this creature's flag1
    Q_INVOKABLE quint32 get_flag1() { return m_unit_flags.at(0); }

    //! return this creature's flag2
    Q_INVOKABLE quint32 get_flag2() { return m_unit_flags.at(1); }

    //! return this creature's flag2
    Q_INVOKABLE quint32 get_flag3() { return m_unit_flags.at(2); }

    //! return this creature's Nth bit from the start of flags1
    Q_INVOKABLE bool get_flag_value(int bit_pos);

    bool has_invalid_flags(QHash<uint, QString> invalid_flags, quint32 dwarf_flags);

    //! return this dwarf's highest skill
    Skill highest_skill();

    Q_INVOKABLE int rust_level() {return m_worst_rust_level;}

    //! sum total of all skill levels in any skill
    Q_INVOKABLE int total_skill_levels();

    //! number of activated labors
    Q_INVOKABLE int total_assigned_labors(bool include_skill_less);

    const QMap<int, ushort> get_labors() {return m_pending_labors;}

    void clear_labors();
    void assign_all_labors();
    void toggle_skilled_labors();
    void toggle_skilled_labor(int labor_id);

    //! return the sum total of all xp this dwarf has earned
    int total_xp() {return m_total_xp;}

    //! return the migration wave
    Q_INVOKABLE int migration_wave() const { return m_arrival_time.count(); }

    //! returns a description of birth or migration
    QString get_migration_desc();

    Q_INVOKABLE bool active_military() {return m_active_military;}

    //! return a hash of skill_id,Skill objects that this dwarf has experience in
    QHash<int, Skill> *get_skills() {return &m_skills;}
    QHash<int, Skill> get_moodable_skills() {return m_moodable_skills;}
    QVector<Attribute> *get_attributes() {return &m_attributes;}
    QHash<int, short> *get_traits(){return &m_traits;}
    void load_trait_values(QVector<double> &list);
    std::multimap<int, std::unique_ptr<Preference>> *get_preferences(){return &m_preferences;}

    double get_role_pref_match_counts(const Role *r, bool load_map = false);
    double get_role_pref_match_counts(const RolePreference *role_pref, const Role *r = 0);

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
    bool trait_is_conflicted(const int &trait_id);
    QList<UnitBelief> trait_conflicts(const int &trait_id){return m_conflicting_beliefs.values(trait_id);}

    //! returns the numeric rating for the this dwarf in the skill specified by skill_id
    float get_skill_level(int skill_id, bool raw = false, bool precise = false);
    //! convenience functions for skill level
    Q_INVOKABLE float skill_level(int skill_id);
    Q_INVOKABLE float skill_level_raw(int skill_id);
    Q_INVOKABLE float skill_level_precise(int skill_id);
    Q_INVOKABLE float skill_level_raw_precise(int skill_id);

    //! returns the numeric rating for the this dwarf in the skill associated with the labor specified by labor_id
    Q_INVOKABLE short labor_rating(int labor_id);

    //! return a hashmap of trait_id to trait score for this dwarf
    const QHash<int, short> &traits() {return m_traits;}
    QHash<int, short> &goals() {return m_goals;}

    bool belief_is_active(const int &belief_id);
    QHash<int, UnitBelief> &beliefs() {return m_beliefs;}
    UnitBelief get_unit_belief(int belief_id);
    Q_INVOKABLE int belief_value(int belief_id){return get_unit_belief(belief_id).belief_value();}

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

    // setters
    //! this will cause all data for this dwarf to be reset to game values (clears all pending uncomitted changes)
    void read_data();
    //! refresh only the data affected by committing or clearing pending changes
    void refresh_minimal_data();

    //! set the pending nickname for this dwarf (does not auto-commit)
    void set_nickname(const QString &nick);

    /*! manually set a labor as enabled or disabled for this dwarf. This method automatically unsets
    exclusive partners of a labor, or weapon choice. It also defends against cheating by not allowing
    labors to be set on certain professions (Baby, Child, Nobles, etc...)
    */
    void set_labor(int labor_id, bool enabled, bool update_cols_realtime=true);

    //! convenience method that calls set_labor() and switches the state of the labor specified by labor_id
    bool toggle_labor(int labor_id);

    //! toggle the n_bit bit of the flag
    bool toggle_flag_bit(int bit_pos);

    //! set's the pending custom profession text for this dwarf
    void set_custom_profession_text(const QString &prof_text);

    /*! sets the pending custom profession text for this dwarf as well as sets all labors for
    this dwarf to match the labor template of the CustomProfession object.
    */
    void apply_custom_profession(CustomProfession *cp);

    /*! clears the pending custom profession text for this dwarf. This will cause the dwarf to
    show up as their default profession in game
    */
    void reset_custom_profession(bool reset_labors = false);

    QList<double> calc_role_ratings();
    double calc_role_rating(Role *);
    Q_INVOKABLE float get_role_rating(QString role_name);
    Q_INVOKABLE float get_raw_role_rating(QString role_name);
    QList<QPair<QString,QString> > get_role_pref_matches(QString role_name){return m_role_pref_map.value(role_name);}
    void refresh_role_display_ratings();

    void calc_attribute_ratings();

    //! static method for mapping a value in the enum DWARF_HAPPINESS to a meaningful text string
    static QString happiness_name(DWARF_HAPPINESS happiness);

    Caste *get_caste() {return m_caste;}
    const Caste *get_caste() const {return m_caste;}

    //! method for mapping a caste id to a meaningful text name string
    Q_INVOKABLE QString caste_name(bool plural_name = false);
    Q_INVOKABLE QString caste_tag();
    Q_INVOKABLE QString caste_desc();
    Q_INVOKABLE QString race_name(bool base = true, bool plural_name = false);

    //! used for building a datamodel that shows all pending changes this dwarf has queued up
    QTreeWidgetItem *get_pending_changes_tree();
    void build_pending_flag_node(int index, QString title, UNIT_FLAGS flag, QTreeWidgetItem *parent);

    //! convenience hack allowing Dwarf objects to know where they live in the gridview model
    QModelIndex m_name_idx;

    //! get's a list of QActions that can be activated on this dwarf, suitable for adding to Toolbars or context menus
    QList<QAction*> get_mem_actions() {return m_actions_memory;}

    //! returns true if this dwarf can have labors specified on it
    Q_INVOKABLE bool can_set_labors() {return m_can_set_labors;}

    //birth year/time invokables
    Q_INVOKABLE quint32 get_birth_time() { return std::get<df_tick>(m_birth_date).count(); }
    Q_INVOKABLE quint32 get_birth_year() { return std::get<df_year>(m_birth_date).count(); }
    Q_INVOKABLE bool born_in_fortress() { return m_age.count() == m_turn_count; }

    QString first_name() const {
        return m_first_name;
    }

    QString squad_name();

    uint turn_count() const {
        return m_turn_count;
    }

    Q_INVOKABLE QString noble_position() {return m_noble_position;}

    QPixmap profession_icon() {return m_icn_prof;}
    QString gender_icon_path() {return m_icn_gender;}

    Q_INVOKABLE int body_size() const {return m_body_size;}
    Q_INVOKABLE int body_size_base() const {return m_body_size_base;}

    bool has_state(short id){return m_states.contains(id);}
    int state_value(short id){return m_states.value(id,-1);}

    Reaction *get_reaction();

    void find_fake_ident();

    bool had_mood() {return m_had_mood;}
    MOOD_TYPE current_mood() {return m_mood_id;}
    Q_INVOKABLE bool locked_in_mood() {return m_locked_mood;}
    Q_INVOKABLE bool in_stressed_mood() {return m_stressed_mood;}
    QString disabled_labor_reason() {return m_labor_reason;}
    QString artifact_name() {return m_artifact_name;}

    QHash<QString, QStringList*> get_grouped_preferences() {return m_grouped_preferences;}

    QList<UnitEmotion*> get_emotions() {return m_emotions;}

    Q_INVOKABLE bool has_preference(QString pref_name, QString category = "");
    Q_INVOKABLE bool find_preference(QString pref_name, QString category_name);
    Q_INVOKABLE bool has_thought(short id) {return m_thoughts.contains(id);}
    //! only used in scripts
    Q_INVOKABLE bool has_health_issue(int id, int idx);

    Q_INVOKABLE bool is_buffed();
    Q_INVOKABLE QString buffs();

    Q_INVOKABLE bool has_syndrome(QString name);
    Q_INVOKABLE QString syndromes();

    QString get_syndrome_names(bool include_buffs, bool include_sick);

    QString get_emotions_desc() {return m_emotions_desc;}

    UnitHealth get_unit_health() {return m_unit_health;}

    Q_INVOKABLE bool has_goal(int goal_id){return m_goals.contains(goal_id);}
    //! number of goals realized
    Q_INVOKABLE int goals_realized(){return m_goals_realized;}
    Q_INVOKABLE QString get_goal_summary() const;

    //! returns a list of items, grouped by body part name
    QHash<QString,QList<Item*> > get_inventory_grouped(){return m_inventory_grouped;}

    //! returns a percentage of the crucial clothing (shirt, shoes, pants)
    Q_INVOKABLE float get_coverage_rating(ITEM_TYPE itype = NONE);
    //! returns the number of items missing
    Q_INVOKABLE int get_missing_equip_count(ITEM_TYPE itype = NONE);
    //! returns a percentage of the inventory items / uniform items
    Q_INVOKABLE float get_uniform_rating(ITEM_TYPE itype = NONE);
    //! returns a percentage of how worn out items a dwarf is wearing
    Q_INVOKABLE int get_max_wear_level(ITEM_TYPE itype = NONE);
    QList<EquipWarn::warn_info> get_equip_warnings(){return m_equip_warnings;}

    Q_INVOKABLE bool can_assign_military() {return m_can_assign_military;}

    int optimized_labors;

    void set_global_sort_key(int group_id, QVariant val){m_global_sort_keys.insert(group_id,val);}
    QVariant get_global_sort_key(int group_id){return m_global_sort_keys.value(group_id,-1);}

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
        //! undo labor changes only
        void clear_pending_labors();
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
    QString m_happiness_desc; //happiness name + stress level
    int m_stress_level; // raw score before being turned into an enum
    MOOD_TYPE m_mood_id;
    bool m_had_mood;
    QString m_artifact_name;
    QString m_curse_name;
    short m_caste_id;
    bool m_show_full_name;
    int m_total_xp;
    df_time m_arrival_time;
    int m_body_size;
    int m_body_size_base;
    TRAINED_LEVEL m_animal_type;

    Q_PROPERTY(QString first_name READ first_name) // no setters (read-only)
    QString m_first_name; // set by game
    QString m_nick_name; // set by user
    QString m_pending_nick_name; // used when not committed yet
    QString m_last_name; // last name in dwarven
    QString m_translated_last_name; // last name in human english
    QString m_nice_name; // full name (depends on settings)
    QString m_translated_name; // full name using human english last name
    QString m_custom_prof_name; // set by user
    QString m_pending_custom_profession; // uncommitted
    QString m_prof_name; // name of profession set by game
    QPixmap m_icn_prof;
    QString m_icn_gender;
    int m_raw_prof_id; // id of profession set by game
    const Profession *m_raw_profession;
    bool m_can_set_labors; // used to prevent cheating
    bool m_locked_mood;
    bool m_stressed_mood;
    short m_current_job_id;
    QString m_current_job;
    QString m_current_sub_job_id;
    QHash<int,Skill> m_skills;
    QMultiMap<float, int> m_sorted_skills; //level, skill_id
    QHash<int,Skill> m_moodable_skills;
    QHash<int, short> m_traits;
    QHash<int, short> m_goals;
    QHash<int, UnitBelief> m_beliefs;
    QMultiHash<int, UnitBelief> m_conflicting_beliefs; //trait_id, conflicting belief_id(s)
    QVector<Attribute> m_attributes;
    QMap<int, ushort> m_labors;
    QMap<int, ushort> m_pending_labors;
    QList<QAction*> m_actions_memory; // actions suitable for context menus
    HistFigure *m_hist_figure;
    int m_squad_id;
    int m_squad_position;
    int m_pending_squad_id;
    int m_pending_squad_position;
    QString m_pending_squad_name; //The name of the squad that the dwarf belongs to (if any)
    QList<quint32> m_unit_flags;
    QList<quint32> m_pending_flags;
    uint m_turn_count; // Dwarf turn count from start of fortress (as best we know)
    QHash<QString, float> m_role_ratings;
    QHash<QString, double> m_raw_role_ratings;
    QList<Role::simple_rating> m_sorted_role_ratings;
    QList<QPair<QString,float> > m_sorted_custom_role_ratings;
    QHash<QString,QList<QPair<QString,QString> > > m_role_pref_map;
    QHash<short, int> m_states;
    std::tuple<df_year, df_tick> m_birth_date;
    df_time m_age;
    QString m_noble_position;
    bool m_is_pet;
    Race* m_race;
    Caste* m_caste;
    std::multimap<int, std::unique_ptr<Preference>> m_preferences;
    QHash<QString, QStringList*> m_grouped_preferences;
    QStringList m_pref_names;
    QString m_pref_tooltip;
    QList<short> m_thoughts;
    QList<UnitEmotion*> m_emotions;
    QString m_emotions_desc;
    bool m_is_animal;
    QString m_true_name; //used for vampires
    int m_true_birth_year; //used for vampires
    UnitHealth m_unit_health;
    bool m_is_valid;
    QList<Syndrome> m_syndromes;
    quint32 m_curse_flags;
    Uniform* m_uniform;
    int m_goals_realized;
    int m_worst_rust_level;
    QString m_labor_reason;
    int m_histfig_id;
    UNIT_OCCUPATION m_occ_type;
    bool m_can_assign_military;
    bool m_active_military;
    unit_gender m_gender_info;
    bool m_is_citizen;

    std::multimap<int, std::unique_ptr<UnitNeed>> m_needs;
    int m_current_focus;
    int m_undistracted_focus;
    int m_current_focus_degree;

    //! inventory grouped by body part /category
    QHash<QString,QList<Item*> > m_inventory_grouped;
    //! inventory coverage ratings
    QHash<ITEM_TYPE,int> m_coverage_ratings;
    //! worst inventory wear level by item type
    QHash<ITEM_TYPE,int> m_max_inventory_wear;
    //! counts of missing/worn items by key (itemname, wear_level)
    QList<EquipWarn::warn_info> m_equip_warnings;
    //! counts of missing uniform items
    QHash<ITEM_TYPE,int> m_missing_counts;

    //quint32 m_curse_flags2;
    eCurse::CURSE_TYPE m_curse_type;
    QHash<ATTRIBUTES_TYPE,QPair<float,int> > m_attribute_mods_perm; //permanent modification of attributes via syndromes (and curses?)
    QHash<ATTRIBUTES_TYPE,QPair<float,int> > m_attribute_mods_temp; //temporary modification of attributes via syndromes (and curses?)
    QHash<ATTRIBUTES_TYPE,QStringList> m_attribute_syndromes;

    QHash<int,QVariant> m_global_sort_keys;

    bool validate();

    // these methods read data from raw memory
    void read_id();
    void read_flags();
    void read_gender_orientation();
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
    void read_current_job();
    bool read_soul();
    void read_soul_aspects();
    void read_skills();
    void read_attributes();
    void load_attribute(VIRTADDR &addr, ATTRIBUTES_TYPE id);
    void read_personality();
    void read_emotions(VIRTADDR personality_base);
    void read_turn_count();
    void read_animal_type();
    void read_noble_position();
    void read_preferences();
    void read_syndromes();
    void read_squad_info();
    void read_inventory();
    void read_uniform();
    void check_availability();
    void set_validation(QString reason, bool *valid_var = 0, bool valid = false, LOG_LEVEL l = LL_INFO);

    void add_inv_warn(Item *i, bool include_mat_name, Item::ITEM_STATE i_status);
    void process_uncovered(ITEM_TYPE i_type, QString desc, int count, int req_count);
    void process_inv_item(QString category, Item *item, bool is_contained_item=false);

    void set_age_and_migration(VIRTADDR birth_year_offset, VIRTADDR birth_time_offset);

    // assembles component names into a nicely formatted single string
    void build_names();
    quint32 build_flag_mask(int bit);

    void recheck_equipment(int squad_id);

signals:
    void name_changed();
};

#endif // DWARF_H
