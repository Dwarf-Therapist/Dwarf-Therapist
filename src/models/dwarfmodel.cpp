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
#include <QtCore>
#include <QtDebug>

#include "dfinstance.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "skill.h"
#include "labor.h"
#include "profession.h"
#include "squad.h"
#include "statetableview.h"
#include "truncatingfilelogger.h"
#include "dwarftherapist.h"

#include "columntypes.h"
#include "gridview.h"
#include "viewcolumnset.h"
#include "viewcolumn.h"
#include "laborcolumn.h"
#include "skillcolumn.h"
#include "spacercolumn.h"
#include "races.h"
#include "fortressentity.h"

QStringList DwarfModel::m_seasons;
QStringList DwarfModel::m_months;

DwarfModel::DwarfModel(QObject *parent)
    : QStandardItemModel(parent)
    , m_df(0)
    , m_group_by(GB_NOTHING)
    , m_selected_col(-1)
{
}

DwarfModel::~DwarfModel() {
    clear_all();
}

void DwarfModel::clear_all() {
    clear_pending();
    foreach(Dwarf *d, m_dwarves) {
        delete d;
    }
    m_dwarves.clear();
    m_grouped_dwarves.clear();
    clear();
}

void DwarfModel::section_right_clicked(int col) {
    if (col == m_selected_col) {
        m_selected_col = -1; // turn off the guides
    } else {
        m_selected_col = col;
    }
    emit dataChanged(index(0, col), index(rowCount()-1, col));
}

void DwarfModel::load_dwarves() {
    // clear id->dwarf map
    foreach(Dwarf *d, m_dwarves) {
        delete d;
    }
    m_dwarves.clear();
    if (rowCount())
        removeRows(0, rowCount());

    m_df->attach();


    foreach(Dwarf *d, m_df->load_dwarves()) {
        m_dwarves[d->id()] = d;
    }


    m_squads.clear();
    foreach(Squad * s, m_df->load_squads()) {
        m_squads[s->id()] = s;        
    }
    m_df->detach();
}

void DwarfModel::update_header_info(int id, COLUMN_TYPE type){
    int index = 0;
    QSettings *s = DT->user_settings();
    foreach(ViewColumnSet *set, m_gridview->sets()) {
        foreach(ViewColumn *col, set->columns()) {
            index ++;
            if(col->type() == type){
                switch(col->type()){
                case CT_LABOR:
                {
                    LaborColumn *l = static_cast<LaborColumn*>(col);
                    if(l->labor_id()==id){
                        l->update_count(); //tell this column to update it's count
                        int cnt = l->count();
                        QStandardItem* header = this->horizontalHeaderItem(index);
                        header->setData(col->bg_color(), Qt::BackgroundColorRole);
                        header->setData(set->name(), Qt::UserRole);
                        if(s->value("options/grid/show_labor_counts",false).toBool())
                            header->setText(QString("%1 %2").arg(cnt).arg(col->title()).trimmed());
                        header->setToolTip(tr("%1 have this labor enabled.").arg(cnt));
                        return;
                    }
                }
                }
            }
        }
    }
}

void DwarfModel::draw_headers(){
    int start_col = 1;
    setHorizontalHeaderItem(0, new QStandardItem);
    emit clear_spacers();
    QSettings *s = DT->user_settings();
    int width = s->value("options/grid/cell_size", DEFAULT_CELL_SIZE).toInt();
    foreach(ViewColumnSet *set, m_gridview->sets()) {
        /*QStandardItem *set_header = new QStandardItem(set->name());
        set_header->setData(set->bg_color(), Qt::BackgroundColorRole);
        set_header->setData(true);
        setHorizontalHeaderItem(start_col++, set_header);
        emit set_index_as_spacer(start_col - 1);
        emit preferred_header_size(start_col - 1, width);*/
        foreach(ViewColumn *col, set->columns()) {
            QString cnt = "";
            QString h_name = col->title();
            if(col->type()==CT_LABOR){
                cnt = col->count() >= 0 ? QString::number(col->count()) : "";
                if(s->value("options/grid/show_labor_counts",false).toBool())
                    h_name = QString("%1 %2").arg(cnt).arg(col->title()).trimmed();
            }
            QStandardItem *header = new QStandardItem(h_name);
            if(col->type()==CT_LABOR)
                header->setToolTip(tr("%1 have this labor enabled.").arg(cnt));

            header->setData(col->bg_color(), Qt::BackgroundColorRole);
            header->setData(set->name(), Qt::UserRole);
            setHorizontalHeaderItem(start_col++, header);
            switch (col->type()) {
            case CT_SPACER:
            {
                SpacerColumn *c = static_cast<SpacerColumn*>(col);
                emit set_index_as_spacer(start_col - 1);
                emit preferred_header_size(start_col - 1, c->width());
            }
                break;
            default:
                emit preferred_header_size(start_col - 1, width);
            }
        }
    }
}

