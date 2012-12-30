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
#include "rolecolumn.h"
#include "gamedatareader.h"
#include "viewcolumnset.h"
#include "columntypes.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "trait.h"
#include "dwarftherapist.h"
#include "truncatingfilelogger.h"

RoleColumn::RoleColumn(const QString &title, Role *r, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title,CT_ROLE,set,parent)
    ,m_role(r)
{
    connect(DT, SIGNAL(roles_changed()), this, SLOT(roles_changed()), Qt::UniqueConnection);
}

RoleColumn::RoleColumn(const RoleColumn &to_copy)
    : ViewColumn(to_copy)
    , m_role(to_copy.m_role)
{
    connect(DT, SIGNAL(roles_changed()), this, SLOT(roles_changed()), Qt::UniqueConnection);
}

RoleColumn::RoleColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s, set, parent)
{
    m_role = GameDataReader::ptr()->get_role(s.value("name").toString());
    connect(DT, SIGNAL(roles_changed()), this, SLOT(roles_changed()), Qt::UniqueConnection);
}

QStandardItem *RoleColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);

    if(d->profession()=="Baby"){
        item->setData(-20, DwarfModel::DR_RATING); //drawing value
        item->setData(-20, DwarfModel::DR_SORT_VALUE);
        item->setData(CT_ROLE, DwarfModel::DR_COL_TYPE);
        item->setToolTip(("<b>Babies aren't included in role calculations.</b>"));
        return item;
    }

    if(m_role){

        float rating_total = d->get_role_rating(m_role->name);
        float drawRating = 0.0;
        if(rating_total >=0 && rating_total <= 100)
            drawRating = (rating_total-50)/3.3;

        item->setData((int)drawRating, DwarfModel::DR_RATING); //drawing value
        item->setData(rating_total, DwarfModel::DR_SORT_VALUE);
        item->setData(CT_ROLE, DwarfModel::DR_COL_TYPE);

        QString match_str;
        QString aspects_str;
        QString tooltip;
        if (m_role->script == "") {
            if(rating_total >= 0){
                aspects_str = m_role->get_role_details();
                aspects_str += tr("<br/><b>Note:</b> A higher weight (w) puts greater value on the aspect. Default weights are not shown.");
                match_str += aspects_str;

                tooltip = QString("<h3>%1 - %3%</h3>%2<h4>%4 is a %3% fit for this role.</h4>")
                        .arg(m_role->name)
                        .arg(match_str)
                        .arg(QString::number(rating_total,'f',2))
                        .arg(d->nice_name());

                item->setToolTip(tooltip);


            }else{
                match_str = tr("Incapable of filling this role.<br><br>Value: %1<br/>").arg(QString::number(rating_total,'f',2));
            }
        } else {
            match_str = tr("Scripted role.<br/><br/>Value: %1<br/>").arg(QString::number(rating_total,'f',2));
            tooltip = QString("<h3>%1 - %3</h3>%2<h4>%4</h4>")
                             .arg(m_role->name)
                             .arg(match_str)
                             .arg(QString::number(ceil(rating_total),'g',2))
                             .arg(d->nice_name());

            item->setToolTip(tooltip);
        }
    }else{
        item->setData(0, DwarfModel::DR_RATING); //drawing value
        item->setData(0, DwarfModel::DR_SORT_VALUE);
        item->setData(CT_ROLE, DwarfModel::DR_COL_TYPE);
        //role wasn't initialized, give the user a useful error message in the tooltip
        item->setToolTip("Role could not be found.");
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
        if(m_role->attributes_weight.is_default)
            m_role->attributes_weight.weight = s->value(QString("options/default_attributes_weight")).toFloat();
        if(m_role->traits_weight.is_default)
            m_role->traits_weight.weight = s->value(QString("options/default_traits_weight")).toFloat();
        if(m_role->skills_weight.is_default)
            m_role->skills_weight.weight = s->value(QString("options/default_skills_weight")).toFloat();
        if(m_role->prefs_weight.is_default)
            m_role->prefs_weight.weight = s->value(QString("options/default_prefs_weight")).toFloat();

        m_role->create_role_details(*s); //rebuild the description
        delete(s);
        s = 0;
    }
}

void RoleColumn::roles_changed(){
    //see if perhaps we have a new role created that fits the missing role
    //or if our role has been updated
    if(!m_role || GameDataReader::ptr()->get_role(this->title()) != m_role)
        m_role = GameDataReader::ptr()->get_role(this->title());

    //update the column header if the role's name has changed
    if(m_role){
        if(m_role->name != this->title())
            this->set_title(m_role->name);
    }
}

