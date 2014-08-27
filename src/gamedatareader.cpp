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
#include <QtDebug>
#include "gamedatareader.h"
#include "labor.h"
#include "trait.h"
#include "attribute.h"
#include "dwarfjob.h"
#include "profession.h"
#include "defines.h"
#include "raws/rawreader.h"
#include "math.h"
#include "laboroptimizerplan.h"
#include "skill.h"
#include "thought.h"
#include "unithealth.h"
#include "belief.h"

QStringList GameDataReader::m_seasons;
QStringList GameDataReader::m_months;

GameDataReader::GameDataReader(QObject *parent)
    : QObject(parent)
    , m_data_settings(0)
{
    foreach(QString path, find_files_list("share", "game_data.ini")) {
        if (QFile::exists(path)) {
            LOGI << "Found game_data.ini:" << path;
            m_data_settings = new QSettings(path, QSettings::IniFormat);
            break;
        }
    }

    if (!m_data_settings) {
        FATAL << "Could not find game_data.ini.";
        qApp->exit(1);
    }

    build_calendar();

    QStringList labor_names;
    int labors = m_data_settings->beginReadArray("labors");
    for(int i = 0; i < labors; ++i) {
        m_data_settings->setArrayIndex(i);
        Labor *l = new Labor(*m_data_settings, this);
        m_labors.insert(l->labor_id, l);
        labor_names << l->name;
        m_skill_labors.insert(l->skill_id,l->labor_id);
    }
    m_data_settings->endArray();
    qDeleteAll(m_ordered_labors);
    m_ordered_labors.clear();
    qSort(labor_names);
    foreach(QString name, labor_names) {
        bool found = false;
        foreach(Labor *l, m_labors) {
            if (name == l->name) {
                m_ordered_labors << l;
                found = true;
                break;
            }
        }
        if (!found) {
            LOGW << tr("'%1' was not found in the labor map! Most likely, labor"
                       "ids are duplicated in game_data.ini");
        }
    }

    //load health category descriptors
    UnitHealth::load_health_descriptors(*m_data_settings);
    if(UnitHealth::get_display_categories().count() <= 0){
        LOGW << tr("Missing health information in game_data.ini!!");
    }

    //load up the list of attributes and their descriptors
    Attribute::load_attribute_descriptors(*m_data_settings);

    //load up some simple lists of the attributes and their names, as well as an ordered list
    int attributes = m_data_settings->beginReadArray("attributes");
    QStringList attribute_names;
    for(int i = 0; i < attributes; ++i) {
        m_data_settings->setArrayIndex(i);        
        int id = m_data_settings->value("id",0).toInt();
        QString name = m_data_settings->value("name","unknown").toString();
        m_attribute_names.insert(id,name);
        m_attributes_by_name.insert(name.toUpper(),static_cast<ATTRIBUTES_TYPE>(id));
        attribute_names << name;
    }
    m_data_settings->endArray();  

    qSort(attribute_names);
    foreach(QString sorted_name, attribute_names) {
        foreach(int id, m_attribute_names.uniqueKeys()) {
            if (m_attribute_names.value(id) == sorted_name) {
                m_ordered_attribute_names << QPair<int, QString>(id, m_attribute_names.value(id));
                break;
            }
        }
    }

    m_data_settings->beginGroup("attribute_levels");
    foreach(QString k, m_data_settings->childKeys()) {
        int num_attributes = k.toInt();
        m_attribute_levels.insert(num_attributes, m_data_settings->value(k).toInt());
    }
    m_data_settings->endGroup();

    m_data_settings->beginGroup("skill_names");
    QStringList skill_names;
    foreach(QString k, m_data_settings->childKeys()) {
        int skill_id = k.toInt();
        QString name = m_data_settings->value(k, "UNKNOWN").toString();
        m_skills.insert(skill_id, name);
        skill_names << name;
    }
    m_data_settings->endGroup();

    qSort(skill_names);
    foreach(QString name, skill_names) {
        foreach(int skill_id, m_skills.uniqueKeys()) {
            if (name == m_skills.value(skill_id)) {
                m_ordered_skills << QPair<int, QString>(skill_id, name);
                break;
            }
        }
    }        

    m_data_settings->beginGroup("skill_levels");
    foreach(QString k, m_data_settings->childKeys()) {
        int rating = k.toInt();
        m_skill_levels.insert(rating, m_data_settings->value(k, "UNKNOWN").toString());
    }
    m_data_settings->endGroup();

    //load moodable skills http://dwarffortresswiki.org/index.php/Mood#Skills_and_workshops
    m_moodable_skills << 0 << 2 << 3 << 4 << 12 << 13 << 16 << 27 << 28 << 29 << 30 << 31 <<
                         32 << 33 << 34 << 35 << 36 << 37 << 49 << 55;
    //load a map of skills to the above moodable professions key = skill_id, value = prof_id
    m_mood_skills_profession_map.insert(0,0);
    m_mood_skills_profession_map.insert(2,2);
    m_mood_skills_profession_map.insert(3,6);
    m_mood_skills_profession_map.insert(4,7);
    m_mood_skills_profession_map.insert(12,46);
    m_mood_skills_profession_map.insert(13,28);
    m_mood_skills_profession_map.insert(16,29);
    m_mood_skills_profession_map.insert(27,16);
    m_mood_skills_profession_map.insert(28,17);
    m_mood_skills_profession_map.insert(29,14);
    m_mood_skills_profession_map.insert(30,21);
    m_mood_skills_profession_map.insert(31,22);
    m_mood_skills_profession_map.insert(32,24);
    m_mood_skills_profession_map.insert(33,25);
    m_mood_skills_profession_map.insert(34,19);
    m_mood_skills_profession_map.insert(35,30);
    m_mood_skills_profession_map.insert(36,26);
    m_mood_skills_profession_map.insert(37,27);
    m_mood_skills_profession_map.insert(49,3);
    m_mood_skills_profession_map.insert(55,60);

    //facets
    refresh_facets();

    //goals
    int goal_count = m_data_settings->beginReadArray("goals");
    QStringList goal_names;
    for(int i = 0; i < goal_count; ++i) {
        m_data_settings->setArrayIndex(i);
        int id = m_data_settings->value("id",-1).toInt();
        QString name = m_data_settings->value("name","unknown").toString();
        QString desc = m_data_settings->value("desc","").toString();
        goal_names << name;
        m_goals.insert(id,qMakePair(name,desc));
    }
    m_data_settings->endArray();

    qSort(goal_names);
    foreach(QString name, goal_names) {
        foreach(int goal_id, m_goals.uniqueKeys()) {
            if (m_goals.value(goal_id).first == name) {
                m_ordered_goals << qMakePair(goal_id,name);
                break;
            }
        }
    }


    //beliefs
    int beliefs = m_data_settings->beginReadArray("beliefs");
    QStringList belief_names;
    for(int i = 0; i < beliefs; i++) {
        m_data_settings->setArrayIndex(i);
        Belief *b = new Belief(i,*m_data_settings, this);
        m_beliefs.insert(i, b);
        belief_names << b->name;
    }
    m_data_settings->endArray();
    qSort(belief_names);
    foreach(QString name, belief_names) {
        foreach(Belief *b, m_beliefs) {
            if (b->name == name) {
                m_ordered_beliefs << qMakePair(b->belief_id(),get_belief_name(b->belief_id()));
                break;
            }
        }
    }

    int job_count = m_data_settings->beginReadArray("dwarf_jobs");
    qDeleteAll(m_dwarf_jobs);
    m_dwarf_jobs.clear();
    QStringList job_names;
    //add custom jobs
    m_dwarf_jobs[-1] = new DwarfJob(-1,tr("Soldier"), DwarfJob::DJT_SOLDIER, "", this);
    job_names << m_dwarf_jobs[-1]->description;
    m_dwarf_jobs[-2] = new DwarfJob(-2,tr("On Break"), DwarfJob::DJT_ON_BREAK, "", this);
    job_names << m_dwarf_jobs[-2]->description;
    m_dwarf_jobs[-3] = new DwarfJob(-3,tr("No Job"), DwarfJob::DJT_IDLE, "", this);
    job_names << m_dwarf_jobs[-3]->description;
    for (short i = 0; i < job_count; ++i) {
        m_data_settings->setArrayIndex(i);

        QString name = m_data_settings->value("name", "???").toString();
        QString job_type = m_data_settings->value("type").toString();
        QString reactionClass = m_data_settings->value("reaction_class").toString();
        DwarfJob::DWARF_JOB_TYPE type = DwarfJob::get_type(job_type);

        m_dwarf_jobs[i] =  new DwarfJob(i, name, type, reactionClass, this);
        job_names << name;
    }
    m_data_settings->endArray();
    qSort(job_names);
    foreach(QString name, job_names) {
        foreach(DwarfJob *j, m_dwarf_jobs) {
            if (j->description == name) {
                m_ordered_jobs << QPair<int, QString>(j->id, name);
                break;
            }
        }
    }

    load_roles();
    load_optimization_plans();

    int professions = m_data_settings->beginReadArray("professions");
    qDeleteAll(m_professions);
    m_professions.clear();
    for(short i = 0; i < professions; ++i) {
        m_data_settings->setArrayIndex(i);
        Profession *p = new Profession(*m_data_settings);
        m_professions.insert(p->id(), p);
    }
    m_data_settings->endArray();

    //thoughts
    int t_count = m_data_settings->beginReadArray("unit_thoughts");
    m_unit_thoughts.clear();
    for(short i = 0; i < t_count; ++i) {
        m_data_settings->setArrayIndex(i);
        Thought *t = new Thought(i, *m_data_settings, this);
        m_unit_thoughts.insert(i,t);
    }
    m_data_settings->endArray();

}

