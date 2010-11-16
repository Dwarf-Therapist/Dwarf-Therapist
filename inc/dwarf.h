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

#include <QtGui>

#include "skill.h"
#include "utils.h"

class DFInstance;
class MemoryLayout;
class CustomProfession;

class Dwarf : public QObject
{
    Q_OBJECT
    friend class Squad;
    Dwarf(DFInstance *df, const uint &addr, QObject *parent=0); //private, use the static get_dwarf() method

public:
    static Dwarf* get_dwarf(DFInstance *df, const VIRTADDR &address);
    virtual ~Dwarf();

    typedef enum {
        DH_MISERABLE = 0,
        DH_VERY_UNHAPPY,
        DH_UNHAPPY,
        DH_FINE,
        DH_CONTENT,
        DH_HAPPY,
        DH_ECSTATIC,
        DH_TOTAL_LEVELS
    } DWARF_HAPPINESS;

    // getters
    //! Return the memory address (in hex) of this creature in the remote DF process
    VIRTADDR address() {return m_address;}

    //! return the the unique id for this creature
    int id() {return m_id;}

    //! true if the creature is male, false if female or "it"
    Q_INVOKABLE bool is_male() {return m_is_male;}

    //! return a text version of this dwarf's profession (will use custom profession if set)
    QString profession();

    //! return the raw game-set profession for a dwarf
    Q_INVOKABLE int raw_profession() {return m_raw_profession;}

    //! custom profession string (if set)
    QString custom_profession_name() {return m_pending_custom_profession;}

    //! return a printable name for this dwarf based on user-settings (may include nickname/firstname or both)
    Q_INVOKABLE QString nice_name() {return m_nice_name;}

    //! return a printable name for this dwarf where each dwarven word is translated to english (not game human)
    QString translated_name() {return m_translated_name;}

    //! return the string nickname for this dwarf (if set)
    QString nickname() {return m_pending_nick_name;}

    //! return the happiness level of this dwarf
    DWARF_HAPPINESS get_happiness() {return m_happiness;}

    //! return the raw happiness score for this dwarf
    Q_INVOKABLE int get_raw_happiness() {return m_raw_happiness;}

    //! return this dwarf's strength attribute score
    Q_INVOKABLE int strength() {return m_strength;}

    //! return this dwarf's agility attribute score
    Q_INVOKABLE int agility() {return m_agility;}

    //! return this dwarf's toughness attribute score
    Q_INVOKABLE int toughness() {return m_toughness;}

    //! return this dwarf's squad reference id
    Q_INVOKABLE int get_squad_ref_id() { return m_squad_ref_id; }

    //! return this dwarf's highest skill
    Skill highest_skill();

    //! sum total of all skill levels in any skill
    Q_INVOKABLE int total_skill_levels();

    //! number of activated labors
    Q_INVOKABLE int total_assigned_labors();

    //! return the sum total of all xp this dwarf has earned
    int total_xp() {return m_total_xp;}

    //! return the probable migration wave this dwarf arrived in (purely a guess)
    int migration_wave() {return m_migration_wave;}

    //! return true if the dwarf's raw_profession is a military professions
    Q_INVOKABLE bool active_military();

    //! return a vector of Skill objects that this dwarf has experience in
    QVector<Skill> *get_skills() {return &m_skills;}

    //! return a skill object by skill_id
    const Skill get_skill(int skill_id);

    //! return all labors that the user has toggled, but not comitted to DF yet
    QVector<int> get_dirty_labors(); // returns labor ids

    //! return true if the labor specified by labor_id is enabled or pending enabled
    Q_INVOKABLE bool labor_enabled(int labor_id);

    //! return true if the labor specified by labor_id has been toggled and not committed
    Q_INVOKABLE bool is_labor_state_dirty(int labor_id);

    //! return the numeric value of a preference setting (uses labor ids for offset)
    short pref_value(const int &labor_id);

    //! sets a numeric value on a preference to the next value in the chain (uses labor ids for offset)
    void toggle_pref_value(const int &labor_id);

    /*! return this dwarf's numeric score for the trait specified by trait_id,
    will return -1 if the trait is in the average range (non-extreme values)
    */
    Q_INVOKABLE short trait(int trait_id) {return m_traits.value(trait_id, -1);}

