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
#include "roleaspect.h"
#include "dwarftherapist.h"
#include "gamedatareader.h"
#include "preference.h"


Role::Role()
    :name("UNKNOWN")
    ,script("")
    ,is_custom(false)
{
    QSettings *u = DT->user_settings();
    attributes_weight.weight = u->value("options/default_attributes_weight",1.0).toFloat();
    attributes_weight.is_default = true;
    skills_weight.weight = u->value("options/default_skills_weight",1.0).toFloat();
    skills_weight.is_default = true;
    traits_weight.weight = u->value("options/default_traits_weight",1.0).toFloat();
    traits_weight.is_default = true;
    prefs_weight.weight = u->value("options/default_prefs_weight",1.0).toFloat();
    prefs_weight.is_default = true;
}

Role::Role(QSettings &s, QObject *parent)
    : QObject(parent)
    , name(s.value("name", "UNKNOWN ROLE").toString())
    , script(s.value("script","").toString())
    , is_custom(false)
{
    QSettings *u = new QSettings(QSettings::IniFormat, QSettings::UserScope, COMPANY, PRODUCT, this);
    parseAspect(s, "attributes", attributes_weight, attributes, u->value("options/default_attributes_weight",1.0).toFloat());
    parseAspect(s, "traits", traits_weight, traits, u->value("options/default_traits_weight",1.0).toFloat());
    parseAspect(s, "skills", skills_weight, skills, u->value("options/default_skills_weight",1.0).toFloat());
    parsePreferences(s, "prefs", prefs_weight, u->value("options/default_prefs_weight",1.0).toFloat());

}

Role::Role(const Role &r)
    :QObject(r.parent())
{
    attributes = r.attributes;
    skills = r.skills;
    traits = r.traits;
    prefs = r.prefs;
    attributes_weight = r.attributes_weight;
    skills_weight = r.skills_weight;
    traits_weight = r.traits_weight;
    prefs_weight = r.prefs_weight;
    name = r.name;
    script = r.script;
    is_custom = r.is_custom;
    role_details = r.role_details;
}

Role::~Role(){
    qDeleteAll(attributes);
    qDeleteAll(skills);
    qDeleteAll(traits);
    qDeleteAll(prefs);

    attributes.clear();
    skills.clear();
    traits.clear();
    prefs.clear();
}

void Role::parseAspect(QSettings &s, QString node, global_weight &g_weight, QHash<QString,RoleAspect*> &list, float default_weight)
{
    qDeleteAll(list);
    list.clear();
    QString id = "";
    RoleAspect *a;

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
        a = new RoleAspect();
        s.setArrayIndex(i);
        a->weight = s.value("weight",1.0).toFloat();
        if(a->weight < 0)
            a->weight = 1.0;
        id = s.value("id", "0").toString();
        if(id.indexOf("-") >= 0){
            id.replace("-","");
            a->is_neg = true;
        }else{
            a->is_neg = false;
        }
        list.insert(id.trimmed(),a);
    }
    s.endArray();
}

void Role::parsePreferences(QSettings &s, QString node, global_weight &g_weight, float default_weight)
{
    qDeleteAll(prefs);
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

        Preference *p = new Preference(this);
        p->set_category(static_cast<PREF_TYPES>(s.value("pref_category",-1).toInt()));
        p->set_item_type(static_cast<ITEM_TYPE>(s.value("item_type",-1).toInt()));
        p->set_exact(s.value("exact",false).toBool());

        p->pref_aspect->weight = s.value("weight",1.0).toFloat();
        if(p->pref_aspect->weight < 0)
            p->pref_aspect->weight = 1.0;

        id = s.value("name").toString();
        if(!id.isEmpty() && id.indexOf("-") >= 0){
            id.replace("-","");
            p->pref_aspect->is_neg = true;
        }else{
            p->pref_aspect->is_neg = false;
        }
        p->set_name(id);

        int flag_count = s.beginReadArray("flags");
        for(int j = 0; j < flag_count; j++){
            s.setArrayIndex(j);
            p->add_flag(s.value("flag").toInt());
        }
        s.endArray();
        prefs.append(p);
    }
    s.endArray();
}

void Role::create_role_details(QSettings &s){
    //unfortunate to have to loop through everything twice, but we need to handle the case where the users
    //change the global weights and re-build the string description. it's still better to do it here than when creating each cell
    role_details = "<h4>Aspects:</h4>";

    float default_attributes_weight = s.value("options/default_attributes_weight",1.0).toFloat();
    float default_skills_weight = s.value("options/default_skills_weight",1.0).toFloat();
    float default_traits_weight = s.value("options/default_traits_weight",1.0).toFloat();
    float default_prefs_weight = s.value("options/default_prefs_weight",1.0).toFloat();

    role_details += build_aspect_detail("Attributes",attributes_weight,default_attributes_weight,attributes);
    role_details += build_aspect_detail("Skills",skills_weight,default_skills_weight,skills);
    role_details += build_aspect_detail("Traits",traits_weight,default_traits_weight,traits);
    role_details += build_preference_detail(default_prefs_weight);
}

