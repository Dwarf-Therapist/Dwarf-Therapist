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
        level l;
        s.setArrayIndex(i);
        int level = s.value("level_id", -1).toInt();
        QString level_name = s.value("level_name", "").toString();
        float level_limit = s.value("level_limit",-1).toFloat();
        if (level != -1){
            l.description = level_name;
            l.rating = level;
            if(level_limit < 0)
                level_limit=find_attribute_limit(id,i); //load the default level for this attribute
            l.limit = level_limit;
            //m_levels.insert(level, level_name);
            //m_limits << level_limit;
            m_levels.append(l);
        }
    }
    s.endArray();
}

int Attribute::find_attribute_limit(int id, int level_index)
{
    int ret_value = 0;
    QList<int> raws;

    //-
    if(id==Attribute::AT_AGILITY)
    {
        if(level_index==0){
            ret_value=1;
        }else if(level_index==1){
            ret_value=151;
        }else if(level_index==2){
            ret_value=401;
        }else if(level_index==3){
            ret_value=651;
        }else if(level_index==4){
            ret_value=1150;
        }else if(level_index==5){
            ret_value=1400;
        }else if(level_index==6){
            ret_value=1650;
        }else if(level_index==7){
            ret_value=1900;
        }else{
            ret_value=5000;
        }

        raws << 150 << 600 << 800 << 900 << 1000 << 1100 << 1500 << 5000;
        DwarfStats::load_attribute_bins(negative, raws);
        m_aspect_type = negative;

    }

    //+
    if (id==Attribute::AT_STRENGTH || id==Attribute::AT_TOUGHNESS || id==Attribute::AT_ANALYTICAL_ABILITY ||
            id==Attribute::AT_CREATIVITY || id==Attribute::AT_PATIENCE || id==Attribute::AT_MEMORY)
    {
        if(level_index==0){
            ret_value=251;
        }else if(level_index==1){
            ret_value=501;
        }else if(level_index==2){
            ret_value=751;
        }else if(level_index==3){
            ret_value=1001;
        }else if(level_index==4){
            ret_value=1500;
        }else if(level_index==5){
            ret_value=1750;
        }else if(level_index==6){
            ret_value=2000;
        }else if(level_index==7){
            ret_value=2250;
        }else{
            ret_value=5000;
        }

        raws << 450 << 950 << 1150 << 1250 << 1350 << 1550 << 2250 << 5000;
        DwarfStats::load_attribute_bins(positive, raws);
        m_aspect_type = positive;
    }

    //++
    if( id==Attribute::AT_SPATIAL_SENSE || id==Attribute::AT_FOCUS)
    {
        if(level_index==0){
            ret_value=543;
        }else if(level_index==1){
            ret_value=793;
        }else if(level_index==2){
            ret_value=1043;
        }else if(level_index==3){
            ret_value=1293;
        }else if(level_index==4){
            ret_value=1792;
        }else if(level_index==5){
            ret_value=2042;
        }else if(level_index==6){
            ret_value=2292;
        }else if(level_index==7){
            ret_value=2542;
        }else{
            ret_value=5000;
        }

        raws << 700 << 1200 << 1400 << 1500 << 1600 << 1800 << 2500 << 5000;
        DwarfStats::load_attribute_bins(double_positive, raws);
        m_aspect_type = double_positive;
    }

    //avg
    if (id==Attribute::AT_ENDURANCE || id==Attribute::AT_RECUPERATION || id==Attribute::AT_DISEASE_RESISTANCE ||
            id==Attribute::AT_INTUITION || id==Attribute::AT_WILLPOWER || id==Attribute::AT_KINESTHETIC_SENSE ||
            id==Attribute::AT_LINGUISTIC_ABILITY || id==Attribute::AT_MUSICALITY || id==Attribute::AT_EMPATHY ||
            id==Attribute::AT_SOCIAL_AWARENESS)
    {
        if(level_index==0){
            ret_value=1;
        }else if(level_index==1){
            ret_value=253;
        }else if(level_index==2){
            ret_value=501;
        }else if(level_index==3){
            ret_value=751;
        }else if(level_index==4){
            ret_value=1250;
        }else if(level_index==5){
            ret_value=1500;
        }else if(level_index==6){
            ret_value=1750;
        }else if(level_index==7){
            ret_value=2000;
        }else{
            ret_value=5000;
        }

        raws << 200 << 700 << 900 << 1000 << 1100 << 1300 << 2000 << 5000;
        DwarfStats::load_attribute_bins(average, raws);
        m_aspect_type = average;

    }

    return ret_value;
}
