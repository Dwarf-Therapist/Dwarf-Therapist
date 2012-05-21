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

#include "attribute.h"
#include "dwarfstats.h"


Attribute::Attribute(QSettings &s, QObject *parent)
        : QObject(parent)
        , id(s.value("id",0).toInt())
        , name(s.value("name", "UNKNOWN ATTRIBUTE").toString())
{
    int levels = s.beginReadArray("levels");
    for (int i = 0; i < levels; ++i) {
        s.setArrayIndex(i);
        m_display_descriptions.append(s.value("level_name").toString());
    }
    s.endArray();

    //load bins for roles
    load_role_bin(id);
}

void Attribute::load_role_bin(int id)
{    
    //these are still based on the default dwarf raws for now. because these bins are exclusively used for roles
    //we DONT want to use caste information, as we want to compare all dwarfs to each other when generating role
    //ratings, rather than comparing them to those in the same caste (obviously this mostly affects mods)
    QList<int> raws;
    //-
    if(id==Attribute::AT_AGILITY)
    {
        raws << 150 << 600 << 800 << 900 << 1000 << 1100 << 1500 << 5000;        
        m_aspect_type = negative;
    }
    //avg
    if (id==Attribute::AT_ENDURANCE || id==Attribute::AT_RECUPERATION || id==Attribute::AT_DISEASE_RESISTANCE ||
            id==Attribute::AT_INTUITION || id==Attribute::AT_WILLPOWER || id==Attribute::AT_KINESTHETIC_SENSE ||
            id==Attribute::AT_LINGUISTIC_ABILITY || id==Attribute::AT_MUSICALITY || id==Attribute::AT_EMPATHY ||
            id==Attribute::AT_SOCIAL_AWARENESS)
    {
        raws << 200 << 700 << 900 << 1000 << 1100 << 1300 << 2000 << 5000;
        m_aspect_type = average;

    }
    //+
    if (id==Attribute::AT_STRENGTH || id==Attribute::AT_TOUGHNESS || id==Attribute::AT_ANALYTICAL_ABILITY ||
            id==Attribute::AT_CREATIVITY || id==Attribute::AT_PATIENCE || id==Attribute::AT_MEMORY)
    {
        raws << 450 << 950 << 1150 << 1250 << 1350 << 1550 << 2250 << 5000;
        m_aspect_type = positive;
    }
    //++
    if( id==Attribute::AT_SPATIAL_SENSE || id==Attribute::AT_FOCUS)
    {
        raws << 700 << 1200 << 1400 << 1500 << 1600 << 1800 << 2500 << 5000;
        m_aspect_type = double_positive;
    }

    DwarfStats::load_attribute_bins(m_aspect_type, raws);
}
