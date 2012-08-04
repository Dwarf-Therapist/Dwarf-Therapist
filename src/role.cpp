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
}

Role::Role(QSettings &s, QObject *parent)
    : QObject(parent)
    , name(s.value("name", "UNKNOWN ROLE").toString())
    , script(s.value("script","").toString())
    , is_custom(false)
{
    parseAspect(s, "attributes", attributes_weight, attributes);
    parseAspect(s, "traits", traits_weight, traits);
    parseAspect(s, "skills", skills_weight, skills);
}

Role::Role(const Role &r)
    :QObject(r.parent())
{
    attributes = r.attributes;
    skills = r.skills;
    traits = r.traits;
    attributes_weight = r.attributes_weight;
    skills_weight = r.skills_weight;
    traits_weight = r.traits_weight;
    name = r.name;
    script = r.script;
    is_custom = r.is_custom;
    role_details = r.role_details;
}

void Role::parseAspect(QSettings &s, QString node, global_weight &g_weight, QHash<QString,aspect> &list)
{
    list.clear();
    QString id = "";
    aspect a;
    float default_weight = DT->user_settings()->value(QString("options/default_%1_weight").arg(node),1.0).toFloat();
    g_weight.weight = s.value(QString("%1_weight").arg(node),-1).toFloat();

    //keep track of whether or not we're using the global default when we redraw if options are changed
    if(g_weight.weight < 0){
        g_weight.weight = default_weight;
        g_weight.is_default = true;
    }else{
        g_weight.is_default = false;
    }

    int count = s.beginReadArray(node);
//    if(count<=0)
//        g_weight.weight = 0;

    for (int i = 0; i < count; ++i) {
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
        list.insert(id.trimmed(),a);
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

    role_details += build_aspect_detail("Attributes",attributes_weight,default_attributes_weight,attributes);
    role_details += build_aspect_detail("Skills",skills_weight,default_skills_weight,skills);
    role_details += build_aspect_detail("Traits",traits_weight,default_traits_weight,traits);
}

QString Role::get_role_details(){
    if(role_details == "")
        create_role_details(*DT->user_settings());
    return role_details;
}

QString Role::build_aspect_detail(QString title, global_weight aspect_group_weight,
                                  float aspect_default_weight, QHash<QString,aspect> &list){

    float weight = 1.0;
    QString detail="";
    aspect a;

    if(list.count()>0){
        detail = title + ":";

        if(aspect_group_weight.weight != aspect_default_weight)
            detail += tr(" (w %1)").arg(aspect_group_weight.weight);

        foreach(QString name, list.uniqueKeys()){
            a = list.value(name);
            weight = a.weight;

            //not optimal, probably better to use a delegate
            if(title=="Skills")
                name = GameDataReader::ptr()->get_skill_name(name.toInt());
            if(title=="Traits")
                name = GameDataReader::ptr()->get_trait_name(name.toInt());

            name[0] = name[0].toUpper();
            detail += tr("<p><dd>%1 %2").arg((a.is_neg ? "<u>Not</u> " : "")).arg(name);
            if(weight != 1.0) //hide default weights
                detail += tr(" (w %2)").arg(weight);
            detail += tr("</dd></p>");
        }
        detail += tr("<br>");
    }
    return detail;
}

void Role::write_to_ini(QSettings &s, float default_attributes_weight, float default_traits_weight, float default_skills_weight){
    //name
    s.setValue("name",name);

    write_aspect_group(s,"attributes",attributes_weight,default_attributes_weight,attributes);
    write_aspect_group(s,"traits",traits_weight,default_traits_weight,traits);
    write_aspect_group(s,"skills",skills_weight,default_skills_weight,skills);
}

void Role::write_aspect_group(QSettings &s, QString group_name, global_weight group_weight, float group_default_weight, QHash<QString,aspect> &list){
    if(list.count()>0){
        aspect a;
        if(group_weight.weight > 0 && group_weight.weight != group_default_weight)
            s.setValue(group_name + "_weight", QString::number(group_weight.weight,'g',0));

        int count = 0;
        s.beginWriteArray(group_name, list.count());
        foreach(QString key, list.uniqueKeys()){
            s.setArrayIndex(count);
            a = (Role::aspect)list.value(key);
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
