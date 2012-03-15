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
        m_attributes.insert(a->name, a);
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

    m_data_settings->beginGroup("attribute_levels");
    foreach(QString k, m_data_settings->childKeys()) {
        int num_attributes = k.toInt();
        m_attribute_levels.insert(num_attributes, m_data_settings->value(k).toInt());
    }
    m_data_settings->endGroup();

    int traits = m_data_settings->beginReadArray("traits");
    for(int i = 0; i < traits; i++) {
        m_data_settings->setArrayIndex(i);
        Trait *t = new Trait(*m_data_settings, this);
        t->trait_id = i;
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

    int dwarf_roles = m_data_settings->beginReadArray("dwarf_roles");
    m_dwarf_roles.clear();
    for (short i = 0; i < dwarf_roles; ++i) {
        m_data_settings->setArrayIndex(i);

        Role *r = new Role(*m_data_settings, this);
        m_dwarf_roles.insert(r->name, r);
    }
    m_data_settings->endArray();

    QStringList role_names;
    foreach(Role *r, m_dwarf_roles) {
        role_names << r->name;
    }

    qSort(role_names);
    foreach(QString name, role_names) {
        foreach(Role *r, m_dwarf_roles) {
            if (r->name == name) {
                m_ordered_roles << QPair<QString, Role*>(r->name, r);
                break;
            }
        }
    }

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

QString GameDataReader::get_attribute_level_name(QString attribute, short level)
{
    return m_attributes.value(attribute, 0)->m_levels.value(level);
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

QString GameDataReader::get_skill_name(short skill_id) {
    return m_skills.value(skill_id, "UNKNOWN");
    //return get_string_for_key(QString("skill_names/%1").arg(skill_id));
}

Role* GameDataReader::get_role(const QString &name) {
    return m_dwarf_roles.value(name, 0);
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

void GameDataReader::load_race_names()
{
    QHash<QString, QRawObjectList>::iterator i;
    for (i = m_creatures_classes.begin(); i != m_creatures_classes.end(); ++i)
    {
        foreach(RawObjectPtr obj, i.value()) {
            QString race = obj->get_value("NAME", "Unknown");
            if (race.size() > 1)
                race[0] = race[0].toUpper();
            m_race_names.insert(obj->get_id(), race);
        }
    }
}

QString GameDataReader::get_race_name(int race_id)
{
    return m_race_names[QString("%1").arg(race_id)];
}

void GameDataReader::load_caste_names()
{
    QHash<QString, QRawObjectList>::iterator i;
    bool found=false;
    for (i = m_creatures_classes.begin(); i != m_creatures_classes.end(); ++i)
    {
        foreach(RawObjectPtr obj, i.value()) {
            if (obj->get_value(1)=="DWARF")
            {
                //here you set the parent node to grab ie. CASTE, SELECT_CASTE
                QVector<RawNodePtr> subNodes = obj->get_children("CASTE");
                //LOGD << "Found " << subNodes.count() << " castes";
                int n_caste=0;                
                foreach (RawNodePtr subNode, subNodes)
                {                                        
                    QStringList casteInfo;
                    //casteInfo.append(subNode->get_value(0, QString("Caste %1").arg(n_caste)));
                    casteInfo.append("");
                    casteInfo.append("");

                    //here you look for the description of the cast in the child nodes, desc, CASTE_NAME, etc..
                    //or rather than do the above, we're going to use CASTE parent nodes, and use CASTE_NAME for the name, and ignore the description
                    //leaving the string array in case this changes
                    foreach (RawNodePtr child, subNode->get_children()){
                        if(child->get_name().toLower().contains("caste_name"))
                            casteInfo.replace(0,child->get_value(0,""));
                            //casteInfo.replace(1,child->get_value(0,""));
                    }
                    if(casteInfo.length() > 0 && casteInfo.at(0) != "ALL") //ignore
                    {
                        m_caste_names.insert(QString("%1").arg(n_caste), casteInfo);
                        //LOGD << "ADDING CASTE raw: " << subNode->get_value(0, QString("Caste %1").arg(n_caste)) << " with name: " << casteInfo.at(0) << " desc: " << casteInfo.at(1);
                        n_caste++;
                    }
                    //m_caste_names.insert(QString("%1").arg(n_caste), subNode->get_value(0, QString("Caste %1").arg(n_caste)).trimmed());
                    //n_caste++;
                }                
                found=true;
                break;
            }
            if (found)
                break;
        }
    }
}

//should be able to use dwarfstats for this now, but i'm leaving this code to be sure
//void GameDataReader::load_attributes_mean_value()
//{
//    //first we load some default value
//    QHash<int, int> mean;
//    int count = m_caste_names.count();
//    for (int i=0;i<count;i++)
//    {
//        mean.insert(Attribute::AT_STRENGTH,1250);
//        mean.insert(Attribute::AT_AGILITY,900);
//        mean.insert(Attribute::AT_TOUGHNESS,1250);
//        mean.insert(Attribute::AT_ENDURANCE,1000);
//        mean.insert(Attribute::AT_RECUPERATION,1000);
//        mean.insert(Attribute::AT_DISEASE_RESISTANCE,1000);

//        m_attributes_mean_value.insert(i,mean);
//        mean.clear();
//    }
//    //then we try to get the mean values from the raw
//    QHash<QString, QRawObjectList>::iterator j;
//    bool found=false;
//    for (j = m_creatures_classes.begin(); j != m_creatures_classes.end(); ++j)
//    {
//        foreach(RawObjectPtr obj, j.value()) {
//            if (obj->get_value(1)=="DWARF")
//            {
//                QVector<RawNodePtr> castes = obj->get_children("CASTE");
//                int n_caste=0;
//                foreach (RawNodePtr caste, castes)
//                {
//                    //redefine the default values so if there are no modification we still have a value
//                    mean.insert(Attribute::AT_STRENGTH,1250);
//                    mean.insert(Attribute::AT_AGILITY,900);
//                    mean.insert(Attribute::AT_TOUGHNESS,1250);
//                    mean.insert(Attribute::AT_ENDURANCE,1000);
//                    mean.insert(Attribute::AT_RECUPERATION,1000);
//                    mean.insert(Attribute::AT_DISEASE_RESISTANCE,1000);
//                    QVector<RawNodePtr> subNodes = caste->get_children("PHYS_ATT_RANGE");
//                    foreach (RawNodePtr subNode, subNodes)
//                    {
//                        QString attr_name = subNode->get_value(0);
//                        if(attr_name=="STRENGTH")
//                            mean.insert(Attribute::AT_STRENGTH,subNode->get_value(4).toInt());
//                        else if(attr_name=="AGILITY")
//                            mean.insert(Attribute::AT_AGILITY,subNode->get_value(4).toInt());
//                        else if(attr_name=="TOUGHNESS")
//                            mean.insert(Attribute::AT_TOUGHNESS,subNode->get_value(4).toInt());
//                        else if(attr_name=="ENDURANCE")
//                            mean.insert(Attribute::AT_ENDURANCE,subNode->get_value(4).toInt());
//                        else if(attr_name=="RECUPERATION")
//                            mean.insert(Attribute::AT_RECUPERATION,subNode->get_value(4).toInt());
//                        else if(attr_name=="DISEASE_RESISTANCE")
//                            mean.insert(Attribute::AT_DISEASE_RESISTANCE,subNode->get_value(4).toInt());
//                    }
//                    m_attributes_mean_value.insert(n_caste,mean);
//                    mean.clear();
//                    n_caste++;
//                }
//                found=true;
//                break;
//            }
//            if (found)
//                break;
//        }
//    }
//}


QString GameDataReader::get_caste_name(int caste_id)
{
    //return m_caste_names[QString("%1").arg(caste_id)].at(0);
    QStringList caste = m_caste_names[QString("%1").arg(caste_id)];
    if(caste.length() > 0){
        return caste.at(0);
    }else{
        return "";
    }
}
QString GameDataReader::get_caste_desc(int caste_id)
{
    //return m_caste_names[QString("%1").arg(caste_id)].at(1);
    QStringList caste = m_caste_names[QString("%1").arg(caste_id)];
    if(caste.length() > 1){
        return caste.at(1);
    }else{
        return "";
    }
}

void GameDataReader::read_raws(QDir df_dir) {
    //Read reactions
    QFileInfo reaction_other(df_dir, "raw/objects/reaction_other.txt");
    m_reaction_classes["reaction_other"] = RawReader::read_objects(reaction_other);
    LOGD << "Read " << m_reaction_classes["reaction_other"].size() << " reactions";
    QDir raw_dir = reaction_other.absoluteDir();
    raw_dir.setFilter( QDir::Files );
    QStringList filters;
    filters << "creature_*.txt";
    raw_dir.setNameFilters(filters);
    raw_dir.setSorting(QDir::Name);
    QFileInfoList infos = raw_dir.entryInfoList();

    int N = 0;
    for (QList<QFileInfo>::iterator info=infos.begin();info!=infos.end();info++)
    {
        m_creatures_classes[(*info).baseName()] = RawReader::read_creatures((*info), N);
        TRACE << "Read " << m_creatures_classes[(*info).baseName()].size() << " " << (*info).baseName();
        N+=m_creatures_classes[(*info).baseName()].size();
    }
    load_race_names();
    load_caste_names();
    //load_attributes_mean_value();
    //m_race_names = RawReader::read_races_names();
}


GameDataReader *GameDataReader::m_instance = 0;