    //! returns the numeric rating for the this dwarf in the skill specified by skill_id
    short get_rating_by_skill(int skill_id);

    //! returns the numeric rating for the this dwarf in the skill associated with the labor specified by labor_id
    Q_INVOKABLE short get_rating_by_labor(int labor_id);

    //! return a hashmap of trait_id to trait score for this dwarf
    const QHash<int, short> &traits() {return m_traits;}

    //! return the text string describing what this dwarf is currently doing ("Idle", "Construct Rock Door" etc...)
    const QString &current_job() {return m_current_job;}

    //! return the id of the job this dwarf is currently doing
    const short &current_job_id() {return m_current_job_id;}

    //! return the total number of changes to this dwarf are uncommitted
    int pending_changes();

    //! return a formatted string suitable for showing in tooltips for this dwarf
    QString tooltip_text();

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
    void set_labor(int labor_id, bool enabled);

    //! convenience method that calls set_labor() and switches the state of the labor specified by labor_id
    bool toggle_labor(int labor_id);

    //! undo any uncomitted changes to this dwarf (reset back to game-state)
    void clear_pending();

    //! write all uncommitted pending changes back to the game (DANGEROUS METHOD)
    void commit_pending();

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

    //! static method for mapping a numeric happiness score into a value of the enum DWARF_HAPPINESS
    static DWARF_HAPPINESS happiness_from_score(int score);

    //! static method for mapping a value in the enum DWARF_HAPPINESS to a meaningful text string
    static QString happiness_name(DWARF_HAPPINESS happiness);

    //! used for building a datamodel that shows all pending changes this dwarf has queued up
    QTreeWidgetItem *get_pending_changes_tree();

    //! convenience hack allowing Dwarf objects to know where they live in the gridview model
    QModelIndex m_name_idx;

    //! get's a list of QActions that can be activated on this dwarf, suitable for adding to Toolbars or context menus
    QList<QAction*> get_actions() {return m_actions;}

    //! returns true if this dwarf can have labors specified on it
    Q_INVOKABLE bool can_set_labors() {return m_can_set_labors;}

    QString first_name() const {
        //qDebug() << "first_name called (from script?)";
        return m_first_name;
    }

    QString squad_name() const {
        return m_squad_name;
    }

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



private:
    int m_id; // each creature in the game has a unique serial ID
    DFInstance *m_df;
    MemoryLayout *m_mem;
    VIRTADDR m_address; // start of the structure in DF's memory space
    VIRTADDR m_first_soul; // start of 1st soul for this creature
    int m_race_id; // each creature has racial ID
    DWARF_HAPPINESS m_happiness; // enum value of happiness
    int m_raw_happiness; // raw score before being turned into an enum
    bool m_is_male;
    bool m_show_full_name;
    int m_total_xp;
    int m_migration_wave;
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
    int m_raw_profession; // id of profession set by game
    bool m_can_set_labors; // used to prevent cheating
    int m_strength;
    int m_agility;
    int m_toughness;
    short m_current_job_id;
    QString m_current_job;
    QVector<Skill> m_skills;
    QHash<int, short> m_traits;
    QMap<int, ushort> m_labors;
    QMap<int, ushort> m_pending_labors;
    QList<QAction*> m_actions; // actions suitable for context menus
    int m_squad_ref_id; //Dwarf reference that appears to be used by squad
    QString m_squad_name; //The name of the squad that the dwarf belongs to (if any)

    // these methods read data from raw memory
    void read_id();
    void read_caste();
    void read_race();
    void read_first_name();
    void read_last_name();
    void read_nick_name();
    void read_profession();
    void read_labors();
    void read_happiness();
    void read_current_job();
    void read_souls();
    void read_skills();
    void read_traits();
    void read_squad_ref_id();

    // utility methods to assist with reading names made up of several words
    // from the language tables
    QString word_chunk(uint word, bool use_generic=false);
    QString read_chunked_name(const VIRTADDR &addr, bool use_generic=false);
    QString read_squad_name(bool use_generic=false);

    // assembles component names into a nicely formatted single string
    void calc_names();

signals:
    void name_changed();
};

#endif // DWARF_H
