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

#include "global_enums.h"
#include "utils.h"
#include <QPointer>

// forward declaration
class QSettings;
class Labor;
class Role;
class laborOptimizerPlan;
class Trait;
class Profession;
class DwarfJob;
class Thought;
class SubThoughtTypes;
class Belief;
class Emotion;
class Mood;

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

    QList<Labor*> get_ordered_labors() {return m_ordered_labors;}
    QList<QPair<int, QPair<QString,QString> > > get_ordered_skills() {return m_ordered_skills;}
    QHash<int, Trait*> get_traits() {return m_traits;}
    QList<QPair<int, Trait*> > get_ordered_traits() {return m_ordered_traits;}
    QList<QPair<ATTRIBUTES_TYPE, QString> > get_ordered_attribute_names() {return m_ordered_attribute_names;}
    QHash<short, Profession*> get_professions() {return m_professions;}
    QHash<QString, Role*>& get_roles(){return m_dwarf_roles;}
    QList<QPair<QString, Role*> > get_ordered_roles() {return m_ordered_roles;}
    QVector<QString> get_default_roles() {return m_default_roles;}
    QHash<int,QVector<Role*> > get_skill_roles() {return m_skill_roles;}
    QHash<int,QPair<QString,QString> > get_skills(){return m_skills;}
    QList<QPair<int,QString> > get_ordered_beliefs(){return m_ordered_beliefs;}
    QList<QPair<int,QString> > get_ordered_goals(){return m_ordered_goals;}
    QString get_building_name(BUILDING_TYPE b_type, int value = -1);
    QString get_sphere_name(int idx);

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
    SubThoughtTypes *get_subthought_types(short id);
    Emotion *get_emotion(EMOTION_TYPE eType);

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
    QString get_skill_name(short skill_id, bool moodable = false, bool noun = false);
    int get_total_skill_count() {return m_skills.count();}
    int get_total_belief_count() {return m_beliefs.count();}
    int get_total_trait_count() {return m_trait_count;}

    const QVector<int> moodable_skills() {return m_moodable_skills;}
    int get_mood_skill_prof(int skill_id) const {return m_mood_skills_profession_map.value(skill_id,-1);}
    const QList<int> social_skills() {return m_social_skills;}

    QString get_goal_desc(int id, bool realized);
    QString get_goal_name(int id){return capitalize(m_goals.value(id).first);}

    Mood *get_mood(MOOD_TYPE);
    QString get_mood_name(MOOD_TYPE m_type,bool colored = false);
    QString get_mood_desc(MOOD_TYPE m_type,bool colored = false);

    QString get_knowledge_desc(int field, int topic);

    bool custom_roles_updated() {return m_cust_roles_updated;}
    void custom_roles_updated(bool val) {m_cust_roles_updated=val;}
    bool default_roles_updated() {return m_def_roles_updated;}
    void default_roles_updated(bool val) {m_def_roles_updated=val;}

    static QStringList m_seasons;
    static QStringList m_months;

protected:
    GameDataReader(QObject *parent = 0);
    virtual ~GameDataReader();
private:
    static GameDataReader *m_instance;
    QPointer<QSettings> m_data_settings;

    QHash<int, Labor*> m_labors;
    QList<Labor*> m_ordered_labors;

    QHash<int, Trait*> m_traits;
    QList<QPair<int, Trait*> > m_ordered_traits;
    int m_trait_count; //excluding custom traits

    QHash<int, Belief*> m_beliefs;
    QList<QPair<int,QString> > m_ordered_beliefs;

    QHash<int,QPair<QString,QString> > m_goals; //id key with pair name and desc
    QList<QPair<int,QString> > m_ordered_goals;

    QHash<int, QPair<QString,QString> > m_skills; //id key with name and noun
    QList<QPair<int, QPair<QString, QString> > > m_ordered_skills;
    QHash<int, QString> m_skill_levels;

    QHash<ATTRIBUTES_TYPE, QString> m_attribute_names;
    QHash<QString, ATTRIBUTES_TYPE> m_attributes_by_name;
    QList<QPair<ATTRIBUTES_TYPE,QString> > m_ordered_attribute_names;

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

    QVector<int> m_moodable_skills;
    QMap<int, int> m_mood_skills_profession_map;
    QList<int> m_social_skills;

    QMap<short, Thought*> m_unit_thoughts;
    QHash<int, SubThoughtTypes*> m_unit_subthought_types;
    QHash<int, Emotion*> m_unit_emotions;

    QHash<MOOD_TYPE,Mood*> m_unit_moods;

    QMap<int,QString> m_building_names;
    QMap<int,QString> m_building_quality;

    QList<QString> m_spheres;
    QHash<int,QMap<int,QString> > m_knowledge;

    bool m_cust_roles_updated;
    bool m_def_roles_updated;

    void build_calendar();
    void read_activity_section(QString section, int offset, QStringList *job_names);
};
#endif