QString Role::get_role_details(){
    if(role_details == "")
        create_role_details(*DT->user_settings());
    return role_details;
}

QString Role::build_aspect_detail(QString title, global_weight aspect_group_weight,
                                  float aspect_default_weight, QHash<QString,RoleAspect*> &list){

    float weight = 1.0;
    QString detail="";
    RoleAspect *a;

    if(list.count()>0){
        detail = tr("&nbsp;&nbsp;%1:").arg(title);

        if(aspect_group_weight.weight != aspect_default_weight)
            detail += tr(" (w %1)").arg(aspect_group_weight.weight);

        foreach(QString name, list.uniqueKeys()){
            a = list.value(name);
            weight = a->weight;

            //not optimal, probably better to use a delegate
            if(title=="Skills")
                name = GameDataReader::ptr()->get_skill_name(name.toInt());
            if(title=="Traits")
                name = GameDataReader::ptr()->get_trait_name(name.toInt());

            name[0] = name[0].toUpper();
            detail += tr("<p style =\"margin:0; margin-left:40px; padding:0px;\">%1 %2").arg((a->is_neg ? "<u>Not</u> " : "")).arg(name);
            if(weight != 1.0) //hide default weights
                detail += tr(" (w %2)").arg(weight);
            detail += tr("</p>");
        }
        detail += tr("<br/>");
    }
    return detail;
}

QString Role::build_preference_detail(float aspect_default_weight){

    float weight = 1.0;
    QString detail="";
    RoleAspect *a;

    if(prefs.count()>0){
        detail = tr("&nbsp;&nbsp;Preferences:");

        if(prefs_weight.weight != aspect_default_weight)
            detail += tr(" (w %1)").arg(prefs_weight.weight);

        for(int i = 0; i < prefs.count(); i++){
            a = prefs.at(i)->pref_aspect;
            weight = a->weight;
            QString name = capitalize(prefs.at(i)->get_name());            
            detail += tr("<p style =\"margin:0; margin-left:40px; padding:0px;\">%1 %2").arg((a->is_neg ? "<u>Dislikes</u> " : "")).arg(name);
            if(weight != 1.0) //hide default weights
                detail += tr(" (w %2)").arg(weight);
            detail += "</p>";//tr("</dd>");
        }
        detail += tr("<br>");
    }
    return detail;
}

void Role::write_to_ini(QSettings &s, float default_attributes_weight, float default_traits_weight, float default_skills_weight, float default_prefs_weight){
    //name
    s.setValue("name",name);
    if(!script.trimmed().isEmpty())
        s.setValue("script", script);
    write_aspect_group(s,"attributes",attributes_weight,default_attributes_weight,attributes);
    write_aspect_group(s,"traits",traits_weight,default_traits_weight,traits);
    write_aspect_group(s,"skills",skills_weight,default_skills_weight,skills);
    write_pref_group(s,default_prefs_weight);
}

void Role::write_aspect_group(QSettings &s, QString group_name, global_weight group_weight, float group_default_weight, QHash<QString, RoleAspect*> &list){
    if(list.count()>0){
        RoleAspect *a;
        if(group_weight.weight > 0 && group_weight.weight != group_default_weight)
            s.setValue(group_name + "_weight", QString::number(group_weight.weight,'g',0));

        int count = 0;
        s.beginWriteArray(group_name, list.count());
        foreach(QString key, list.uniqueKeys()){
            s.setArrayIndex(count);
            a = list.value(key);
            QString id = tr("%1%2").arg(a->is_neg ? "-" : "").arg(key);
            s.setValue("id",id);
            if(a->weight != 1.0){
                s.setValue("weight",QString::number(a->weight,'g',2));
            }
            count ++;
        }
        s.endArray();
    }
}

void Role::write_pref_group(QSettings &s, float default_prefs_weight){
    if(prefs.count()>0){
        Preference *p;
        if(prefs_weight.weight > 0 && prefs_weight.weight != default_prefs_weight)
            s.setValue("prefs_weight", QString::number(prefs_weight.weight,'g',0));

        s.beginWriteArray("preferences", prefs.count());
        for(int i = 0; i < prefs.count(); i++){
            s.setArrayIndex(i);
            p = prefs.at(i);
            s.setValue("pref_category",QString::number((int)p->get_pref_category()));
            s.setValue("item_type", QString::number((int)p->get_item_type()));
            s.setValue("exact", p->exact_match());

            QString id = tr("%1%2").arg(p->pref_aspect->is_neg ? "-" : "").arg(p->get_name());
            s.setValue("name",id);
            if(p->pref_aspect->weight != 1.0){
                s.setValue("weight",QString::number(p->pref_aspect->weight,'g',2));
            }

            s.beginWriteArray("flags",p->special_flags().count());
            for(int j = 0; j < p->special_flags().count(); j++){
                s.setArrayIndex(j);
                s.setValue("flag",p->special_flags().at(j));
            }
            s.endArray();
        }
        s.endArray();
    }
}
