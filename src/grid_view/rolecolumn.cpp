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
#include <QtScript>

#include "rolecolumn.h"
#include "gamedatareader.h"
#include "viewcolumnset.h"
#include "columntypes.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "dwarfstats.h"
#include "trait.h"

RoleColumn::RoleColumn(const QString &title, Role *r, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title,CT_ROLE,set,parent)
    ,m_role(r)
{}

RoleColumn::RoleColumn(const RoleColumn &to_copy)
    : ViewColumn(to_copy)
    , m_role(to_copy.m_role)
{}

RoleColumn::RoleColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s, set, parent)
{
    m_role = GameDataReader::ptr()->get_role(s.value("name").toString()); //new Role(s,parent);
}

QStandardItem *RoleColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);
    float rawRating = 0.0;
    float rating_att = 0.0;
    float rating_trait = 0.0;
    float rating_skill = 0.0;
    float rating_total = 0.0;

    float drawRating = 0.0;

    float skill_weight = m_role->skills_weight;
    float attrib_weight = m_role->attribute_weight;
    float trait_weight = m_role->trait_weight;

    Role::aspect a;

    QString skills_str="";
    QString attribs_str="";
    QString traits_str="";

    //if we have a script, use that
    if(m_role->script != ""){
        QScriptValue d_obj = m_engine.newQObject(d);
        m_engine.globalObject().setProperty("d", d_obj);
        rating_total = m_engine.evaluate(m_role->script).toNumber(); //just show the raw value the script generates
        drawRating = rating_total; //won't be drawn as it's arbitrary numbers
    }else
    {
        //read the attributes, traits and skills, and calculate the ratings
        float total_weight = 0.0;
        float weight = 1.0;
        int id = 0;

        //**************** ATTRIBUTES ****************
        if(m_role->attributes.count()>0){

            attribs_str = tr("Attributes");
            if(attrib_weight != 1.0)
                attribs_str += tr(" (w %1)").arg(attrib_weight);

            foreach(QString name, m_role->attributes.uniqueKeys()){
                a = m_role->attributes.value(name);
                weight = a.weight;
                name[0] = name[0].toUpper();
                attribs_str += tr("<p><dd>%1 %2").arg((a.is_neg ? "<u>Not</u> " : "")).arg(name);
                if(weight != 1.0)
                    attribs_str += tr(" (w %2)").arg(weight);                            
                attribs_str += tr("</dd></p>");

                name = name.toLower();
                //map the user's attribute name to enum
                if(name == "strength"){id = Attribute::AT_STRENGTH;}
                else if(name == "agility"){id = Attribute::AT_AGILITY;}
                else if(name == "toughness"){id = Attribute::AT_TOUGHNESS;}
                else if(name == "endurance"){id = Attribute::AT_ENDURANCE;}
                else if(name == "recuperation"){id = Attribute::AT_RECUPERATION;}
                else if(name == "disease resistance"){id = Attribute::AT_DISEASE_RESISTANCE;}
                else if(name == "analytical ability"){id = Attribute::AT_ANALYTICAL_ABILITY;}
                else if(name == "focus"){id = Attribute::AT_FOCUS;}
                else if(name == "willpower"){id = Attribute::AT_WILLPOWER;}
                else if(name == "creativity"){id = Attribute::AT_CREATIVITY;}
                else if(name == "intuition"){id = Attribute::AT_INTUITION;}
                else if(name == "patience"){id = Attribute::AT_PATIENCE;}
                else if(name == "memory"){id = Attribute::AT_MEMORY;}
                else if(name == "linguistic ability"){id = Attribute::AT_LINGUISTIC_ABILITY;}
                else if(name == "spatial sense"){id = Attribute::AT_SPATIAL_SENSE;}
                else if(name == "musicality"){id = Attribute::AT_MUSICALITY;}
                else if(name == "kinesthetic sense"){id = Attribute::AT_KINESTHETIC_SENSE;}
                else if(name == "empathy"){id = Attribute::AT_EMPATHY;}
                else if(name == "social awareness"){id = Attribute::AT_SOCIAL_AWARENESS;}

                //standardize
                rawRating = DwarfStats::get_attribute_rating(static_cast<Attribute::ATTRIBUTES_TYPE>(id), d->attribute(id));
                //invert if necessary
                if(a.is_neg)
                    rawRating = (1 - rawRating);
                //weigh
                rating_att += (rawRating * weight);
                //track the total weights
                total_weight += weight;
            }
            attribs_str += "</br>";

            //calculate weighted average
            rating_att = (rating_att / total_weight) * 100;
            //weight the attributes on the attribute weight
            rating_att *= attrib_weight;
        }
        //********************************


        //**************** TRAITS ****************
        if(m_role->traits.count()>0)
        {
            traits_str=tr("Traits");
            if(trait_weight != 1.0)
                traits_str += tr(" (w %1)").arg(trait_weight);
            total_weight = 0;
            Trait *t;
            foreach(QString trait_id, m_role->traits.uniqueKeys()){
                a = m_role->traits.value(trait_id);
                weight = a.weight;
                t = GameDataReader::ptr()->get_trait(trait_id.toInt());
                QString trait_name = t->name;
                trait_name[0] = trait_name[0].toUpper();
                traits_str += tr("<p><dd>%1 %2").arg((a.is_neg ? "<u>Not</u> " : "")).arg(trait_name);
                if(weight != 1.0)
                    traits_str += tr(" (w %2)").arg(weight);
                traits_str += tr("</dd></p>");

                //traits are already in a range from 0-100 so we don't need to standardize them, we just hack in arbitrary mean/stdev
                rawRating = DwarfStats::get_trait_rating(d->trait(trait_id.toInt()));
                //if the trait is negative, then we need to invert the rating ie. for a liar, the less honest they are, the better
                if(a.is_neg)
                    rawRating = (1 - rawRating);
                rating_trait += (rawRating * weight);
                total_weight += weight;
            }
            traits_str += "</br>";
            rating_trait = (rating_trait / total_weight) * 100;
            //weight the traits on the trait weight
            rating_trait *= trait_weight;
        }
        //********************************


        //************ SKILLS ************
        if(m_role->skills.count()>0){
            skills_str=tr("Skills");
            if(skill_weight != 1.0)
                skills_str += tr(" (w %1)").arg(skill_weight);

            total_weight = 0;
            int skill_value = 0;
            foreach(QString skill_id, m_role->skills.uniqueKeys()){
                a = m_role->skills.value(skill_id);
                weight = a.weight;

                QString skill_name = GameDataReader::ptr()->get_skill_name(skill_id.toInt());
                skill_name[0] = skill_name[0].toUpper();
                skills_str += tr("<p><dd>%1 %2").arg((a.is_neg ? "<u>Not</u> " : "")).arg(skill_name);
                if(weight != 1.0)
                    skills_str += tr(" (w %2)").arg(weight);
                skills_str += tr("</dd></p>");

                skill_value = d->skill_rating(skill_id.toInt());
                if(skill_value < 0)
                    skill_value = 0;
                rawRating = DwarfStats::get_skill_rating(skill_id.toInt(), skill_value);
                if(rawRating < 0)
                    rawRating = 0;
                if(a.is_neg)
                    rawRating = 1-rawRating;
                rating_skill += (rawRating * weight);
                total_weight += weight;
            }
            rating_skill = (rating_skill / total_weight) * 100;
            //weight the skills on the skill weight
            rating_skill *= skill_weight;
            skills_str += "</br>";
        }
        //********************************

        //calculate the total weighted average rating of attributes, traits and skills
        if(attrib_weight+trait_weight+skill_weight>0){
            rating_total = (rating_att + rating_trait + rating_skill) / (attrib_weight+trait_weight+skill_weight);
        }else{
            //something has gone wrong
            rating_total = rating_att + rating_trait + rating_skill;
        }

        //assume the values are between 0 and 100, now augment them to be between -20 and +20
        //this is if we want to draw like the attributes, the worse, the larger the red square
        drawRating = (rating_total-50)/2.5;
    }


    item->setData((int)drawRating, DwarfModel::DR_RATING); //drawing value
    item->setData(rating_total, DwarfModel::DR_SORT_VALUE);
    item->setData(CT_ROLE, DwarfModel::DR_COL_TYPE);

    QString match_str;
    QString aspects_str;
    if (m_role->script == "") {
        if(rating_total > 0){
            match_str = QString("Meets %1% of this role's aspects on <u>average</u>.").arg(ceil(rating_total));
            aspects_str = "<h4>Aspects:</h4></br>";
            aspects_str += attribs_str;
            aspects_str += skills_str;
            aspects_str += traits_str;
            aspects_str += tr("<br><b>Note:</b> A higher weight (w) puts greater value on the aspect. Default weights of 1 are not shown.");
            match_str += "</br>" + aspects_str;
        }else{
            match_str = tr("Incapable of filling this role.<br><br>Value: %1</br>").arg(rating_total);
        }
    } else {
        match_str = tr("Scripted role.<br><br>Value: %1</br>").arg(rating_total);
        item->setToolTip(QString("<h3>%1 - %3</h3>%2<h4>%4</h4>").arg(m_role->name).arg(match_str).arg(ceil(rating_total)).arg(d->nice_name()));
    }

    item->setToolTip(QString("<h3>%1 - %3%</h3>%2<h4>%4</h4>").arg(m_role->name).arg(match_str).arg(ceil(rating_total)).arg(d->nice_name()));
    return item;
}

QStandardItem *RoleColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) {
    Q_UNUSED(group_name);
    Q_UNUSED(dwarves);
    QStandardItem *item = new QStandardItem;
    item->setData(m_bg_color, DwarfModel::DR_DEFAULT_BG_COLOR);
    return item;
}