void DwarfModel::build_rows() {
    // don't need to go delete the dwarf pointers in here, since the earlier foreach should have
    // deleted them
    m_grouped_dwarves.clear();
    clear();
    draw_headers();
    QSettings *s = DT->user_settings();

    // populate dwarf maps
    bool only_animals = s->value("read_animals",false).toBool();
    int n_adults=0;
    int n_children=0;
    int n_babies=0;
    QString race_name = "";

    //setup calendar
    build_calendar();

    foreach(Dwarf *d, m_dwarves) {
        if (only_animals)
        {
            race_name = "Animals";
            if(d->is_animal())
            {
                switch (m_group_by) {
                default:
                case GB_NOTHING:
                    m_grouped_dwarves[QString::number(d->id())].append(d);
                    break;
                case GB_SEX:
                    if (d->is_male())
                        m_grouped_dwarves[tr("Males")].append(d);
                    else
                        m_grouped_dwarves[tr("Females")].append(d);
                    break;
                case GB_MIGRATION_WAVE:
                    m_grouped_dwarves[get_migration_desc(d)].append(d);
                    break;
                case GB_PROFESSION:
                case GB_LEGENDARY:
                case GB_HAPPINESS:
                case GB_CASTE:
                    m_grouped_dwarves[d->caste_name()].append(d);
                    break;
                case GB_CURRENT_JOB:
                case GB_MILITARY_STATUS:
                case GB_HIGHEST_SKILL:
                case GB_TOTAL_SKILL_LEVELS:
                case GB_ASSIGNED_LABORS:
                case GB_RACE:
                {
                    QString grp_name = d->race_name(true);
                    if(d->profession() != "")
                        grp_name += " (" + d->race_name(false) + ")";
                    m_grouped_dwarves[grp_name].append(d);
                    break;
                }
                case GB_HAS_NICKNAME:
                {
                    if (d->nickname().isEmpty()) {
                        m_grouped_dwarves[tr("No Nickname")].append(d);
                    } else {
                        m_grouped_dwarves[tr("Has Nickname")].append(d);
                    }
                }
                    break;
                case GB_SQUAD:
                {
                    if(d->squad_name().isEmpty()) {
                        m_grouped_dwarves[tr("No Squad")].append(d);
                    } else {
                        m_grouped_dwarves[d->squad_name()].append(d);
                    }
                }
                    break;
                }

                if(d->profession()=="Child")
                    n_children ++;
                else if(d->profession()=="Baby")
                    n_babies ++;
                else
                    n_adults ++;
            }
        }
        else
        {
            if(!d->is_animal())
            {
                if(race_name.isEmpty()){
                    Race* r = m_df->get_race(d->get_race_id());
                    race_name = r->plural_name();
                }

                switch (m_group_by) {
                default:
                case GB_NOTHING:
                    m_grouped_dwarves[QString::number(d->id())].append(d);
                    break;
                case GB_PROFESSION:
                    m_grouped_dwarves[d->profession()].append(d);
                    break;
                case GB_LEGENDARY:
                {
                    int legendary_skills = 0;
                    foreach(Skill s, *d->get_skills()) {
                        if (s.rating() >= 15)
                            legendary_skills++;
                    }
                    if (legendary_skills)
                        m_grouped_dwarves[tr("Legends")].append(d);
                    else
                        m_grouped_dwarves[tr("Losers")].append(d);
                }
                    break;
                case GB_SEX:
                    if (d->is_male())
                        m_grouped_dwarves[tr("Males")].append(d);
                    else
                        m_grouped_dwarves[tr("Females")].append(d);
                    break;
                case GB_HAPPINESS:
                    m_grouped_dwarves[d->happiness_name(d->get_happiness())].append(d);
                    break;
                case GB_MIGRATION_WAVE:
                {
                    if(DT->user_settings()->value("options/hide_children_and_babies",true).toBool() == false
                        || (d->profession()!="Child" && d->profession()!="Baby"))
                        m_grouped_dwarves[get_migration_desc(d)].append(d);
                    break;
                }
                case GB_CASTE:
                    m_grouped_dwarves[d->caste_name()].append(d);
                    break;
                case GB_RACE:
                    m_grouped_dwarves[d->race_name(true)].append(d);
                    break;

                case GB_CURRENT_JOB:
                    m_grouped_dwarves[d->current_job()].append(d);
                    break;
                case GB_MILITARY_STATUS:
                {
                    // groups
                    if (d->profession() == "Baby" ||
                            d->profession() == "Child") {
                        m_grouped_dwarves[tr("Juveniles")].append(d);
                    } else if (d->active_military() && !d->can_set_labors()) { // epic military
                        m_grouped_dwarves[tr("Champions")].append(d);
                    } else if (!d->can_set_labors()) {
                        m_grouped_dwarves[tr("Nobles")].append(d);
                    } else if (d->active_military()) {
                        m_grouped_dwarves[tr("Active Military")].append(d);
                    } else {
                        m_grouped_dwarves[tr("Can Activate")].append(d);
                    }
                    /*
                            4a) Heroes and Champions (who cannot deactivate)
                            4b) Non-Heroic Soldiers and Guards (who can deactivate)
                            4c) Civilians (who can activate)
                            4d) Juveniles (who may one day activate)
                            4e) Immigrant Nobles (who are forever off-limits)
                            */
                }
                    break;
                case GB_HIGHEST_SKILL:
                {
                    Skill highest = d->highest_skill();
                    GameDataReader *gdr = GameDataReader::ptr();
                    QString level = gdr->get_skill_level_name(highest.rating());
                    m_grouped_dwarves[level].append(d);
                }
                    break;
                case GB_TOTAL_SKILL_LEVELS:
                    m_grouped_dwarves[tr("Levels: %1").arg(d->total_skill_levels())]
                            .append(d);
                    break;
                case GB_ASSIGNED_LABORS:
                    m_grouped_dwarves[tr("%1 Assigned Labors")
                            .arg(d->total_assigned_labors())].append(d);
                    break;
                case GB_HAS_NICKNAME:
                {
                    if (d->nickname().isEmpty()) {
                        m_grouped_dwarves[tr("No Nickname")].append(d);
                    } else {
                        m_grouped_dwarves[tr("Has Nickname")].append(d);
                    }
                }
                    break;
                case GB_SQUAD:
                {
                    if(d->squad_name().isEmpty()) {
                        m_grouped_dwarves[tr("No Squad")].append(d);
                    } else {
                        m_grouped_dwarves[d->squad_name()].append(d);
                    }
                }
                    break;
                }

                if(d->profession()=="Child")
                    n_children ++;
                else if(d->profession()=="Baby")
                    n_babies ++;
                else
                    n_adults ++;
            }
        }
    }

    foreach(QString key, m_grouped_dwarves.uniqueKeys()) {
        build_row(key);
    }
    emit new_creatures_count(n_adults,n_children,n_babies,race_name);
}