GameDataReader::~GameDataReader(){
    delete m_instance;
    m_data_settings = 0;
}

int GameDataReader::get_int_for_key(QString key, short base) {
    if (!m_data_settings->contains(key)) {
        LOGE << tr("Couldn't find key '%1' in file '%2'").arg(key)
                .arg(m_data_settings->fileName());
    }
    bool ok;
    QString offset_str = m_data_settings->value(key, QVariant(-1)).toString();
    int val = offset_str.toInt(&ok, base);
    if (!ok) {
        LOGE << tr("Key '%1' could not be read as an integer in file '%2'")
                .arg(key).arg(m_data_settings->fileName());
    }
    return val;
}

QString GameDataReader::get_string_for_key(QString key) {
    if (!m_data_settings->contains(key)) {
        LOGE << tr("Couldn't find key '%1' in file '%2'").arg(key)
                .arg(m_data_settings->fileName());
    }
    return m_data_settings->value(key, QVariant("UNKNOWN")).toString();
}

QColor GameDataReader::get_color(QString key) {
    QString hex_code = get_string_for_key(key);
    bool ok;
    QColor c(hex_code.toInt(&ok, 16));
    if (!ok || !c.isValid())
        c = Qt::white;
    return c;
}

QString GameDataReader::get_skill_level_name(short level) {
    return m_skill_levels.value(level, "UNKNOWN");
    //return get_string_for_key(QString("skill_levels/%1").arg(level));
}

