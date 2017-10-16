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
#include "dwarftherapist.h"
#include "gamedatareader.h"
#include "preference.h"
#include "defines.h"
#include "dwarf.h"
#include "material.h"
#include "item.h"

#include <QSettings>
#include <QRegularExpression>

Role::Role()
    : m_name("UNKNOWN")
    , m_script("")
    , m_is_custom(false)
    , m_cur_pref_len(0)
    , m_updated(false)
{
    QSettings *u = DT->user_settings();
    attributes_weight.weight = u->value("options/default_attributes_weight",1.0).toFloat();
    attributes_weight.is_default = true;
    attributes_weight.is_neg = false;
    skills_weight.weight = u->value("options/default_skills_weight",1.0).toFloat();
    skills_weight.is_default = true;
    skills_weight.is_neg = false;
    traits_weight.weight = u->value("options/default_traits_weight",1.0).toFloat();
    traits_weight.is_default = true;
    traits_weight.is_neg = false;
    prefs_weight.weight = u->value("options/default_prefs_weight",1.0).toFloat();
    prefs_weight.is_default = true;
    prefs_weight.is_neg = false;
}

Role::Role(QSettings &s, QObject *parent)
    : QObject(parent)
    , m_name(s.value("name", "UNKNOWN ROLE").toString())
    , m_script(s.value("script","").toString())
    , m_is_custom(false)
    , m_cur_pref_len(0)
    , m_updated(false)
{
    QSettings *u = new QSettings(this);
    parseAspect(s, "attributes", attributes_weight, attributes, u->value("options/default_attributes_weight",1.0).toFloat());
    parseAspect(s, "traits", traits_weight, traits, u->value("options/default_traits_weight",1.0).toFloat());
    parseAspect(s, "skills", skills_weight, skills, u->value("options/default_skills_weight",1.0).toFloat());
    parsePreferences(s, "prefs", prefs_weight, u->value("options/default_prefs_weight",1.0).toFloat());
}

Role::Role(const Role &r)
    : QObject(r.parent())
    , attributes(r.attributes)
    , skills(r.skills)
    , traits(r.traits)
    , attributes_weight(r.attributes_weight)
    , skills_weight(r.skills_weight)
    , traits_weight(r.traits_weight)
    , prefs_weight(r.prefs_weight)
    , m_name(r.m_name)
    , m_script(r.m_script)
    , m_is_custom(r.m_is_custom)
    , role_details(r.role_details)
    , m_cur_pref_len(0)
    , m_updated(false)
{
    prefs.reserve(r.prefs.size());
    for (const auto &pref: r.prefs)
        prefs.emplace_back(std::make_unique<Preference>(*pref));
}

Role::~Role(){
}

void Role::parseAspect(QSettings &s, QString node, weight_info &g_weight, std::map<QString,RoleAspect> &list, float default_weight)
{
    list.clear();
    QString id = "";

    //keep track of whether or not we're using the global default when we redraw if options are changed
    g_weight.weight = s.value(QString("%1_weight").arg(node),-1).toFloat();
    if(g_weight.weight < 0){
        g_weight.weight = default_weight;
        g_weight.is_default = true;
    }else{
        g_weight.is_default = false;
    }

    int count = s.beginReadArray(node);

    for (int i = 0; i < count; ++i) {
        RoleAspect a;
        s.setArrayIndex(i);
        a.weight = s.value("weight",1.0).toFloat();
        if(a.weight < 0)
            a.weight = 1.0;
        id = s.value("id", "0").toString();
        if(id.indexOf("-") >= 0){
            id.replace("-","");
            a.is_neg = true;
        }else{
            a.is_neg = false;
        }
        list.emplace(id.trimmed(),a);
    }
    s.endArray();
}

void Role::parsePreferences(QSettings &s, QString node, weight_info &g_weight, float default_weight)
{
    prefs.clear();
    g_weight.weight = s.value(QString("%1_weight").arg(node),-1).toFloat();

    //keep track of whether or not we're using the global default when we redraw if options are changed
    if(g_weight.weight < 0){
        g_weight.weight = default_weight;
        g_weight.is_default = true;
    }else{
        g_weight.is_default = false;
    }

    int count = s.beginReadArray("preferences");
    QString id = "";
    for (int i = 0; i < count; ++i) {
        s.setArrayIndex(i);

        auto p = std::make_unique<Preference>();
        p->set_category(static_cast<PREF_TYPES>(s.value("pref_category",-1).toInt()));
        p->set_item_type(static_cast<ITEM_TYPE>(s.value("item_type",-1).toInt()));
        p->set_exact(s.value("exact",false).toBool());
        p->set_mat_state(static_cast<MATERIAL_STATES>(s.value("mat_state", -1).toInt ()));

        p->pref_aspect.weight = s.value("weight",1.0).toFloat();
        if(p->pref_aspect.weight < 0)
            p->pref_aspect.weight = 1.0;

        id = s.value("name",tr("Unknown")).toString();
        if(id == "Unknown"){
            LOGW << "Role" << m_name << "has an invalid preference, index:" << i;
        }

        if(!id.isEmpty() && id.indexOf("-") >= 0){
            id.replace("-","");
            p->pref_aspect.is_neg = true;
        }else{
            p->pref_aspect.is_neg = false;
        }
        p->set_name(id);

        int flag_count = s.beginReadArray("flags");
        int first_flag = -1;
        for(int j = 0; j < flag_count; j++){
            s.setArrayIndex(j);
            int f = s.value("flag").toInt();
            p->add_flag(f);
            if(j==0)
                first_flag = f;
        }
        s.endArray();

        //update any old flags with new ones
        validate_pref(p.get(),first_flag);

        prefs.emplace_back(std::move(p));
    }
    s.endArray();
}

