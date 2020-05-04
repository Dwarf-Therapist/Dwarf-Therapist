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

#include "role.h"
#include "defaultroleweight.h"
#include "dwarftherapist.h"
#include "gamedatareader.h"
#include "rolepreference.h"
#include "defines.h"
#include "dwarf.h"
#include "material.h"
#include "item.h"
#include "standardpaths.h"

#include <QSettings>
#include <QRegularExpression>

Role::weight_info::weight_info(const DefaultRoleWeight &default_value)
    : m_default_value(default_value)
    , m_is_default(true)
    , m_weight(1.0f)
{
}

float Role::weight_info::weight() const
{
    return m_is_default ? m_default_value.value() : m_weight;
}

float Role::weight_info::default_weight() const
{
    return m_default_value.value();
}

void Role::weight_info::reset_to_default()
{
    m_is_default = true;
}

void Role::weight_info::set(float w)
{
    m_is_default = false;
    m_weight = w;
}

Role::Role(QObject *parent)
    : QObject(parent)
    , attributes_weight(DefaultRoleWeight::attributes)
    , skills_weight(DefaultRoleWeight::skills)
    , facets_weight(DefaultRoleWeight::facets)
    , beliefs_weight(DefaultRoleWeight::beliefs)
    , goals_weight(DefaultRoleWeight::goals)
    , needs_weight(DefaultRoleWeight::needs)
    , prefs_weight(DefaultRoleWeight::preferences)
    , m_name("UNKNOWN")
    , m_script("")
    , m_is_custom(false)
    , m_cur_pref_len(0)
    , m_updated(false)
{
    attributes_weight.reset_to_default();
    skills_weight.reset_to_default();
    facets_weight.reset_to_default();
    beliefs_weight.reset_to_default();
    goals_weight.reset_to_default();
    needs_weight.reset_to_default();
    prefs_weight.reset_to_default();
}

Role::Role(QSettings &s, QObject *parent)
    : QObject(parent)
    , attributes_weight(DefaultRoleWeight::attributes)
    , skills_weight(DefaultRoleWeight::skills)
    , facets_weight(DefaultRoleWeight::facets)
    , beliefs_weight(DefaultRoleWeight::beliefs)
    , goals_weight(DefaultRoleWeight::goals)
    , needs_weight(DefaultRoleWeight::needs)
    , prefs_weight(DefaultRoleWeight::preferences)
    , m_name(s.value("name", "UNKNOWN ROLE").toString())
    , m_script(s.value("script","").toString())
    , m_is_custom(false)
    , m_cur_pref_len(0)
    , m_updated(false)
{
    if (s.contains("prefs_weight")) {
        // update preference weight field name
        auto w = s.value("prefs_weight", -1).toFloat();
        s.remove("prefs_weight");
        s.setValue("preferences_weight", w);
        LOGD << "update role key: rename prefs_weight";
        m_updated = true;
    }

    parseAspect(s, "attributes", attributes_weight, attributes);
    parseAspect(s, "traits", facets_weight, facets);
    parseAspect(s, "beliefs", beliefs_weight, beliefs);
    parseAspect(s, "goals", goals_weight, goals);
    parseAspect(s, "needs", needs_weight, needs);
    parseAspect(s, "skills", skills_weight, skills);
    parseAspect(s, "preferences", prefs_weight, prefs);
}

Role::Role(const Role &r, QObject *parent)
    : QObject(parent)
    , attributes(r.attributes)
    , skills(r.skills)
    , facets(r.facets)
    , beliefs(r.beliefs)
    , goals(r.goals)
    , needs(r.needs)
    , attributes_weight(r.attributes_weight)
    , skills_weight(r.skills_weight)
    , facets_weight(r.facets_weight)
    , beliefs_weight(r.beliefs_weight)
    , goals_weight(r.goals_weight)
    , needs_weight(r.needs_weight)
    , prefs_weight(r.prefs_weight)
    , m_name(r.m_name)
    , m_script(r.m_script)
    , m_is_custom(r.m_is_custom)
    , role_details(r.role_details)
    , m_cur_pref_len(0)
    , m_updated(false)
{
    prefs.reserve(r.prefs.size());
    for (const auto &p: r.prefs)
        prefs.emplace_back(p.first->copy(), p.second);
}

Role::~Role(){
}

template<typename T>
static T parse_object(QSettings &s, bool &)
{
    return qvariant_cast<T>(s.value("id", T()));
}

template<>
std::unique_ptr<RolePreference> parse_object<std::unique_ptr<RolePreference>> (QSettings &s, bool &updated)
{
    return RolePreference::parse(s, updated);
}

