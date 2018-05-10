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

#include "viewcolumnset.h"
#include "columntypes.h"
#include "laborcolumn.h"
#include "happinesscolumn.h"
#include "spacercolumn.h"
#include "skillcolumn.h"
#include "flagcolumn.h"
#include "currentjobcolumn.h"
#include "traitcolumn.h"
#include "attributecolumn.h"
#include "rolecolumn.h"
#include "truncatingfilelogger.h"
#include "dwarftherapist.h"
#include "gridviewdialog.h"
#include "weaponcolumn.h"
#include "professioncolumn.h"
#include "highestmoodcolumn.h"
#include "trainedcolumn.h"
#include "healthcolumn.h"
#include "equipmentcolumn.h"
#include "itemtypecolumn.h"
#include "superlaborcolumn.h"
#include "customprofessioncolumn.h"
#include "beliefcolumn.h"
#include "unitkillscolumn.h"
#include "preferencecolumn.h"
#include "viewcolumnsetcolors.h"

ViewColumnSet::ViewColumnSet(QString name, QObject *parent)
    : QObject(parent)
    , m_name(name)
    , m_bg_color(Qt::white)
{
    m_cell_colors = new ViewColumnSetColors(this);
    connect_settings();
}

ViewColumnSet::ViewColumnSet(const ViewColumnSet &copy)
    : QObject(copy.parent())
    , m_name(copy.m_name)
    , m_bg_color(copy.m_bg_color)
    , m_cell_colors(copy.m_cell_colors)
{
    connect_settings();
    foreach(ViewColumn *vc, copy.m_columns) {
        ViewColumn *new_c = vc->clone();
        new_c->setParent(this);
        new_c->set_viewcolumnset(this);
        add_column(new_c); // manually add a cloned copy
    }
}

ViewColumnSet::ViewColumnSet(QSettings &s, QObject *parent, int set_num)
    : QObject(parent)
{
    m_name = s.value("name","unknown").toString();
    set_bg_color(s.value("bg_color", QColor(Qt::white)).value<QColor>());
    m_cell_colors = new ViewColumnSetColors(s,this);
    connect_settings();
    if(set_num == 0)
        new SpacerColumn(0,0, this, parent);

    int total_columns = s.beginReadArray("columns");
    for (int i = 0; i < total_columns; ++i) {
        s.setArrayIndex(i);
        COLUMN_TYPE cType = (get_column_type(s.value("type", "DEFAULT").toString()));
        switch(cType) {
        case CT_SPACER:
            new SpacerColumn(s, this, parent);
            break;
        case CT_HAPPINESS:
            new HappinessColumn(s, this, parent);
            break;
        case CT_LABOR:
            new LaborColumn(s, this, parent);
            break;
        case CT_SKILL:
            new SkillColumn(s, this, parent);
            break;
        case CT_IDLE:
            new CurrentJobColumn(s, this, parent);
            break;
        case CT_TRAIT:
            new TraitColumn(s, this, parent);
            break;
        case CT_ATTRIBUTE:
            new AttributeColumn(s, this, parent);
            break;
        case CT_FLAGS:
            new FlagColumn(s, this, parent);
            break;
        case CT_ROLE:
            new RoleColumn(s, this, parent);
            break;
        case CT_WEAPON:
            new WeaponColumn(s,this,parent);
            break;
        case CT_PROFESSION:
            new ProfessionColumn(s, this, parent);
            break;
        case CT_HIGHEST_MOOD:
            new HighestMoodColumn(s, this, parent);
            break;
        case CT_TRAINED:
            new TrainedColumn(s, this, parent);
            break;
        case CT_HEALTH:
            new HealthColumn(s,this,parent);
            break;
        case CT_EQUIPMENT:
            new EquipmentColumn(s,this,parent);
            break;
        case CT_ITEMTYPE:
            new ItemTypeColumn(s,this,parent);
            break;
        case CT_SUPER_LABOR:
            new SuperLaborColumn(s,this,parent);
            break;
        case CT_CUSTOM_PROFESSION:
            new CustomProfessionColumn(s,this,parent);
            break;
        case CT_BELIEF:
            new BeliefColumn(s,this,parent);
            break;
        case CT_KILLS:
            new UnitKillsColumn(s, this, parent);
            break;
        case CT_PREFERENCE:
            new PreferenceColumn(s, this, parent);
            break;
        case CT_DEFAULT:
        default:
            if(set_num != 0 || total_columns-1 != i){
                LOGW << "unidentified column type in set" << this->name() << "!";
            }
            break;
        }
    }
    s.endArray();
}

void ViewColumnSet::connect_settings(){
    connect(DT,SIGNAL(settings_changed()),SLOT(read_settings()));
}

ViewColumnSet::~ViewColumnSet(){
    m_cell_colors = 0;
    foreach(ViewColumn *c, m_columns){
        c->deleteLater();
    }
    m_columns.clear();
}

void ViewColumnSet::re_parent(QObject *parent) {
    setParent(parent);
    foreach(ViewColumn *vc, m_columns) {
        vc->setParent(parent);
    }
}

void ViewColumnSet::set_name(const QString &name) {
    m_name = name;
}

void ViewColumnSet::add_column(ViewColumn *col,int idx) {
    bool name_ok = false;
    while (!name_ok) {
        name_ok = true;
        foreach(ViewColumn *vc, m_columns) { // protect against duplicate names
            QString title = col->title();
            if (title == vc->title()) {
                // col is a dupe, see if it ends with a number
                QRegExp regex("([a-zA-Z0-9\\[\\]. ]*)\\s*(\\d+)$");
                QString stripped_title = title; // title without an ending number
                int num = 0;
                int pos = regex.indexIn(title);
                if (pos != -1) {
                    stripped_title = regex.cap(1).trimmed();
                    num = regex.cap(2).toInt();
                }
                col->set_title(QString("%1 %2").arg(stripped_title).arg(num + 1));
                name_ok = false;
                break;
            }
        }
    }
    if(idx == -1)
        m_columns << col;
    else
        m_columns.insert(idx,col);
}

void ViewColumnSet::reorder_columns(const QStandardItemModel &model) {
    QList<ViewColumn*> new_cols;
    for (int i = 0; i < model.rowCount(); ++i) {
        new_cols << model.item(i, 0)->data().value<ViewColumn *>();
    }
    Q_ASSERT(new_cols.size() == m_columns.size());

    m_columns = std::move(new_cols);
}

void ViewColumnSet::read_settings(){
    m_cell_colors->read_settings();
}

void ViewColumnSet::write_to_ini(QSettings &s, int start_idx) {
    s.setValue("name", m_name);
    s.setValue("bg_color", m_bg_color);
    m_cell_colors->write_to_ini(s);
    s.beginWriteArray("columns", m_columns.size());
    int i = 0;
    for(int idx=start_idx;idx < m_columns.count(); idx++){
        s.setArrayIndex(i++);
        m_columns.at(idx)->write_to_ini(s);
    }
    s.endArray();
}
