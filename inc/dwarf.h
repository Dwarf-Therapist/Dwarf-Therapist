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

class DFInstance;
class CustomProfession;

class Dwarf : public QObject
{
    Q_OBJECT
    Dwarf(DFInstance *df, const uint &addr, QObject *parent=0); //private, use the static get_dwarf() method

public:
    static Dwarf* get_dwarf(DFInstance *df, const uint &address);
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
    uint address() {return m_address;}

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
    void set_nickname(QString nick) {m_pending_nick_name = nick; calc_names();}

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

    /************************************************************************/
    /* SQUAD STUFF                                                          */
    /************************************************************************/
    //! return a pointer to this dwarf's squad leader, or 0 if no squad leader is set
    Dwarf *get_squad_leader();

    /*! return the dwarf string name of this dwarf's squad
    If this dwarf is not a squad leader but still in a squad, this method will return the
    name of the squad if this dwarf were squad leader. If you want the name of the squad
    this dwarf is a follower of, then get the squad name from this dwarf's squad_leader

    /sa: get_squad_leader()
    */
    QString squad_name();

    //! add a dwarf to this dwarf's squad
    void add_squad_member(Dwarf *d);

    QList<Dwarf*> squad_members() {return m_squad_members;}

    QString first_name() const {
        //qDebug() << "first_name called (from script?)";
        return m_first_name;
    }

    public slots:
        //! called when global user settings change
        void read_settings();
        //! show a dialog with a memory dump for this dwarf...
        void dump_memory();
        //! show details for this dwarf in a new window...
        void show_details();



private:
    int m_id;
    DFInstance *m_df;
    uint m_address;
    int m_race_id;
    DWARF_HAPPINESS m_happiness;
    int m_raw_happiness;
    int m_money;
    bool m_is_male;
    bool m_show_full_name;
    int m_total_xp;
    int m_migration_wave;

    QString read_profession(const uint &addr);
    QString read_last_name(const uint &addr, bool use_generic=false);
    QString read_squad_name(const uint &addr, bool use_generic=false);
    QVector<Skill> read_skills(const uint &addr);
    void read_prefs(const uint &addr);
    void read_labors(const uint &addr);
    void calc_names();
    void read_traits(const uint &addr);
    void read_current_job(const uint &addr);


    Q_PROPERTY(QString first_name READ first_name) // no setters (scripting read-only)
    QString m_first_name;
    QString m_last_name, m_translated_last_name;
    QString m_nick_name, m_pending_nick_name;
    QString m_nice_name, m_translated_name; // used to cache this value
    QString m_custom_profession, m_pending_custom_profession;
    QString m_profession;
    int m_raw_profession;
    bool m_can_set_labors;
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

    // Squad settings
    int m_squad_leader_id;
    QString m_squad_name;
    QString m_generic_squad_name;
    //! maintain a list of all dwarfs who follow this dwarf in the military hierarchy
    QList<Dwarf*> m_squad_members;

signals:
    void name_changed();
};

#endif // DWARF_H