template<typename T>
void Role::parseAspect(QSettings &s, QString node, weight_info &g_weight, std::vector<std::pair<T,aspect_weight>> &list)
{
    list.clear();

    //keep track of whether or not we're using the global default when we redraw if options are changed
    float weight = s.value(QString("%1_weight").arg(node),-1).toFloat();
    if (weight < 0.0f)
        g_weight.reset_to_default();
    else
        g_weight.set(weight);

    int count = s.beginReadArray(node);

    for (int i = 0; i < count; ++i) {
        s.setArrayIndex(i);

        if (!s.contains("is_neg")) {
            // update old id/name containing dash for negative aspect
            QString key;
            if (s.contains("id"))
                key = "id";
            else if (s.contains("name")) // preferences used name field
                key = "name";
            auto id = s.value(key).toString();
            if(id.indexOf("-") >= 0){
                id.replace("-","");
                s.setValue(key, id);
                s.setValue("is_neg", true);
                LOGD << "update role key: use is_neg";
                m_updated = true;
            }
        }

        // Read weight info
        aspect_weight w;
        w.weight = s.value("weight", 1.0).toFloat();
        if (w.weight < 0)
            w.weight = 1.0f;
        w.is_neg = s.value("is_neg", false).toBool();

        list.emplace_back(parse_object<T>(s, m_updated), w);
    }
    s.endArray();
}

void Role::create_role_details(Dwarf *d){
    if(m_script.trimmed().isEmpty()){
        //unfortunate to have to loop through everything twice, but we need to handle the case where the users
        //change the global weights and re-build the string description. it's still better to do it here than when creating each cell
        role_details = "<h4>Aspects:</h4>";

        auto gdr = GameDataReader::ptr();
        role_details += get_aspect_details("Attributes",
                                           attributes_weight, attributes,
                                           [](const QString &name){return name;});
        role_details += get_aspect_details("Skills",
                                           skills_weight, skills,
                                           [gdr](int id){return gdr->get_skill_name(id);});
        role_details += get_aspect_details("Facets",
                                           facets_weight, facets,
                                           [gdr](int id){return gdr->get_trait_name(id);});
        role_details += get_aspect_details("Beliefs",
                                           beliefs_weight, beliefs,
                                           [gdr](int id){return gdr->get_belief_name(id);});
        role_details += get_aspect_details("Goals",
                                           goals_weight, goals,
                                           [gdr](int id){return gdr->get_goal_name(id);});
        role_details += get_aspect_details("Needs",
                                           needs_weight, needs,
                                           [gdr](int id){return gdr->get_need_name(id);});
        role_details += get_preference_details(d);
    }else{
        role_details = m_script;
        role_details.replace("d.","");
        role_details.replace("()", "");
        if(role_details.length() > 250)
            role_details = role_details.mid(0,250) + "...";
        role_details = "<h4>Script:</h4>" + role_details;
    }
}

QString Role::get_role_details(Dwarf *d){
    if(role_details == "")
        create_role_details(d);
    else
        refresh_preferences(d);
    return role_details;
}

template<typename T, typename F>
QString Role::get_aspect_details(const QString &title,
                                 const weight_info &aspect_group_weight,
                                 const std::vector<std::pair<T,aspect_weight>> &list,
                                 F id_to_name){

    std::vector<std::pair<QString, aspect_weight>> aspects;
    for(const auto &p: list)
        aspects.emplace_back(id_to_name(p.first), p.second);
    return generate_details(title, aspect_group_weight, aspects);
}

QString Role::generate_details(const QString &title,
                               const weight_info &aspect_group_weight,
                               const std::vector<std::pair<QString, aspect_weight>> &aspects){

    QString detail="";
    QString summary = "";
    QString title_formatted = title;

    if(aspects.size()>0){
        //sort the list by weight. if there are more than 3 values, group them by weights
        QMultiMap<float,QStringList> details;
        bool group_lines = false;
        if(aspects.size() > 3)
            group_lines = true;

        if(!aspect_group_weight.is_default())
            title_formatted.append(tr("<i> (w %1)</i>").arg(aspect_group_weight.weight()));

        summary = tr("<p style =\"margin:0; margin-left:10px; padding:0px;\"><b>%1:</b></p>").arg(title_formatted);
        QString w_str;
        for (const auto &p: aspects) {
            const QString &id = p.first;
            const auto &w = p.second;

            detail = capitalizeEach(id);
            float key = w.weight;
            if(w.is_neg){
                w_str = tr("<i> <font color=red>(w-%1)</font></i>").arg(w.weight,0,'f',2);
                key *= -1;
            }else{
                w_str = tr("<i> (w%1)</i>").arg(w.weight,0,'f',2);
            }

            if(group_lines){
                QStringList vals = details.take(key);
                if(vals.count() <= 0) //add the weight only to the first item
                    vals.append(w_str);
                vals.append(detail);
                details.insert(key,vals);
            }else{
                detail.append(w_str);
                details.insertMulti(key,QStringList(detail));
            }
        }

        summary += "<p style =\"margin:0; margin-left:40px; padding:0px;\">";

        QString aspect_list;
        if(group_lines){
            foreach(float key, details.uniqueKeys()){
                QStringList d = details.value(key);
                d.sort();
                //first value is the weight string, place it at the end
                aspect_list.prepend(" " + d.takeFirst() + "<br/>");
                aspect_list.prepend(d.join(", "));
            }
        }else{
            foreach(QStringList d, details.values()){
                aspect_list.prepend(d.join("<br/>") + "<br/>");
            }
        }

        summary += aspect_list;
        summary += "</p>";
    }
    return summary;
}