QString DwarfModel::get_migration_desc(Dwarf *d){
    qint32 wave = d->migration_wave();
    quint32 season = 0;
    quint32 year = 0;
    quint32 month = 0;
    quint32 day = 0;
    QString suffix = "th";

    day = (wave % 100) + 1;
    month = (wave / 100) % 100;
    season = (wave / 10000) % 10;
    year = wave / 100000;
    if ((day == 1) || (day == 21))
        suffix = "st";
    else if ((day == 2) || (day == 22))
        suffix = "nd";
    else if ((day == 3) || (day == 23))
        suffix = "rd";
    else
        suffix = "th";
    if (d->born_in_fortress())
    {
        return QString("Born on the %1%4 of %2 in the year %3").arg(day).arg(m_months.at(month)).arg(year).arg(suffix);
    }
    else
    {
        return QString("Arrived in the %2 of the year %1").arg(year).arg(m_seasons.at(season));
    }
    return "Unknown Arrival";
}

void DwarfModel::build_calendar(){
    if(m_seasons.length()<=0 || m_months.length()<=0){
        m_seasons.append("Spring");
        m_seasons.append("Summer");
        m_seasons.append("Autumn");
        m_seasons.append("Winter");

        m_months.append("Granite");
        m_months.append("Slate");
        m_months.append("Felsite");
        m_months.append("Hematite");
        m_months.append("Malachite");
        m_months.append("Galena");
        m_months.append("Limestone");
        m_months.append("Sandstone");
        m_months.append("Timber");
        m_months.append("Moonstone");
        m_months.append("Opal");
        m_months.append("Obsidian");
    }
}