void Role::validate_pref(Preference *p, int first_flag){
    //check and update any missing armor/clothing flags
    if(Item::is_armor_type(p->get_item_type()) && !p->flags().has_flag(IS_ARMOR) && !p->flags().has_flag(IS_CLOTHING)){
        p->add_flag(IS_ARMOR);
        m_updated = true;
    }

    //add missing trade goods flags
    if(Item::is_trade_good(p->get_item_type()) && !p->flags().has_flag(IS_TRADE_GOOD)){
        p->add_flag(IS_TRADE_GOOD);
        m_updated = true;
    }

    //add missing trainable, remove old flags
    if(p->get_pref_category() == LIKE_CREATURE &&
            p->flags().has_flag(TRAINABLE_HUNTING) && p->flags().has_flag(TRAINABLE_WAR) && !p->flags().has_flag(TRAINABLE)){
        p->add_flag(TRAINABLE);
        p->flags().set_flag(TRAINABLE_HUNTING,false);
        p->flags().set_flag(TRAINABLE_WAR,false);
        m_updated = true;
    }

    //update any general preference material names (eg. Horn -> Horn/Hoof)
    if(p->get_item_type() == NONE && !p->exact_match() && p->get_pref_category() == LIKE_MATERIAL &&
            first_flag >= 0 && first_flag < NUM_OF_MATERIAL_FLAGS){
            QString new_name = (Material::get_material_flag_desc(static_cast<MATERIAL_FLAGS>(first_flag)));
            if(new_name != p->get_name()){
                p->set_name(new_name);
                m_updated = true;
            }
    }

    //update old outdoor preference category (9) to new (99)
    if(p->get_pref_category() == 9 && p->get_item_type() == -1 && p->flags().has_flag(999)){
        p->set_category(LIKE_OUTDOORS);
        m_updated = true;
    }
}