QString Role::get_preference_details(Dwarf *d){
    std::vector<std::pair<QString, aspect_weight>> list;
    for (const auto &p: prefs) {
        const auto &pref = p.first;
        const auto &wi = p.second;
        list.emplace_back(pref->get_name(), wi);
    }
    m_pref_desc = generate_details(tr("Preferences"), prefs_weight, list);
    QString pref_desc = m_pref_desc;

    highlight_pref_matches(d,pref_desc);
    m_cur_pref_len = pref_desc.length();
    return pref_desc;
}

void Role::refresh_preferences(Dwarf *d){
    role_details.chop(m_cur_pref_len);
    QString pref_desc = m_pref_desc;
    if(d){
        highlight_pref_matches(d,pref_desc);
    }
    m_cur_pref_len = pref_desc.length();
    role_details.append(pref_desc);
}

void Role::highlight_pref_matches(Dwarf *d, QString &pref_desc){
    if(d){
        QStringList pref_names;
        if(pref_desc != ""){
            QList<QPair<QString, QString> > pref_matches = d->get_role_pref_matches(m_name);
            QPair<QString,QString> p_match;
            QRegularExpression re;
            re.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
            foreach(p_match, pref_matches){
                re.setPattern(QString("((?<=, )|(?<=>)|^)%1(?=[$,\\n]|\\s*[<])").arg(re.escape(p_match.first)));
                pref_desc.replace(re, QString("<b>%2</b>").arg(p_match.first));
                pref_names.append(p_match.second);
            }

            if(pref_names.count() > 0){
                std::sort(pref_names.begin(), pref_names.end());
                pref_desc.insert(pref_desc.lastIndexOf("</p>"),tr("<br/><b>Matches:</b> %1").arg(pref_names.join(", ")));
            }
        }
    }
}

void Role::write_to_ini(QSettings &s) const {
    //name
    s.setValue("name",m_name);
    if(!m_script.trimmed().isEmpty())
        s.setValue("script", m_script);
    write_aspect_group(s,"attributes",attributes_weight,attributes);
    write_aspect_group(s,"traits",facets_weight,facets);
    write_aspect_group(s,"beliefs",beliefs_weight,beliefs);
    write_aspect_group(s,"goals",goals_weight,goals);
    write_aspect_group(s,"needs",needs_weight,needs);
    write_aspect_group(s,"skills",skills_weight,skills);
    write_aspect_group(s,"preferences",prefs_weight,prefs);
}

template<typename T>
static void write_object(QSettings &s, const T &value)
{
    s.setValue("id", value);
}

static void write_object(QSettings &s, const std::unique_ptr<RolePreference> &pref)
{
    pref->write(s);
}

template<typename T>
void Role::write_aspect_group(QSettings &s, const QString &group_name,
                              const weight_info &group_weight,
                              const std::vector<std::pair<T, aspect_weight>> &list) const {
    if(!list.empty()){
        if (!group_weight.is_default())
            s.setValue(group_name + "_weight", QString::number(group_weight.weight(),'g',0));

        int count = 0;
        s.beginWriteArray(group_name, list.size());
        for (const auto &p: list){
            auto &w = p.second;
            s.setArrayIndex(count);
            if(w.weight != 1.0)
                s.setValue("weight",QString::number(w.weight,'g',2));
            if (w.is_neg)
                s.setValue("is_neg", true);
            write_object(s, p.first);
            count ++;
        }
        s.endArray();
    }
}

RolePreference* Role::has_preference(QString name){
    for (const auto &p: prefs){
        if(QString::compare(p.first->get_name(), name, Qt::CaseInsensitive)==0)
            return p.first.get();
    }
    return 0;
}
