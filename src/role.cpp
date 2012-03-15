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

Role::Role()
    :name("UNKNOWN")
    ,script("")
{}

Role::Role(QSettings &s, QObject *parent)
    : QObject(parent)
    , name(s.value("name", "UNKNOWN ROLE").toString())
    , script(s.value("script","").toString())
{
//    QStringList aList;
//    QStringList detail;
//    QString name = "";
//    QString a = "";
//    int id=0;
//    float val = 1.0;

    parseAspect(s.value("attributes").toString(), attribute_weight, attributes);
//    a = s.value("attributes").toString();
//    if(a.trimmed()!=""){
//        if(a.indexOf("::")>0){
//            attribute_weight = a.split("::")[0].QString::toFloat();
//            a = a.midRef(a.indexOf("::")+2).toString();
//        }else{
//            attribute_weight = 1.0;
//        }
//        aList = a.split(",");

//        //load the attributes
//        foreach(QString w, aList){
//            detail = w.split(":");
//            name = detail[0];
//            if(detail.length() > 1)
//                val = detail[1].QString::toFloat();
//            attributes.insert(name.trimmed(),(val <=0 ? 1 : val));
//        }
//    }else{
//        attribute_weight = 0;
//    }

    //load the traits
    parseAspect(s.value("traits").toString(), trait_weight, traits);
//    a = s.value("traits").toString();
//    QString id2;
//    if(a.trimmed()!=""){
//        if(a.indexOf("::")>0){
//            trait_weight = a.split("::")[0].QString::toFloat();
//            a = a.midRef(a.indexOf("::")+2).toString();
//        }else{
//            trait_weight = 1.0;
//        }
//        val = 1.0;
//        aList = a.split(",");
//        foreach(QString w, aList){
//            detail = w.split(":");
//            aspect t;
//            //id = detail[0].QString::toInt();
//            id2 = detail[0];
//            id2.indexOf("-")>=0 ? t.is_neg=true : t.is_neg=false;
//            if(detail.length() > 1)
//                val = detail[1].QString::toFloat();
//            t.weight = (val <=0 ? 1 : val);
//            traits.insert(abs(id2.toInt()), t);
//        }
//    }else{
//        trait_weight = 0;
//    }
    parseAspect(s.value("skills").toString(), skills_weight, skills);
//    //load the skills
//    a = s.value("skills").toString();
//    if(a.trimmed()!=""){
//        if(a.indexOf("::")>0){
//            skills_weight = a.split("::")[0].QString::toFloat();
//            a = a.midRef(a.indexOf("::")+2).toString();
//        }else{
//            skills_weight = 1.0;
//        }
//        val = 1.0;
//        aList = a.split(",");
//        foreach(QString w, aList){
//            detail = w.split(":");
//            id = detail[0].QString::toInt();
//            if(detail.length() > 1)
//                val = detail[1].QString::toFloat();
//            skills.insert(id,(val <=0 ? 1 : val));
//        }
//    }else{
//        skills_weight=0;
//    }
}

void Role::parseAspect(QString raw, float &weight, QHash<QString,aspect> &list)
{
    QStringList aList;
    QStringList detail;
    QString id = "0";
    float val = 1.0;

    if(raw.trimmed()!=""){
        if(raw.indexOf("::")>0){
            weight = raw.split("::")[0].QString::toFloat();
            raw = raw.midRef(raw.indexOf("::")+2).toString();
        }else{
            weight = 1.0;
        }
        val = 1.0;
        aList = raw.split(",");
        foreach(QString w, aList){
            detail = w.split(":");
            aspect t;
            id = detail[0];
            if (id.indexOf("-") >= 0){
                id = id.replace("-","");
                t.is_neg = true;
            }else{
                t.is_neg = false;
            }
            if(detail.length() > 1)
                val = detail[1].QString::toFloat();
            t.weight = (val <=0 ? 1 : val);
            list.insert(id.trimmed(), t);
        }
    }else{
        weight = 0;
    }
}
