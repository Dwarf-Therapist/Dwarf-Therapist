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
#include "gamedatareader.h"
#include "attribute.h"
#include "belief.h"
#include "dwarfjob.h"
#include "emotion.h"
#include "labor.h"
#include "laboroptimizerplan.h"
#include "mood.h"
#include "profession.h"
#include "role.h"
#include "subthoughttypes.h"
#include "thought.h"
#include "trait.h"
#include "unithealth.h"

#include <QMessageBox>
#include <QFile>

QStringList GameDataReader::m_seasons;
QStringList GameDataReader::m_months;

GameDataReader::GameDataReader(QObject *parent)
    : QObject(parent)
{
    //load override game_data
    if (QFile::exists("share:game_data.ini")) {
        m_data_settings = QPointer<QSettings>(new QSettings("share:game_data.ini", QSettings::IniFormat));
        m_data_settings->setIniCodec("UTF-8");
        LOGI << "Found custom game_data.ini:" << m_data_settings->fileName();
    } else {
        //load default game_data
        m_data_settings = QPointer<QSettings>(new QSettings(":config/game_data", QSettings::IniFormat));
        m_data_settings->setIniCodec("UTF-8");
        if(m_data_settings->childGroups().count() <= 0){
            QString err = tr("Dwarf Therapist cannot run because game_data.ini could not be found!");
            QMessageBox::critical(0,tr("Missing File"),err);
            FATAL << err;
            exit(1);
        }
    }

    QStringList required_sections;
    required_sections << "labors" << "attributes" << "unit_jobs" << "goals" << "beliefs" << "unit_thoughts" << "facets" << "skills" << "skill_levels";
    foreach(QString key, required_sections){
        if(!m_data_settings->childGroups().contains(key)){
            QString err = tr("Dwarf Therapist cannot run because game_data.ini is missing [%1], a critical section!").arg(key);
            QMessageBox::critical(0,tr("Missing Section"),err);
            FATAL << err;
            break;
        }
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
        ATTRIBUTES_TYPE id = static_cast<ATTRIBUTES_TYPE>(m_data_settings->value("id",0).toInt());
        QString name = m_data_settings->value("name","unknown").toString();
        m_attribute_names.insert(id,name);
        m_attributes_by_name.insert(name.toUpper(),id);
        attribute_names << name;
    }
    m_data_settings->endArray();

    qSort(attribute_names);
    foreach(QString sorted_name, attribute_names) {
        foreach(ATTRIBUTES_TYPE id, m_attribute_names.uniqueKeys()) {
            if (m_attribute_names.value(id) == sorted_name) {
                m_ordered_attribute_names << QPair<ATTRIBUTES_TYPE, QString>(id, m_attribute_names.value(id));
                break;
            }
        }
    }

    int skill_count = m_data_settings->beginReadArray("skills");
    QStringList skills;
    for(int idx = 0; idx < skill_count; ++idx) {
        m_data_settings->setArrayIndex(idx);
        QString name = m_data_settings->value("name","unknown").toString();
        QString noun = m_data_settings->value("noun",name).toString();
        m_skills.emplace(idx, name, noun);

        //load moodable skills http://dwarffortresswiki.org/index.php/Mood#Skills_and_workshops
        bool mood = m_data_settings->value("mood",false).toBool();
        if(mood){
            m_moodable_skills << idx;
            int prof_id = m_data_settings->value("profession_id",-1).toInt();
            if(prof_id >= 0){
                m_mood_skills_profession_map.insert(idx,prof_id);
            }
        }
        //keep track of 'social' skills for UI options
        if(m_data_settings->value("social",false).toBool()){
            m_social_skills << idx;
        }
    }
    m_data_settings->endArray();

    m_name_ordered_skills.reserve(m_skills.size());
    m_noun_ordered_skills.reserve(m_skills.size());
    for (auto &skill: m_skills) {
        m_name_ordered_skills.push_back(&skill);
        m_noun_ordered_skills.push_back(&skill);
    }
    std::sort(m_name_ordered_skills.begin(), m_name_ordered_skills.end(),
              [] (const SkillInfo *s1, const SkillInfo *s2) { return s1->name < s2->name; });
    std::sort(m_noun_ordered_skills.begin(), m_noun_ordered_skills.end(),
              [] (const SkillInfo *s1, const SkillInfo *s2) { return s1->noun < s2->noun; });

    m_data_settings->beginGroup("skill_levels");
    foreach(QString k, m_data_settings->childKeys()) {
        int rating = k.toInt();
        m_skill_levels.insert(rating, m_data_settings->value(k, "UNKNOWN").toString());
    }
    m_data_settings->endGroup();

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

    //facets (after beliefs)
    refresh_facets();

    qDeleteAll(m_dwarf_jobs);
    m_dwarf_jobs.clear();
    QStringList job_names;
    read_activity_section("unit_jobs",0,&job_names);
    read_activity_section("unit_activities",DwarfJob::ACTIVITY_OFFSET,&job_names);
    read_activity_section("unit_orders",DwarfJob::ORDER_OFFSET,&job_names);

    qSort(job_names);
    foreach(QString name, job_names) {
        foreach(DwarfJob *j, m_dwarf_jobs) {
            if (j->name() == name) {
                m_ordered_jobs << QPair<int, QString>(j->id(), name);
                break;
            }
        }
    }
    m_dwarf_jobs.insert(DwarfJob::JOB_UNKNOWN,new DwarfJob());

    //moods
    int moods = m_data_settings->beginReadArray("unit_moods");
    m_unit_moods.insert(MT_NONE,new Mood(this));
    for(int i = 0; i < moods; ++i) {
        m_data_settings->setArrayIndex(i);
        m_unit_moods.insert(static_cast<MOOD_TYPE>(i),new Mood(*m_data_settings, this));
    }
    m_data_settings->endArray();

    load_roles();
    load_optimization_plans();

    int count = m_data_settings->beginReadArray("professions");
    qDeleteAll(m_professions);
    m_professions.clear();
    for(short i = 0; i < count; ++i) {
        m_data_settings->setArrayIndex(i);
        Profession *p = new Profession(*m_data_settings);
        m_professions.insert(p->id(), p);
    }
    m_data_settings->endArray();

    //sub-thoughts
    count = m_data_settings->beginReadArray("unit_subthoughts");
    m_unit_subthought_types.clear();
    for(short i = 0; i < count; ++i) {
        m_data_settings->setArrayIndex(i);
        m_unit_subthought_types.insert(i,new SubThoughtTypes(*m_data_settings, this));
    }
    m_data_settings->endArray();

    //thoughts
    count = m_data_settings->beginReadArray("unit_thoughts");
    m_unit_thoughts.clear();
    for(short i = 0; i < count; ++i) {
        m_data_settings->setArrayIndex(i);
        m_unit_thoughts.insert(i,new Thought(i, *m_data_settings, this));
    }
    m_data_settings->endArray();

    //emotions
    count = m_data_settings->beginReadArray("unit_emotions");
    m_unit_emotions.clear();
    for(short i = 0; i < count; ++i) {
        m_data_settings->setArrayIndex(i);
        m_unit_emotions.insert(i-1, new Emotion(i-1, *m_data_settings, this)); //start at -1
    }
    m_data_settings->endArray();

    m_building_names.insert(-1,"None");
    m_building_names.insert(0,"Seat");
    m_building_names.insert(1,"Bed");
    m_building_names.insert(2,"Table");
    m_building_names.insert(3,"Coffin");
    m_building_names.insert(4,"Farm Plot");
    m_building_names.insert(5,"Furnace");
    m_building_names.insert(6,"Trade Depot");
    m_building_names.insert(7,"Shop");
    m_building_names.insert(8,"Door");
    m_building_names.insert(9,"Floodgate");
    m_building_names.insert(10,"Chest");
    m_building_names.insert(11,"Weapon Rack");
    m_building_names.insert(12,"Armor Stand");
    m_building_names.insert(13,"Workshop");
    m_building_names.insert(14,"Cabinet");
    m_building_names.insert(15,"Statue");
    m_building_names.insert(16,"Glass Window");
    m_building_names.insert(17,"Gem Window");
    m_building_names.insert(18,"Well");
    m_building_names.insert(19,"Bridge");
    m_building_names.insert(20,"Dirt Road");
    m_building_names.insert(21,"Paved Road");
    m_building_names.insert(22,"Siege Engine");
    m_building_names.insert(23,"Trap");
    m_building_names.insert(24,"Animal Trap");
    m_building_names.insert(25,"Support");
    m_building_names.insert(26,"Archery Target");
    m_building_names.insert(27,"Chain");
    m_building_names.insert(28,"Cage");
    m_building_names.insert(29,"Stockpile");
    m_building_names.insert(30,"Civzone");
    m_building_names.insert(31,"Weapon");
    m_building_names.insert(32,"Wagon");
    m_building_names.insert(33,"Screw Pump");
    m_building_names.insert(34,"Construction");
    m_building_names.insert(35,"Hatch");
    m_building_names.insert(36,"Wall Grate");
    m_building_names.insert(37,"Floor Grate");
    m_building_names.insert(38,"Vertical Bars");
    m_building_names.insert(39,"Floor Bars");
    m_building_names.insert(40,"Gear Assembly");
    m_building_names.insert(41,"Horizontal Axle");
    m_building_names.insert(42,"Vertical Axle");
    m_building_names.insert(43,"Water Wheel");
    m_building_names.insert(44,"Windmill");
    m_building_names.insert(45,"Traction Bench");
    m_building_names.insert(46,"Slab");
    m_building_names.insert(47,"Nest");
    m_building_names.insert(48,"NestBox");
    m_building_names.insert(49,"Hive");
    m_building_names.insert(50,"Rollers");

    m_building_quality.insert(0,tr("fine"));
    m_building_quality.insert(1,tr("very fine"));
    m_building_quality.insert(2,tr("splendid"));
    m_building_quality.insert(3,tr("wonderful"));
    m_building_quality.insert(4,tr("completely sublime"));

    m_data_settings->beginGroup("sphere_names");
    foreach(QString id, m_data_settings->childKeys()) {
        m_spheres << m_data_settings->value(id, "UNKNOWN").toString();
    }
    qSort(m_spheres);
    m_data_settings->endGroup();

    count = m_data_settings->beginReadArray("knowledge");
    m_knowledge.clear();
    for(short i = 0; i < count; ++i) {
        m_data_settings->setArrayIndex(i);
        QString field = m_data_settings->value("field").toString();
        int topic_count = m_data_settings->beginReadArray("topics");
        QMap<int,QString> topics;
        for(int j = 0; j < topic_count; j++){
            m_data_settings->setArrayIndex(j);
            QString area = capitalizeEach(m_data_settings->value("area","??").toString());
            QString subject = capitalizeEach(m_data_settings->value("subject","??").toString());
            topics.insert(j,tr("<h4>%1 - %2</h4>Pondering %3")
                                  .arg(field).arg(area).arg(subject));
        }
        m_data_settings->endArray();

        m_knowledge.insert(i,topics);
    }
    m_data_settings->endArray();
}