QString GameDataReader::get_skill_name(short skill_id, bool moodable) {
    QString name = m_skills.value(skill_id, "UNKNOWN");
    if(moodable && name == "UNKNOWN")
        name = "Craftsdwarf";

    return name;
}

Role* GameDataReader::get_role(const QString &name) {
    return m_dwarf_roles.value(name,NULL);
}

Profession* GameDataReader::get_profession(const short &profession_id) {
    return m_professions.value(profession_id, 0);
}

QString GameDataReader::get_goal_desc(int id, bool realized){
    QString desc = capitalize(m_goals.value(id).second);
    if(realized)
        desc.append(tr(", and this dream was realized"));
    return desc;
}

QStringList GameDataReader::get_child_groups(QString section) {
    m_data_settings->beginGroup(section);
    QStringList groups = m_data_settings->childGroups();
    m_data_settings->endGroup();
    return groups;
}

QStringList GameDataReader::get_keys(QString section) {
    m_data_settings->beginGroup(section);
    QStringList keys = m_data_settings->childKeys();
    m_data_settings->endGroup();
    return keys;
}

Labor *GameDataReader::get_labor(const int &labor_id) {
    return m_labors.value(labor_id, 0);
}

Trait *GameDataReader::get_trait(const int &trait_id) {
    return m_traits.value(trait_id, 0);
}

Belief *GameDataReader::get_belief(const int &belief_id) {
    return m_beliefs.value(belief_id, 0);
}

QString GameDataReader::get_trait_name(const short &trait_id) {
    return get_trait(trait_id)->name;
}

QString GameDataReader::get_belief_name(const int &belief_id) {
    return get_belief(belief_id)->name;
}

Thought *GameDataReader::get_thought(short id){
    if(!m_unit_thoughts.contains(id)){
        m_unit_thoughts.insert(id, new Thought(id, this));
    }
    return m_unit_thoughts.value(id);
}

laborOptimizerPlan* GameDataReader::get_opt_plan(const QString &name){
    return m_opt_plans.value(name);
}

DwarfJob *GameDataReader::get_job(const short &job_id) {
    return m_dwarf_jobs.value(job_id, 0);
}

int GameDataReader::get_xp_for_next_attribute_level(int current_number_of_attributes) {
    return m_attribute_levels.value(current_number_of_attributes + 1, 0); // return 0 if we don't know
}

int GameDataReader::get_level_from_xp(int xp) {
    int last_xp = 0;
    int ret_val = 0;
    QList<int> levels = m_attribute_levels.uniqueKeys();
    qStableSort(levels);
    foreach(int lvl, levels) {
        if (last_xp <= xp && xp <= m_attribute_levels.value(lvl, 0)) {
            ret_val = lvl - 1;
            break;
        }
    }
    return ret_val;
}