void Role::create_role_details(QSettings &s, Dwarf *d){
    if(m_script.trimmed().isEmpty()){
        //unfortunate to have to loop through everything twice, but we need to handle the case where the users
        //change the global weights and re-build the string description. it's still better to do it here than when creating each cell
        role_details = "<h4>Aspects:</h4>";

        float default_attributes_weight = s.value("options/default_attributes_weight",1.0).toFloat();
        float default_skills_weight = s.value("options/default_skills_weight",1.0).toFloat();
        float default_traits_weight = s.value("options/default_traits_weight",1.0).toFloat();
        float default_prefs_weight = s.value("options/default_prefs_weight",1.0).toFloat();

        role_details += get_aspect_details("Attributes",attributes_weight,default_attributes_weight,attributes);
        role_details += get_aspect_details("Skills",skills_weight,default_skills_weight,skills);
        role_details += get_aspect_details("Traits",traits_weight,default_traits_weight,traits);
        role_details += get_preference_details(default_prefs_weight,d);
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
        create_role_details(*DT->user_settings(),d);
    else
        refresh_preferences(d);
    return role_details;
}

QString Role::get_aspect_details(QString title, weight_info aspect_group_weight,
                                 float aspect_default_weight, std::map<QString,RoleAspect> &list){

    QHash<QString, weight_info> weight_infos;
    for(const auto &p: list){
        weight_info wi = {false,p.second.is_neg,p.second.weight};
        weight_infos.insert(p.first, wi);
    }
    return generate_details(title,aspect_group_weight, aspect_default_weight, weight_infos);
}

QString Role::generate_details(QString title, weight_info aspect_group_weight,
                               float aspect_default_weight, QHash<QString,weight_info> weight_infos){

    float weight = 1.0;
    QString detail="";
    QString summary = "";
    QString title_formatted = title;
    weight_info w_info;

    if(weight_infos.count()>0){
        //sort the list by weight. if there are more than 3 values, group them by weights
        QMultiMap<float,QStringList> details;
        bool group_lines = false;
        if(weight_infos.count() > 3)
            group_lines = true;

        if(aspect_group_weight.weight != aspect_default_weight)
            title_formatted.append(tr("<i> (w %1)</i>").arg(aspect_group_weight.weight));

        summary = tr("<p style =\"margin:0; margin-left:10px; padding:0px;\"><b>%1:</b></p>").arg(title_formatted);
        QString w_str;
        foreach(QString id, weight_infos.uniqueKeys()){
            w_info = weight_infos.value(id);
            weight = w_info.weight;

            if(title.toLower()==tr("skills"))
                id = GameDataReader::ptr()->get_skill_name(id.toInt());
            if(title.toLower()==tr("traits"))
                id = GameDataReader::ptr()->get_trait_name(id.toInt());

            detail = capitalizeEach(id);
            float key = weight;
            if(w_info.is_neg){
                w_str = tr("<i> <font color=red>(w-%1)</font></i>").arg(weight,0,'f',2);
                key *= -1;
            }else{
                w_str = tr("<i> (w%1)</i>").arg(weight,0,'f',2);
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

QString Role::get_preference_details(float aspect_default_weight, Dwarf *d){
    QHash<QString, weight_info> weight_infos;
    for (const auto &pref: prefs) {
        weight_info wi = {false, pref->pref_aspect.is_neg, pref->pref_aspect.weight};
        weight_infos.insert(pref->get_name(),wi);
    }
    m_pref_desc = generate_details(tr("Preferences"), prefs_weight, aspect_default_weight, weight_infos);
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
                qSort(pref_names);
                pref_desc.insert(pref_desc.lastIndexOf("</p>"),tr("<br/><b>Matches:</b> %1").arg(pref_names.join(", ")));
            }
        }
    }
}

void Role::write_to_ini(QSettings &s, float default_attributes_weight, float default_traits_weight, float default_skills_weight, float default_prefs_weight){
    //name
    s.setValue("name",m_name);
    if(!m_script.trimmed().isEmpty())
        s.setValue("script", m_script);
    write_aspect_group(s,"attributes",attributes_weight,default_attributes_weight,attributes);
    write_aspect_group(s,"traits",traits_weight,default_traits_weight,traits);
    write_aspect_group(s,"skills",skills_weight,default_skills_weight,skills);
    write_pref_group(s,default_prefs_weight);
}

void Role::write_aspect_group(QSettings &s, QString group_name, weight_info group_weight, float group_default_weight, std::map<QString, RoleAspect> &list){
    if(!list.empty()){
        if(group_weight.weight > 0 && group_weight.weight != group_default_weight)
            s.setValue(group_name + "_weight", QString::number(group_weight.weight,'g',0));

        int count = 0;
        s.beginWriteArray(group_name, list.size());
        for (const auto &p: list){
            auto &key = p.first;
            auto &a = p.second;
            s.setArrayIndex(count);
            QString id = tr("%1%2").arg(a.is_neg ? "-" : "").arg(key);
            s.setValue("id",id);
            if(a.weight != 1.0){
                s.setValue("weight",QString::number(a.weight,'g',2));
            }
            count ++;
        }
        s.endArray();
    }
}

void Role::write_pref_group(QSettings &s, float default_prefs_weight){
    if(!prefs.empty()){
        if(prefs_weight.weight > 0 && prefs_weight.weight != default_prefs_weight)
            s.setValue("prefs_weight", QString::number(prefs_weight.weight,'g',0));

        int i_type = -1;
        QList<int> active_flags;
        s.beginWriteArray("preferences", prefs.size());
        for(unsigned int i = 0; i < prefs.size(); i++){
            s.setArrayIndex(i);
            auto *p = prefs.at(i).get();
            s.setValue("pref_category",QString::number((int)p->get_pref_category()));
            i_type = (int)p->get_item_type();
            if(i_type != -1){
                s.setValue("item_type", QString::number(i_type));
            }
            s.setValue("exact", p->exact_match());
            if (p->mat_state() != ANY_STATE){
                s.setValue("mat_state", QString::number(p->mat_state()));
            }

            QString id = tr("%1%2").arg(p->pref_aspect.is_neg ? "-" : "").arg(p->get_name());
            s.setValue("name",id);
            if(p->pref_aspect.weight != 1.0){
                s.setValue("weight",QString::number(p->pref_aspect.weight,'g',2));
            }

            active_flags = p->flags().active_flags();
            if(active_flags.count() > 0){
                s.beginWriteArray("flags",active_flags.count());
                for(int idx = 0; idx < active_flags.count(); idx++){
                    s.setArrayIndex(idx);
                    s.setValue("flag",active_flags.at(idx));
                }
                s.endArray();
            }
        }
        s.endArray();
    }
}

Preference* Role::has_preference(QString name){
    for (const auto &p: prefs){
        if(QString::compare(p->get_name(), name, Qt::CaseInsensitive)==0)
            return p.get();
    }
    return 0;
}
