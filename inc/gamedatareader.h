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
#ifndef GAME_DATA_READER_H
#define GAME_DATA_READER_H

#include <string>
#include <stdexcept>
#include <QtCore>
#include "raws/rawobjectlist.h"
#include "global_enums.h"
#include "utils.h"

// forward declaration
class QSettings;
class Labor;
class Attribute;
class Role;
class laborOptimizerPlan;
class Trait;
class MilitaryPreference;
class Profession;
class DwarfJob;
class Thought;
class Belief;

// exceptions
class MissingValueException : public std::runtime_error {
public:
    MissingValueException(const std::string &msg) : runtime_error(msg) {}
};

class CorruptedValueException : public std::runtime_error {
public:
    CorruptedValueException(const std::string &msg) : runtime_error(msg) {}
};

//singleton reader of game data
class GameDataReader : public QObject {
    Q_OBJECT
public:
    static GameDataReader *ptr() {
        if (!m_instance) {
            m_instance = new GameDataReader(0);
        }
        return m_instance;
    }

    int get_int_for_key(QString key, short base = 16);
    int get_address(QString key) {return get_int_for_key("addresses/" + key);}
    int get_offset(QString key) {return get_int_for_key("offsets/" + key);}
    int get_dwarf_offset(QString key) {return get_int_for_key("dwarf_offsets/" + key);}
    int get_xp_for_next_attribute_level(int current_number_of_attributes);

    QList<Labor*> get_ordered_labors() {return m_ordered_labors;}
    QList<QPair<int, QString> > get_ordered_skills() {return m_ordered_skills;}
    QHash<int, Trait*> get_traits() {return m_traits;}
    QList<QPair<int, Trait*> > get_ordered_traits() {return m_ordered_traits;}
    QList<QPair<int, QString> > get_ordered_attribute_names() {return m_ordered_attribute_names;}    
    QHash<short, Profession*> get_professions() {return m_professions;}    
    QHash<QString, Role*>& get_roles(){return m_dwarf_roles;}
    QList<QPair<QString, Role*> > get_ordered_roles() {return m_ordered_roles;}
    QVector<QString> get_default_roles() {return m_default_roles;}
    QHash<int,QVector<Role*> > get_skill_roles() {return m_skill_roles;}    
    QHash<int,QString> get_skills(){return m_skills;}
    QList<QPair<int,QString> > get_ordered_beliefs(){return m_ordered_beliefs;}
    QList<QPair<int,QString> > get_ordered_goals(){return m_ordered_goals;}

    QList<QPair<QString, laborOptimizerPlan*> > get_ordered_opt_plans() {return m_ordered_opts;}
    QHash<QString, laborOptimizerPlan*>& get_opt_plans(){return m_opt_plans;}
    laborOptimizerPlan *get_opt_plan(const QString &name);

    Labor *get_labor(const int &labor_id);
    Trait *get_trait(const int &trait_id);
    QString get_trait_name(const short &trait_id);
    Belief *get_belief(const int &belief_id);
    QString get_belief_name(const int &belief_id);

    QMap<short, Thought*> get_thoughts(){return m_unit_thoughts;}
    Thought *get_thought(short id);

    DwarfJob *get_job(const short &job_id);
    QList<QPair<int, QString> > get_ordered_jobs() {return m_ordered_jobs;}

    Role *get_role(const QString &name);

    void load_roles();
    void load_role_mappings();
    void load_optimization_plans();
    void refresh_opt_plans();
    void refresh_facets();

    QString get_attribute_name(ATTRIBUTES_TYPE id){return m_attribute_names.value(id);}
    QHash<ATTRIBUTES_TYPE,QString> get_attributes(){return m_attribute_names;}
    ATTRIBUTES_TYPE get_attribute_type(QString name){return m_attributes_by_name.value(name);}

    QString get_string_for_key(QString key);
    Profession* get_profession(const short &profession_id);
    QString get_skill_level_name(short level);
    QString get_skill_name(short skill_id, bool moodable = false);
    int get_total_skill_count() {return m_skills.count();}
    int get_total_belief_count() {return m_beliefs.count();}

    QColor get_color(QString key);

    QStringList get_child_groups(QString section);
    QStringList get_keys(QString section);
    int get_level_from_xp(int xp);

    RawObjectPtr get_reaction(QString reactionClass, QString id) {
        if(m_reaction_classes.contains(reactionClass)) {
            return m_reaction_classes.value(reactionClass)
                    .getRawObject("REACTION", id);
        }
        return RawObjectPtr();
    }

    RawObjectPtr get_creature(QString creatureClass, QString id) {
        if(m_creatures_classes.contains(creatureClass)) {
            return m_creatures_classes.value(creatureClass)
                    .getRawObject("CREATURE", id);
        }
        return RawObjectPtr();
    }

    const QVector<int> moodable_skills() {return m_moodable_skills;}
    int get_pref_from_skill(int skill_id) const {return m_mood_skills_profession_map.value(skill_id,-1);}

    QString get_goal_desc(int id, bool realized);
    QString get_goal_name(int id){return capitalize(m_goals.value(id).first);}

    static QStringList m_seasons;
    static QStringList m_months;

protected:
    GameDataReader(QObject *parent = 0);
    virtual ~GameDataReader();
private:
    static GameDataReader *m_instance;
    QSettings *m_data_settings;

    QHash<int, Labor*> m_labors;
    QList<Labor*> m_ordered_labors;

    QHash<int, Trait*> m_traits;
    QList<QPair<int, Trait*> > m_ordered_traits;

    QHash<int, Belief*> m_beliefs;
    QList<QPair<int,QString> > m_ordered_beliefs;

    QHash<int,QPair<QString,QString> > m_goals; //id key with pair name and desc
    QList<QPair<int,QString> > m_ordered_goals;

    QHash<int, QString> m_skills;
    QList<QPair<int, QString> > m_ordered_skills;    
    QHash<int, QString> m_skill_levels;

    QHash<int, int> m_attribute_levels;
    QHash<ATTRIBUTES_TYPE, QString> m_attribute_names;
    QHash<QString, ATTRIBUTES_TYPE> m_attributes_by_name;
    QList<QPair<int,QString> > m_ordered_attribute_names;

    QHash<short, DwarfJob*> m_dwarf_jobs;
    QList<QPair<int, QString> > m_ordered_jobs;

    QHash<short, Profession*> m_professions;

    QHash<QString, Role*> m_dwarf_roles;
    QList<QPair<QString, Role*> > m_ordered_roles;
    QVector<QString> m_default_roles;
    QHash<int,QVector<Role*> > m_skill_roles;
    QHash<int,int> m_skill_labors; //mapping of skills to labors

    QHash<QString, laborOptimizerPlan*> m_opt_plans;
    QList<QPair<QString, laborOptimizerPlan*> > m_ordered_opts;

    QHash<QString, QRawObjectList> m_reaction_classes;
    QHash<QString, QRawObjectList> m_creatures_classes;    

    QHash<QString, QString> m_race_names;
    QHash<QString, QStringList> m_caste_names;

    QVector<int> m_moodable_skills;
    QMap<int, int> m_mood_skills_profession_map;

    QMap<short, Thought*> m_unit_thoughts;

    void load_race_names();
    void load_caste_names();
    void build_calendar();
};
#endif
