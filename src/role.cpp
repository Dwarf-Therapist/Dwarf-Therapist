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

Role::Role()
    :name("UNKNOWN")
    ,script("")
{}

Role::Role(QSettings &s, QObject *parent)
    : QObject(parent)
    , name(s.value("name", "UNKNOWN ROLE").toString())
    , script(s.value("script","").toString())
{
    parseAspect(s, "attributes", attributes_weight, attributes);
    parseAspect(s, "traits", traits_weight, traits);
    parseAspect(s, "skills", skills_weight, skills);
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
    if(count<=0)
        g_weight.weight = 0;

    for (int i = 0; i < count; ++i) {
        s.setArrayIndex(i);
        a.weight = s.value("weight",1.0).toFloat();
        if((a.weight) < 0)
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
