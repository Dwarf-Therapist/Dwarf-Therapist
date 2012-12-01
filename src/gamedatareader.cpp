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
#include <QtGui>
#include "gamedatareader.h"
#include "labor.h"
#include "trait.h"
#include "dwarfjob.h"
#include "profession.h"
#include "militarypreference.h"
#include "defines.h"
#include <QtDebug>
#include "raws/rawreader.h"
#include "math.h"
#include "laboroptimizerplan.h"



GameDataReader::GameDataReader(QObject *parent)
    : QObject(parent)
{
    QDir working_dir = QDir::current();
    QString filename = working_dir.absoluteFilePath("etc/game_data.ini");
    m_data_settings = new QSettings(filename, QSettings::IniFormat);

    QStringList labor_names;
    int labors = m_data_settings->beginReadArray("labors");
    for(int i = 0; i < labors; ++i) {
        m_data_settings->setArrayIndex(i);
        Labor *l = new Labor(*m_data_settings, this);
        m_labors.insert(l->labor_id, l);
        labor_names << l->name;
    }
    m_data_settings->endArray();
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

    int attributes = m_data_settings->beginReadArray("attributes");
    for(int i = 0; i < attributes; ++i) {
        m_data_settings->setArrayIndex(i);
        Attribute *a = new Attribute(*m_data_settings, this);
        m_attributes.insert(a->id, a);
    }
    m_data_settings->endArray();  
    QStringList attribute_names;
    foreach(Attribute *a, m_attributes) {
        attribute_names << a->name;
    }
    qSort(attribute_names);
    foreach(QString name, attribute_names) {
        foreach(Attribute *a, m_attributes) {
            if (a->name == name) {
                m_ordered_attributes << QPair<int, Attribute*>(a->id, a);
                break;
            }
        }
    }    

    m_data_settings->beginGroup("skill_names");
    foreach(QString k, m_data_settings->childKeys()) {
        int skill_id = k.toInt();
        m_skills.insert(skill_id, m_data_settings->value(k, "UNKNOWN").toString());
    }
    m_data_settings->endGroup();

    QStringList skill_names;
    foreach(QString name, m_skills) {
        skill_names << name;
    }
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
    m_moodable_skills << 28 << 37 << 49 << 2 << 16 << 3 << 30 << 31 <<
                         35 << 36 << 4 << 55 << 34 << 29 << 0 << 33 << 12 << 27 << 13 << 32;

    m_data_settings->beginGroup("attribute_levels");
    foreach(QString k, m_data_settings->childKeys()) {
        int num_attributes = k.toInt();
        m_attribute_levels.insert(num_attributes, m_data_settings->value(k).toInt());
    }
    m_data_settings->endGroup();

    int traits = m_data_settings->beginReadArray("traits");
    for(int i = 0; i < traits; i++) {
        m_data_settings->setArrayIndex(i);
        Trait *t = new Trait(i,*m_data_settings, this);
        m_traits.insert(i, t);
    }
    m_data_settings->endArray();
    QStringList trait_names;
    foreach(Trait *t, m_traits) {
        trait_names << t->name;
    }
    qSort(trait_names);
    foreach(QString name, trait_names) {
        foreach(Trait *t, m_traits) {
            if (t->name == name) {
                m_ordered_traits << QPair<int, Trait*>(t->trait_id, t);
                break;
            }
        }
    }

    int job_names = m_data_settings->beginReadArray("dwarf_jobs");
    m_dwarf_jobs.clear();
    for (short i = 0; i < job_names; ++i) {
        m_data_settings->setArrayIndex(i);

        QString name = m_data_settings->value("name", "???").toString();
        QString job_type = m_data_settings->value("type").toString();
        QString reactionClass = m_data_settings->value("reaction_class").toString();
        DwarfJob::DWARF_JOB_TYPE type = DwarfJob::get_type(job_type);

        TRACE << "Creating job(" << name << "," << type << "," << reactionClass << ")";

        m_dwarf_jobs[i + 1] =  new DwarfJob(i + 1, name, type, reactionClass, this);
    }
    m_data_settings->endArray();

    load_roles();
    load_optimization_plans();

    int professions = m_data_settings->beginReadArray("professions");
    m_professions.clear();
    for(short i = 0; i < professions; ++i) {
        m_data_settings->setArrayIndex(i);
        Profession *p = new Profession(*m_data_settings);
        m_professions.insert(p->id(), p);
    }
    m_data_settings->endArray();

    int mil_prefs = m_data_settings->beginReadArray("military_prefs");
    m_military_preferences.clear();
    for(short i = 0; i < mil_prefs; ++i) {
        m_data_settings->setArrayIndex(i);
        MilitaryPreference *p = new MilitaryPreference(*m_data_settings, this);
        m_military_preferences.insert(p->labor_id, p);
    }
    m_data_settings->endArray();    

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

//QString GameDataReader::get_attribute_level_name(Attribute::ATTRIBUTES_TYPE attribute, short id)
//{
//    //return m_attributes.value(attribute, 0)->m_levels.value(level);
//    return m_attributes.value(attribute,0)->m_levels.at(id).name;
//}

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

QString GameDataReader::get_skill_name(short skill_id) {
    return m_skills.value(skill_id, "UNKNOWN");
}

Role* GameDataReader::get_role(const QString &name) {
    return m_dwarf_roles.value(name,NULL);
}

Profession* GameDataReader::get_profession(const short &profession_id) {
    return m_professions.value(profession_id, 0);
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

QString GameDataReader::get_trait_name(short trait_id) {
    return get_trait(trait_id)->name;
}

laborOptimizerPlan* GameDataReader::get_opt_plan(const QString &name){
    return m_opt_plans.value(name);
}

DwarfJob *GameDataReader::get_job(const short &job_id) {
    return m_dwarf_jobs.value(job_id, 0);
}

MilitaryPreference *GameDataReader::get_military_preference(const int &mil_pref_id) {
    return m_military_preferences.value(mil_pref_id, 0);
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

    //load a mapping of skills to roles as well (used for showing roles in labor columns)
    m_skill_roles.clear();
    foreach(Role *r, m_dwarf_roles){
        QVector<Role*> roles;
        foreach(QString key, r->skills.uniqueKeys()){
            roles = m_skill_roles.value(key.toInt());
            roles.append(r);
            m_skill_roles.insert(key.toInt(),roles);
        }
    }
}

GameDataReader *GameDataReader::m_instance = 0;