//value here is the base value of the item/building
QString GameDataReader::get_building_name(BUILDING_TYPE b_type, int value){
    QString name = m_building_names.value(b_type,tr("building"));
    int key = value / 128;
    if(key > 4)
        key = 4;
    QString quality = m_building_quality.value(key,"");
    return QString(quality + " " + name).trimmed();
}

QString GameDataReader::get_sphere_name(int idx){
    if(idx >= 0 && idx < m_spheres.length()){
        return m_spheres.at(idx);
    }else{
        return tr("Unknown");
    }
}

GameDataReader::~GameDataReader(){
    delete m_instance;
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

QString GameDataReader::get_skill_level_name(short level) {
    return m_skill_levels.value(level, "UNKNOWN");
    //return get_string_for_key(QString("skill_levels/%1").arg(level));
}

QString GameDataReader::get_skill_name(short skill_id, bool noun) {
    auto it = m_skills.find(skill_id);
    if (it == m_skills.end()) {
        LOGE << "Skill" << skill_id << "not found";
        return "UNKNOWN";
    }
    return noun ? it->noun : it->name;
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

Mood *GameDataReader::get_mood(MOOD_TYPE m_type){
    if(!m_unit_moods.contains(m_type)){
        m_type = MT_NONE;
    }
    return m_unit_moods.value(m_type);
}
QString GameDataReader::get_mood_name(MOOD_TYPE m_type, bool colored){
    return get_mood(m_type)->get_mood_name(colored);
}
QString GameDataReader::get_mood_desc(MOOD_TYPE m_type, bool colored){
    return get_mood(m_type)->get_mood_desc(colored);
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
    return get_trait(trait_id)->get_name();
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

Emotion *GameDataReader::get_emotion(EMOTION_TYPE eType){
    if(!m_unit_emotions.contains(eType)){
        return m_unit_emotions.value(EM_NONE);
    }
    return m_unit_emotions.value(eType);
}

SubThoughtTypes *GameDataReader::get_subthought_types(short id){
    return m_unit_subthought_types.value(id);
}

laborOptimizerPlan* GameDataReader::get_opt_plan(const QString &name){
    return m_opt_plans.value(name);
}

DwarfJob *GameDataReader::get_job(const short &job_id) {
    if(m_dwarf_jobs.contains(job_id)){
        return m_dwarf_jobs.value(job_id);
    }else{
        LOGE << "Unknown jobID" << job_id;
        return m_dwarf_jobs.value((short)DwarfJob::JOB_UNKNOWN);
    }
}

void GameDataReader::load_roles(){
    m_cust_roles_updated = false;
    m_def_roles_updated = false;

    qDeleteAll(m_dwarf_roles);
    m_dwarf_roles.clear();
    m_default_roles.clear();
    //first add custom roles
    QSettings *u = DT->user_settings();
    int dwarf_roles = u->beginReadArray("custom_roles");
    for (short i = 0; i < dwarf_roles; ++i) {
        u->setArrayIndex(i);
        Role *r = new Role(*u, this);
        r->is_custom(true);
        if(r->updated()){
            LOGI << "custom role" << r->name() << "has been updated!";
            m_cust_roles_updated = true;
        }
        m_dwarf_roles.insert(r->name(),r);
    }
    u->endArray();

    dwarf_roles = m_data_settings->beginReadArray("dwarf_roles");
    for (short i = 0; i < dwarf_roles; ++i) {
        m_data_settings->setArrayIndex(i);
        Role *r = new Role(*m_data_settings, this);
        if(r->updated()){
            LOGI << "default role" << r->name() << "requires updating!";
            m_def_roles_updated = true;
        }
        //keep a list of default roles to check custom roles against
        m_default_roles.append(r->name());
        //don't overwrite any custom role with the same name
        if(!m_dwarf_roles.contains(r->name()))
            m_dwarf_roles.insert(r->name(), r);
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
    m_trait_count = 0;

    int traits = m_data_settings->beginReadArray("facets");
    QStringList trait_names;
    for(int trait_id = 0; trait_id < traits; trait_id++) {
        m_data_settings->setArrayIndex(trait_id);
        Trait *t = new Trait(trait_id,*m_data_settings, this);
        m_traits.insert(t->id(), t);

        foreach(int belief_id, t->get_conflicting_beliefs()){
            if(m_beliefs.contains(belief_id))
                m_beliefs[belief_id]->add_conflict(trait_id);
        }

        trait_names << t->get_name();
        if(t->id() >= 0){
            m_trait_count++;
        }
    }
    m_data_settings->endArray();

    qSort(trait_names);
    foreach(QString name, trait_names) {
        foreach(Trait *t, m_traits) {
            if (t->get_name() == name) {
                m_ordered_traits << QPair<int, Trait*>(t->id(), t);
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
        role_names << r->name().toUpper();
    }
    qSort(role_names);
    foreach(QString name, role_names) {
        foreach(Role *r, m_dwarf_roles) {
            if (r->name().toUpper() == name.toUpper()) {
                m_ordered_roles << QPair<QString, Role*>(r->name(), r);
                break;
            }
        }
    }

    //load a mapping of skills to roles as well (used for showing roles in labor cell tooltips)
    //also load roles with which labors they use based on their skills (used to toggle labors in role cells)
    m_skill_roles.clear();
    foreach(Role *r, m_dwarf_roles){
        QVector<Role*> roles;
        QList<int> labors;
        for (const auto &p: r->skills){
            int skill_id = p.first.toInt();
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
        m_seasons.append(tr("Spring"));
        m_seasons.append(tr("Summer"));
        m_seasons.append(tr("Autumn"));
        m_seasons.append(tr("Winter"));

        m_months.append(tr("Granite"));
        m_months.append(tr("Slate"));
        m_months.append(tr("Felsite"));
        m_months.append(tr("Hematite"));
        m_months.append(tr("Malachite"));
        m_months.append(tr("Galena"));
        m_months.append(tr("Limestone"));
        m_months.append(tr("Sandstone"));
        m_months.append(tr("Timber"));
        m_months.append(tr("Moonstone"));
        m_months.append(tr("Opal"));
        m_months.append(tr("Obsidian"));
    }
}

void GameDataReader::read_activity_section(QString section, int offset, QStringList *job_names){
    int job_count = m_data_settings->beginReadArray(section);
    for(int idx = 0; idx < job_count; ++idx){
        m_data_settings->setArrayIndex(idx);

        QString group_name = m_data_settings->value("name","").toString();
        QString img_path = m_data_settings->value("img","control-play-blue").toString();
        bool is_military = m_data_settings->value("is_military",false).toBool();

        if(m_data_settings->childGroups().contains("sub")){
            int sub_jobs = m_data_settings->beginReadArray("sub");
            for(int sub_idx = 0; sub_idx < sub_jobs; ++sub_idx){
                m_data_settings->setArrayIndex(sub_idx);
                DwarfJob *j = new DwarfJob(*m_data_settings,offset,group_name,img_path,is_military,this);
                if(m_dwarf_jobs.contains(j->id())){
                    LOGW << "duplicate job!" << j->name() << j->id();
                }else{
                    m_dwarf_jobs.insert(j->id(),j);
                }
                job_names->append(j->name());
            }
            m_data_settings->endArray();
        }else{
            DwarfJob *j = new DwarfJob(*m_data_settings,offset,"","",is_military,this);
            if(m_dwarf_jobs.contains(j->id())){
                LOGW << "duplicate job!" << j->name() << j->id();
            }else{
                m_dwarf_jobs.insert(j->id(),j);
            }
            job_names->append(j->name());
        }
    }
    m_data_settings->endArray();
}

QString GameDataReader::get_knowledge_desc(int field, int topic){
    if(m_knowledge.contains(field)){
        return m_knowledge.value(field).value(topic,"Unknown");
    }
    return "Unknown";
}

GameDataReader *GameDataReader::m_instance = 0;
