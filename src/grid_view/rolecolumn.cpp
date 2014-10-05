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
#include "labor.h"
#include "defines.h"

RoleColumn::RoleColumn(const QString &title, Role *r, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title,CT_ROLE,set,parent)
    , m_role(r)
    , m_role_name("")
{
    if(m_role)
        m_role_name = r->name();
    else
        m_role_name = m_title;
    connect(DT, SIGNAL(roles_changed()), this, SLOT(roles_changed()), Qt::UniqueConnection);
}

RoleColumn::RoleColumn(const RoleColumn &to_copy)
    : ViewColumn(to_copy)
    , m_role(to_copy.m_role)
    , m_role_name(to_copy.m_role_name)
{
    connect(DT, SIGNAL(roles_changed()), this, SLOT(roles_changed()), Qt::UniqueConnection);
}

RoleColumn::RoleColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s, set, parent)
    , m_role_name("")
{
    //older versions required the column title to match the role name, so fall back to that if 'role_id' doesn't exist
    m_role = GameDataReader::ptr()->get_role(s.value("role_id",s.value("name").toString()).toString());
    if(m_role)
        m_role_name = m_role->name();
    else
        m_role_name = m_title;
    connect(DT, SIGNAL(roles_changed()), this, SLOT(roles_changed()), Qt::UniqueConnection);
}

RoleColumn::~RoleColumn(){
    m_role = 0;
}

QStandardItem *RoleColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);
    //defaults
    item->setData(-1, DwarfModel::DR_RATING);
    item->setData(-1, DwarfModel::DR_DISPLAY_RATING);
    item->setData(CT_ROLE, DwarfModel::DR_COL_TYPE);
    item->setData(-1,DwarfModel::DR_LABORS);

    if(d->is_baby()){
        item->setData(-2, DwarfModel::DR_SORT_VALUE);
        item->setToolTip(("<b>Babies aren't included in role calculations.</b>"));
        return item;
    }else if(d->is_child() && !DT->labor_cheats_allowed()){
        item->setData(-1, DwarfModel::DR_SORT_VALUE);
        item->setToolTip(("<b>Children are only included in role calculations if labor cheats are enabled.</b>"));
        return item;
    }

    if(m_role){
        float raw_rating = d->get_raw_role_rating(m_role->name());
        float drawn_rating = d->get_role_rating(m_role->name());
        if(drawn_rating < 0.0001)
            drawn_rating = 0.0001; //just to ensure very low values are drawn
        item->setData(drawn_rating, DwarfModel::DR_RATING);
        item->setData(roundf(drawn_rating), DwarfModel::DR_DISPLAY_RATING);
        item->setData(raw_rating, DwarfModel::DR_SORT_VALUE);
        item->setData(CT_ROLE, DwarfModel::DR_COL_TYPE);
        set_export_role(DwarfModel::DR_SORT_VALUE);

        QList<QVariant> related_labors;
        QStringList labor_names;

        foreach(int labor_id, m_role->get_labors()){
            Labor *l = GameDataReader::ptr()->get_labor(labor_id);
            labor_names.append(l->name);
            related_labors.append(labor_id);
        }

        QString labors_desc = "";
        labors_desc = QString("<br/><br/><b>Associated Labors:</b> %1").arg(labor_names.count() <= 0 ? "None" : labor_names.join(", "));
        item->setData(related_labors,DwarfModel::DR_LABORS);

        QString match_str;
        QString aspects_str;
        QString tooltip;
        if (m_role->script() == "") {
            if(raw_rating >= 0){
                aspects_str = m_role->get_role_details(d);
                aspects_str += tr("<br/><b>Note:</b> A higher weight (w) puts greater value on the aspect. Default weights are not shown.");
                match_str += aspects_str;

                tooltip = QString("<center><h3>%1 - %3%</h3></center>%2%5<center><h4 font-weight:normal>%4 is a %3% fit for this role.</h4></center>")
                        .arg(m_role->name())
                        .arg(match_str)
                        .arg(QString::number(drawn_rating,'f',2))
                        .arg(d->nice_name())
                        .arg(labors_desc);

                item->setToolTip(tooltip);


            }else{
                match_str = tr("Incapable of filling this role.<br><br>Value: %1<br/>").arg(QString::number(raw_rating,'f',2));
            }
        } else {
            match_str = tr("%1<h4><b>Raw Rating:</b> %2</h4>")
                    .arg(m_role->get_role_details())
                    .arg(raw_rating, 0, 'f', 2);
            tooltip = QString("<center><h3>%1 - %3</h3></center>%2%4")
                             .arg(m_role->name())
                             .arg(match_str)
                             .arg(roundf(raw_rating), 0, 'f', 0)
                             .arg(tooltip_name_footer(d));

            item->setToolTip(tooltip);
        }
    }else{
        item->setData(-1, DwarfModel::DR_RATING); //drawing value
        item->setData(-1, DwarfModel::DR_SORT_VALUE);
        item->setData(CT_ROLE, DwarfModel::DR_COL_TYPE);
        //role wasn't initialized, give the user a useful error message in the tooltip
        item->setToolTip("Role could not be found.");
    }

    return item;
}

QStandardItem *RoleColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) {
    Q_UNUSED(dwarves);
    QStandardItem *item = init_aggregate(group_name);
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
        m_role_name = m_role->name();
        delete(s);
        s = 0;
    }
}

void RoleColumn::roles_changed(){
    //see if perhaps we have a new role created that fits the target or missing role
    //or if our role has been updated
    if(!m_role || GameDataReader::ptr()->get_role(m_role_name) != m_role)
        m_role = GameDataReader::ptr()->get_role(m_role_name);
}

void RoleColumn::write_to_ini(QSettings &s) {
    ViewColumn::write_to_ini(s);
    if(m_role)
        s.setValue("role_id", m_role->name());
}
