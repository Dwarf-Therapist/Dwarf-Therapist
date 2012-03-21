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
#include "dwarftherapist.h"
#include "truncatingfilelogger.h"

RoleColumn::RoleColumn(const QString &title, Role *r, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title,CT_ROLE,set,parent)
    ,m_role(r)
{
    connect(DT, SIGNAL(settings_changed()), this, SLOT(read_settings()));
}

RoleColumn::RoleColumn(const RoleColumn &to_copy)
    : ViewColumn(to_copy)
    , m_role(to_copy.m_role)
{}

RoleColumn::RoleColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s, set, parent)
{
    m_role = GameDataReader::ptr()->get_role(s.value("name").toString());
    connect(DT, SIGNAL(settings_changed()), this, SLOT(read_settings()));
}

QStandardItem *RoleColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);
    float rating_att = 0.0;
    float rating_trait = 0.0;
    float rating_skill = 0.0;
    float rating_total = 0.0;

    float drawRating = 0.0;

    QSettings *s = DT->user_settings();
    float default_attributes_weight = s->value("options/default_attributes_weight",1.0).toFloat();
    float default_skills_weight = s->value("options/default_skills_weight",1.0).toFloat();
    float default_traits_weight = s->value("options/default_traits_weight",1.0).toFloat();

    QString skills_str="";
    QString attribs_str="";
    QString traits_str="";

    float avgMeanDev_attribs = 0.0;
    float avgStdev_attribs=0.0;

    float avgMeanDev_skills = 0.0;
    float avgStdev_skills=0.0;

    float avgMeanDev_traits = 0.0;
    float avgStdev_traits=0.0;

    float deviation = 0.0;

    if(m_role){

        float skill_weight = m_role->skills_weight.weight;
        float attrib_weight = m_role->attributes_weight.weight;
        float trait_weight = m_role->traits_weight.weight;

        Role::aspect a;

        //if we have a script, use that
        if(m_role->script != ""){
            QScriptValue d_obj = m_engine.newQObject(d);
            m_engine.globalObject().setProperty("d", d_obj);
            rating_total = m_engine.evaluate(m_role->script).toNumber(); //just show the raw value the script generates
            if(rating_total <=100 && rating_total >=0)
                drawRating = (rating_total - 50) / 2.5; //if it looks like a percent result, scale it to -20->+20
            else
                drawRating = rating_total; //won't be drawn as it's arbitrary numbers
        }else
        {
            //read the attributes, traits and skills, and calculate the ratings
            float total_weight = 0.0;
            float weight = 1.0;

            //**************** ATTRIBUTES ****************
            if(m_role->attributes.count()>0){

                attribs_str = tr("Attributes");
                if(attrib_weight != default_attributes_weight)
                    attribs_str += tr(" (w %1)").arg(attrib_weight);

                int attrib_id = 0;
                deviation = 0;
                foreach(QString name, m_role->attributes.uniqueKeys()){
                    a = m_role->attributes.value(name);
                    weight = a.weight;
                    name[0] = name[0].toUpper();
                    attribs_str += tr("<p><dd>%1 %2").arg((a.is_neg ? "<u>Not</u> " : "")).arg(name);
                    if(weight != 1.0) //hide default weights
                        attribs_str += tr(" (w %2)").arg(weight);
                    attribs_str += tr("</dd></p>");

                    name = name.toLower();
                    //map the user's attribute name to enum
                    if(name == "strength"){attrib_id = Attribute::AT_STRENGTH;}
                    else if(name == "agility"){attrib_id = Attribute::AT_AGILITY;}
                    else if(name == "toughness"){attrib_id = Attribute::AT_TOUGHNESS;}
                    else if(name == "endurance"){attrib_id = Attribute::AT_ENDURANCE;}
                    else if(name == "recuperation"){attrib_id = Attribute::AT_RECUPERATION;}
                    else if(name == "disease resistance"){attrib_id = Attribute::AT_DISEASE_RESISTANCE;}
                    else if(name == "analytical ability"){attrib_id = Attribute::AT_ANALYTICAL_ABILITY;}
                    else if(name == "focus"){attrib_id = Attribute::AT_FOCUS;}
                    else if(name == "willpower"){attrib_id = Attribute::AT_WILLPOWER;}
                    else if(name == "creativity"){attrib_id = Attribute::AT_CREATIVITY;}
                    else if(name == "intuition"){attrib_id = Attribute::AT_INTUITION;}
                    else if(name == "patience"){attrib_id = Attribute::AT_PATIENCE;}
                    else if(name == "memory"){attrib_id = Attribute::AT_MEMORY;}
                    else if(name == "linguistic ability"){attrib_id = Attribute::AT_LINGUISTIC_ABILITY;}
                    else if(name == "spatial sense"){attrib_id = Attribute::AT_SPATIAL_SENSE;}
                    else if(name == "musicality"){attrib_id = Attribute::AT_MUSICALITY;}
                    else if(name == "kinesthetic sense"){attrib_id = Attribute::AT_KINESTHETIC_SENSE;}
                    else if(name == "empathy"){attrib_id = Attribute::AT_EMPATHY;}
                    else if(name == "social awareness"){attrib_id = Attribute::AT_SOCIAL_AWARENESS;}

                    deviation = d->attribute(attrib_id) - DwarfStats::get_attribute_mean(attrib_id);
                    if(a.is_neg)
                        deviation *= -1;
                    deviation /= DwarfStats::get_attribute_stdev(attrib_id);
                    deviation *= weight;
                    rating_att += deviation;
                    total_weight += pow(weight,2);

                }
                attribs_str += "</br>";

                avgMeanDev_attribs = rating_att / m_role->attributes.count();
                avgStdev_attribs = sqrt(total_weight / m_role->attributes.count());
                rating_att = DwarfStats::calc_cdf(0,avgStdev_attribs,avgMeanDev_attribs)*100;
            }
            //********************************


            //**************** TRAITS ****************
            if(m_role->traits.count()>0)
            {
                traits_str=tr("Traits");
                if(trait_weight != default_traits_weight)
                    traits_str += tr(" (w %1)").arg(trait_weight);
                total_weight = 0;
                deviation = 0;
                Trait *t;
                foreach(QString trait_id, m_role->traits.uniqueKeys()){
                    a = m_role->traits.value(trait_id);
                    weight = a.weight;
                    t = GameDataReader::ptr()->get_trait(trait_id.toInt());
                    QString trait_name = t->name;
                    trait_name[0] = trait_name[0].toUpper();
                    traits_str += tr("<p><dd>%1 %2").arg((a.is_neg ? "<u>Not</u> " : "")).arg(trait_name);
                    if(weight != 1.0) //hide default weights
                        traits_str += tr(" (w %2)").arg(weight);
                    traits_str += tr("</dd></p>");

                    deviation = d->trait(trait_id.toInt()) - DwarfStats::get_trait_mean(trait_id.toInt());
                    if(a.is_neg)
                        deviation *= -1;
                    deviation /= DwarfStats::get_trait_stdev(trait_id.toInt());
                    deviation *= weight;
                    rating_trait += deviation;
                    total_weight += pow(weight,2);
                }
                traits_str += "</br>";
                avgMeanDev_traits = rating_trait / m_role->traits.count();
                avgStdev_traits = sqrt(total_weight / m_role->traits.count());
                rating_trait = DwarfStats::calc_cdf(0,avgStdev_traits,avgMeanDev_traits)*100;
            }
            //********************************


            //************ SKILLS ************
            if(m_role->skills.count()>0){
                skills_str=tr("Skills");
                if(skill_weight != default_skills_weight)
                    skills_str += tr(" (w %1)").arg(skill_weight);

                total_weight = 0;
                int skill_value = 0;
                deviation = 0;
                foreach(QString skill_id, m_role->skills.uniqueKeys()){
                    a = m_role->skills.value(skill_id);
                    weight = a.weight;

                    QString skill_name = GameDataReader::ptr()->get_skill_name(skill_id.toInt());
                    skill_name[0] = skill_name[0].toUpper();
                    skills_str += tr("<p><dd>%1 %2").arg((a.is_neg ? "<u>Not</u> " : "")).arg(skill_name);
                    if(weight != 1.0) //hide default weights
                        skills_str += tr(" (w %2)").arg(weight);
                    skills_str += tr("</dd></p>");

                    skill_value = d->skill_rating(skill_id.toInt());
                    if(skill_value < 0)
                        skill_value = 0;

                    deviation = skill_value - DwarfStats::get_skill_mean(skill_id.toInt());
                    float stdev = DwarfStats::get_skill_stdev(skill_id.toInt());
                    if(stdev != 0){
                    deviation /= stdev;
                    deviation *= weight;
                    }else{
                        deviation = 0;
                    }
                    rating_skill += deviation;
                    total_weight += pow(weight,2);

                }
                skills_str += "</br>";
                avgMeanDev_skills = rating_skill / m_role->skills.count();
                avgStdev_skills = sqrt(total_weight / m_role->skills.count());
                rating_skill = DwarfStats::calc_cdf(0,avgStdev_skills,avgMeanDev_skills)*100;
            }
            //********************************
            rating_total = ((rating_att*attrib_weight)+(rating_skill*skill_weight)+(rating_trait*trait_weight)) / (attrib_weight+trait_weight+skill_weight);

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

                aspects_str = "<h4>Aspects:</h4></br>";
                aspects_str += attribs_str;
                aspects_str += skills_str;
                aspects_str += traits_str;
                aspects_str += tr("<br><b>Note:</b> A higher weight (w) puts greater value on the aspect. Default weights are not shown.");
                match_str += "</br>" + aspects_str;

                item->setToolTip(QString("<h3>%1 - %3%</h3>%2<h4>%4 satisfies %3% of this role's aspects on average.</h4>")
                                 .arg(m_role->name)
                                 .arg(match_str)
                                 .arg(ceil(rating_total))
                                 .arg(d->nice_name()));

            }else{
                match_str = tr("Incapable of filling this role.<br><br>Value: %1</br>").arg(rating_total);
            }
        } else {
            match_str = tr("Scripted role.<br><br>Value: %1</br>").arg(rating_total);
            item->setToolTip(QString("<h3>%1 - %3</h3>%2<h4>%4</h4>").arg(m_role->name).arg(match_str).arg(ceil(rating_total)).arg(d->nice_name()));
        }
    }else{
        item->setData(0, DwarfModel::DR_RATING); //drawing value
        item->setData(0, DwarfModel::DR_SORT_VALUE);
        item->setData(CT_ROLE, DwarfModel::DR_COL_TYPE);
        //role wasn't initialized, give the user a useful error message in the tooltip
        item->setToolTip("<h4>Role could not be found.</h4>");
    }

    return item;
}

QStandardItem *RoleColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) {
    Q_UNUSED(group_name);
    Q_UNUSED(dwarves);
    QStandardItem *item = new QStandardItem;
    item->setData(m_bg_color, DwarfModel::DR_DEFAULT_BG_COLOR);
    return item;
}

void RoleColumn::read_settings() {
    if(m_role){
        //reset role's global weights to the new default weights, but only if they were using them in the first place
        QSettings *s = new QSettings(QSettings::IniFormat, QSettings::UserScope, COMPANY, PRODUCT, this);
        if(m_role->attributes_weight.weight !=0 && m_role->attributes_weight.is_default)
            m_role->attributes_weight.weight = s->value(QString("options/default_attributes_weight")).toFloat();
        if(m_role->traits_weight.weight !=0 && m_role->traits_weight.is_default)
            m_role->traits_weight.weight = s->value(QString("options/default_traits_weight")).toFloat();
        if(m_role->skills_weight.weight !=0 && m_role->skills_weight.is_default)
            m_role->skills_weight.weight = s->value(QString("options/default_skills_weight")).toFloat();
    }
}