void DwarfModel::build_row(const QString &key) {
    QIcon icn_f(":img/female.png");
    QIcon icn_m(":img/male.png");
    QStandardItem *root = 0;
    QList<QStandardItem*> root_row;
    Dwarf *first_dwarf = m_grouped_dwarves.value(key).at(0);
    if (!first_dwarf) {
        LOGE << "'Group by'' set for" << key << "has a bad ref for its first "
                << "dwarf";
        return;
    }

    if (m_group_by != GB_NOTHING) {
        // we need a root element to hold group members...
        QString title = QString("%1 (%2)").arg(key).arg(m_grouped_dwarves.value(key).size());
        root = new QStandardItem(title);
        root->setData(true, DR_IS_AGGREGATE);
        root->setData(key, DR_GROUP_NAME);
        root->setData(0, DR_RATING);        
        //root->setData(title, DR_SORT_VALUE);
        // for integer based values we want to make sure they sort by the int
        // values instead of the string values
        if (m_group_by == GB_MIGRATION_WAVE) {
            root->setData(first_dwarf->migration_wave(), DR_SORT_VALUE);
        } else if (m_group_by == GB_HIGHEST_SKILL) {
            root->setData(first_dwarf->highest_skill().rating(), DR_SORT_VALUE);
        } else if (m_group_by == GB_TOTAL_SKILL_LEVELS) {
            root->setData(first_dwarf->total_skill_levels(), DR_SORT_VALUE);
        } else if (m_group_by == GB_HAPPINESS) {
            root->setData(first_dwarf->get_happiness(), DR_SORT_VALUE);
        } else if (m_group_by == GB_ASSIGNED_LABORS) {
            root->setData(first_dwarf->total_assigned_labors(), DR_SORT_VALUE);
        } else if (m_group_by == GB_CASTE) {
            root->setData(first_dwarf->get_caste_id(), DR_SORT_VALUE);
        } else if (m_group_by == GB_SQUAD){
            root->setData(first_dwarf->squad_id(), DR_ID);
        }
        root_row << root;
    }

    if (root) { // we have a parent, so we should draw an aggregate row
        foreach(ViewColumnSet *set, m_gridview->sets()) {
            foreach(ViewColumn *col, set->columns()) {
                QStandardItem *item = col->build_aggregate(key, m_grouped_dwarves[key]);
                root_row << item;
            }
        }
    }

    QStandardItem *i_name = new QStandardItem(first_dwarf->nice_name());
    QFont f = i_name->font();
    QFontMetrics fm(f);
    QChar symbol(0x263C);
    if(!fm.inFont(QChar(0x263C))){
        symbol = QChar(0x2261);
        if(!fm.inFont(symbol))
            symbol = QChar(0x002A);
    }

    foreach(Dwarf *d, m_grouped_dwarves.value(key)) {
        i_name = new QStandardItem(d->nice_name());

        //font settings ***
        if (d->active_military()) {            
            f.setBold(true);
        }
        if((m_group_by==GB_SQUAD && d->squad_position()==0) || d->noble_position() != ""){
            i_name->setText(QString("%1 %2 %1").arg(symbol).arg(i_name->text()));
            f.setItalic(true);            
        }
        i_name->setFont(f);
        //******

        //background gradients for curses, nobles, etc.
        if(d && DT->user_settings()->value("options/highlight_nobles",false).toBool()){
            if(d->noble_position() != "")
                i_name->setData(build_gradient_brush(m_df->fortress()->get_noble_color(d->historical_id())
                                                     ,200,0,QPoint(0,0),QPoint(1,0)),Qt::BackgroundRole);
        }
        if(d && DT->user_settings()->value("options/highlight_cursed",false).toBool()){
            if(d->curse_name() != ""){ //QChar(0x2261) =
                //i_name->setText(QString("%1 %2 %1").arg(QChar(0x203C)).arg(i_name->text()));
                f.setItalic(true);
                i_name->setData(build_gradient_brush(DT->user_settings()->value("options/colors/cursed", QColor(125,97,186)).value<QColor>()
                                                     ,200,0,QPoint(0,0),QPoint(1,0)),Qt::BackgroundRole);
            }
        }


        i_name->setToolTip(d->tooltip_text());
        i_name->setStatusTip(d->nice_name());
        i_name->setData(false, DR_IS_AGGREGATE);
        i_name->setData(0, DR_RATING);
        i_name->setData(d->id(), DR_ID);        
        QVariant sort_val;
        switch(m_group_by) {
        case GB_PROFESSION:
            sort_val = d->raw_profession();
            break;
        case GB_HAPPINESS:
            sort_val = d->get_raw_happiness();
            break;
        case GB_SQUAD:
            sort_val = d->squad_position();
            if(sort_val.toInt() < 0)
                sort_val = d->nice_name();
            break;
        case GB_NOTHING:
        default:
            sort_val = d->nice_name();
            break;
        }

        i_name->setData(sort_val, DR_SORT_VALUE);

        if (d->is_male()) {
            i_name->setIcon(icn_m);
        } else {
            i_name->setIcon(icn_f);
        }

        QList<QStandardItem*> items;
        items << i_name;
        foreach(ViewColumnSet *set, m_gridview->sets()) {
            foreach(ViewColumn *col, set->columns()) {
                QStandardItem *item = col->build_cell(d);
                items << item;
            }
        }

        if (root) {
            root->appendRow(items);
        } else {
            appendRow(items);
        }
        d->m_name_idx = indexFromItem(i_name);
    }
    if (root) {
        appendRow(root_row);
    }
}