void GameDataReader::load_roles(){
    qDeleteAll(m_dwarf_roles);
    m_dwarf_roles.clear();
    m_default_roles.clear();
    //first add custom roles
    QSettings *u = DT->user_settings();
    int dwarf_roles = u->beginReadArray("custom_roles");
    for (short i = 0; i < dwarf_roles; ++i) {
        u->setArrayIndex(i);
        Role *r = new Role(*u, this);
        r->is_custom = true;
        m_dwarf_roles.insert(r->name,r);
    }
    u->endArray();

    dwarf_roles = m_data_settings->beginReadArray("dwarf_roles");
    for (short i = 0; i < dwarf_roles; ++i) {
        m_data_settings->setArrayIndex(i);
        Role *r = new Role(*m_data_settings, this);
        //keep a list of default roles to check custom roles against
        m_default_roles.append(r->name);
        //don't overwrite any custom role with the same name
        if(!m_dwarf_roles.contains(r->name))
            m_dwarf_roles.insert(r->name, r);
    }
    m_data_settings->endArray();

    load_role_mappings();
}

void GameDataReader::load_optimization_plans(){
    qDeleteAll(m_opt_plans);
    m_opt_plans.clear();

    //load labor optimization data
    QSettings *u = DT->user_settings();
    int labor_opts = u->beginReadArray("labor_optimizations");
    for(short i=0; i < labor_opts; i++){
        u->setArrayIndex(i);
        laborOptimizerPlan *lo = new laborOptimizerPlan(*u, this);
        m_opt_plans.insert(lo->name,lo);
    }
    u->endArray();

    refresh_opt_plans();
}

void GameDataReader::refresh_facets(){
    qDeleteAll(m_traits);
    m_traits.clear();
    m_ordered_traits.clear();

    int traits = m_data_settings->beginReadArray("facets");
    QStringList trait_names;
    for(int i = 0; i < traits; i++) {
        m_data_settings->setArrayIndex(i);
        Trait *t = new Trait(i,*m_data_settings, this);
        m_traits.insert(i, t);
        trait_names << t->name;
    }
    m_data_settings->endArray();

    qSort(trait_names);
    foreach(QString name, trait_names) {
        foreach(Trait *t, m_traits) {
            if (t->name == name) {
                m_ordered_traits << QPair<int, Trait*>(t->trait_id, t);
                break;
            }
        }
    }
}

void GameDataReader::refresh_opt_plans(){
    m_ordered_opts.clear();
    QStringList opt_names;
    foreach(laborOptimizerPlan *l, m_opt_plans) {
        opt_names << l->name.toUpper();
    }
    qSort(opt_names);
    foreach(QString name, opt_names) {
        foreach(laborOptimizerPlan *l, m_opt_plans) {
            if (l->name.toUpper() == name.toUpper()) {
                m_ordered_opts << QPair<QString, laborOptimizerPlan*>(l->name, l);
                break;
            }
        }
    }
}

void GameDataReader::load_role_mappings(){
    //load sorted role list    
    m_ordered_roles.clear();
    QStringList role_names;
    foreach(Role *r, m_dwarf_roles) {
        role_names << r->name.toUpper();
    }
    qSort(role_names);
    foreach(QString name, role_names) {
        foreach(Role *r, m_dwarf_roles) {
            if (r->name.toUpper() == name.toUpper()) {
                m_ordered_roles << QPair<QString, Role*>(r->name, r);
                break;
            }
        }
    }

    //load a mapping of skills to roles as well (used for showing roles in labor cell tooltips)
    //also load roles with which labors they use based on their skills (used to toggle labors in role cells)
    m_skill_roles.clear();
    int skill_id;
    foreach(Role *r, m_dwarf_roles){
        QVector<Role*> roles;
        QList<int> labors;
        foreach(QString key, r->skills.uniqueKeys()){
            skill_id = key.toInt();
            roles = m_skill_roles.value(skill_id);
            roles.append(r);
            m_skill_roles.insert(skill_id,roles);
            if(m_skill_labors.contains(skill_id))
                labors.append(m_skill_labors.value(skill_id));
        }
        r->set_labors(labors);
    }
}

void GameDataReader::build_calendar(){
    if(m_seasons.length()<=0 || m_months.length()<=0){
        m_seasons.append("Spring");
        m_seasons.append("Summer");
        m_seasons.append("Autumn");
        m_seasons.append("Winter");

        m_months.append("Granite");
        m_months.append("Slate");
        m_months.append("Felsite");
        m_months.append("Hematite");
        m_months.append("Malachite");
        m_months.append("Galena");
        m_months.append("Limestone");
        m_months.append("Sandstone");
        m_months.append("Timber");
        m_months.append("Moonstone");
        m_months.append("Opal");
        m_months.append("Obsidian");
    }
}

GameDataReader *GameDataReader::m_instance = 0;