void DwarfModel::cell_activated(const QModelIndex &idx) {
    QStandardItem *item = itemFromIndex(idx);
    bool is_aggregate = item->data(DR_IS_AGGREGATE).toBool();
    if (idx.column() == 0) {
        if (is_aggregate)
            return; // no double clicking aggregate names
        int dwarf_id = item->data(DR_ID).toInt(); // TODO: handle no id
        if (!dwarf_id) {
            LOGW << "double clicked what should have been a dwarf name, but the ID wasn't set!";
            return;
        }
        Dwarf *d = get_dwarf_by_id(dwarf_id);
        d->show_details();
        return;
    }

    COLUMN_TYPE type = static_cast<COLUMN_TYPE>(idx.data(DwarfModel::DR_COL_TYPE).toInt());
    if (type != CT_LABOR && type != CT_MILITARY_PREFERENCE && type != CT_FLAGS)
        return;

    Q_ASSERT(item);

    int labor_id = item->data(DR_LABOR_ID).toInt();
    int dwarf_id = item->data(DR_ID).toInt(); // TODO: handle no id
    if (is_aggregate) {
        QModelIndex first_col = idx.sibling(idx.row(), 0);

        // first find out how many are enabled...
        int enabled_count = 0;
        int settable_dwarves = 0;
        QString group_name = idx.data(DwarfModel::DR_GROUP_NAME).toString();

        foreach(Dwarf *d, m_grouped_dwarves.value(group_name)) {
            if (d->can_set_labors() || DT->labor_cheats_allowed()) {
                settable_dwarves++;
                if (d->labor_enabled(labor_id))
                    enabled_count++;
            }
        }

        // if none or some are enabled, enable all of them
        bool enabled = (enabled_count < settable_dwarves);
        foreach(Dwarf *d, m_grouped_dwarves.value(group_name)) {            
            d->set_labor(labor_id, enabled, false);
        }

        // tell the view what we touched...
        emit dataChanged(idx, idx);
        QModelIndex left = index(0, 0, first_col);
        QModelIndex right = index(rowCount(first_col) - 1, columnCount(first_col) - 1, first_col);
        emit dataChanged(left, right); // tell the view we changed every dwarf under this agg to pick up implicit exclusive changes
        DT->emit_labor_counts_updated(); //update column header text
    } else {

        if (!dwarf_id) {
            LOGW << "dwarf_id was 0 for cell at" << idx << "!";
        } else {
            QModelIndex left = index(idx.parent().row(), 0, idx.parent().parent());
            QModelIndex right = index(idx.parent().row(), columnCount(idx.parent()) - 1, idx.parent().parent());
            emit dataChanged(left, right); // update the agg row

            left = index(idx.row(), 0, idx.parent());
            right = index(idx.row(), columnCount(idx.parent()) - 1, idx.parent());
            emit dataChanged(left, right); // update the dwarf row
            if (type == CT_LABOR)
                m_dwarves[dwarf_id]->toggle_labor(labor_id);
            else if (type == CT_MILITARY_PREFERENCE)
                m_dwarves[dwarf_id]->toggle_pref_value(labor_id);
            else if (type == CT_FLAGS)
                m_dwarves[dwarf_id]->toggle_flag_bit(labor_id);
        }
    }
    calculate_pending();
    TRACE << "toggling" << labor_id << "for dwarf:" << dwarf_id;
}

void DwarfModel::set_group_by(int group_by) {
    LOGD << "group_by now set to" << group_by;
    m_group_by = static_cast<GROUP_BY>(group_by);
    if (m_df)
        build_rows();    

}

void DwarfModel::calculate_pending() {
    int changes = 0;
    foreach(Dwarf *d, m_dwarves) {
        changes += d->pending_changes();
    }
    emit new_pending_changes(changes);
}

void DwarfModel::clear_pending() {
    foreach(Dwarf *d, m_dwarves) {
        if (d->pending_changes()) {
            d->clear_pending();
        }
    }
    //reset();
    emit new_pending_changes(0);
    emit need_redraw();
}

void DwarfModel::commit_pending() {
    foreach(Dwarf *d, m_dwarves) {
        if (d->pending_changes()) {
            d->commit_pending();
        }
    }
    load_dwarves();

    emit new_pending_changes(0);
    emit need_redraw();
}

QVector<Dwarf*> DwarfModel::get_dirty_dwarves() {
    QVector<Dwarf*> dwarves;
    foreach(Dwarf *d, m_dwarves) {
        if (d->pending_changes())
            dwarves.append(d);
    }    
    return dwarves;    
}

QModelIndex DwarfModel::findOne(const QVariant &needle, int role, int column, const QModelIndex &start_index) {
    QModelIndex ret_val;
    if (data(start_index, role) == needle) {
        ret_val = start_index;
        return ret_val;
    }
    for (int i = 0; i < rowCount(start_index); ++i) {
        ret_val = findOne(needle, role, column, index(i, column, start_index));
        if (ret_val.isValid())
            return ret_val;
    }
    return ret_val;
}

QList<QPersistentModelIndex> DwarfModel::findAll(const QVariant &needle, int role, int column, QModelIndex start_index) {
    QList<QPersistentModelIndex> ret_val;
    if (data(start_index, role) == needle)
        ret_val.append(QPersistentModelIndex(start_index));
    for (int i = 0; i < rowCount(start_index); ++i) {
        if (column == -1) {
            for (int j = 0; j < columnCount(start_index); ++j) {
                ret_val.append(findAll(needle, role, -1, index(i, j, start_index)));
            }
        } else {
            ret_val.append(findAll(needle, role, column, index(i, column, start_index)));
        }
    }
    return ret_val;
}

bool DwarfModel::compare_turn_count(const Dwarf *a, const Dwarf *b) {
    return a->turn_count() > b->turn_count();
}

void DwarfModel::dwarf_group_toggled(const QString &group_name) {
    QModelIndex agg_cell = findOne(group_name, DR_GROUP_NAME);
    if (agg_cell.isValid()) {
        QModelIndex left = agg_cell;
        QModelIndex right = index(agg_cell.row(), columnCount(agg_cell.parent()) -1, agg_cell.parent());
        emit dataChanged(left, right);
    }
    foreach (Dwarf *d, m_grouped_dwarves[group_name]) {
        foreach(QModelIndex idx, findAll(d->id(), DR_ID, 0, agg_cell)) {
            QModelIndex left = idx;
            QModelIndex right = index(idx.row(), columnCount(idx.parent()) - 1, idx.parent());
            emit dataChanged(left, right);
        }
    }
}

void DwarfModel::dwarf_set_toggled(Dwarf *d) {
    // just update all cells we can find with this dwarf's id
    QList<QPersistentModelIndex> cells = findAll(d->id(), DR_ID, 0);
    foreach(QPersistentModelIndex idx, cells) {
        QModelIndex left = idx;
        QModelIndex right = index(idx.row(), columnCount(idx.parent()) - 1, idx.parent());
        emit dataChanged(left, right);
    }
    if (cells.size()) {
        QPersistentModelIndex first_idx = cells.at(0);
        QModelIndex left = first_idx.parent();
        QModelIndex right = index(left.row(), columnCount(left) - 1, left.parent());
        emit dataChanged(left, right);
    }
}

QBrush DwarfModel::build_gradient_brush(QColor base_col, int alpha_start, int alpha_finish, QPoint start, QPoint end){
    QLinearGradient grad(start.x(),start.y(),end.x(),end.y());
    grad.setCoordinateMode(QGradient::ObjectBoundingMode);
    base_col.setAlpha(alpha_start);
    grad.setColorAt(0,base_col);
    base_col.setAlpha(alpha_finish);
    grad.setColorAt(1,base_col);
    return QBrush(grad);
}
