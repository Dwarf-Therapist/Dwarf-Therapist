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
#include "dfinstance.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "skill.h"
#include "squad.h"
#include "truncatingfilelogger.h"
#include "dwarftherapist.h"
#include "defaultfonts.h"

#include "columntypes.h"
#include "gridview.h"
#include "viewcolumnset.h"
#include "viewcolumn.h"
#include "laborcolumn.h"
#include "spacercolumn.h"
#include "races.h"
#include "fortressentity.h"
#include "gamedatareader.h"
#include "dwarfjob.h"
#include "unithealth.h"
#include "customprofession.h"
#include "defines.h"

#include <QTime>

DwarfModel::DwarfModel(QObject *parent)
    : QStandardItemModel(parent)
    , m_df(0)
    , m_group_by(GB_NOTHING)
    , m_selected_col(-1)
    , m_gridview(0x0)
    , m_total_row_count(0)
    , m_clearing_data(false)
{
    connect(DT, SIGNAL(settings_changed()), this, SLOT(read_settings()));
    read_settings();
}

DwarfModel::~DwarfModel() {
    clear_all(false);
}

void DwarfModel::clear_all(bool clr_pend) {
    m_clearing_data = true;
    if(clr_pend)
        clear_pending();

    qDeleteAll(m_dwarves);
    m_dwarves.clear();
    m_grouped_dwarves.clear();

    if(m_gridview){
        foreach(ViewColumnSet *set, m_gridview->sets()) {
            foreach(ViewColumn *col, set->columns()) {
                col->clear_cells();
            }
        }
    }

    m_total_row_count = 0;
    clear();

    m_clearing_data = false;
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
    clear_all(false);

    m_df->attach();
    foreach(Dwarf *d, m_df->load_dwarves()) {
        m_dwarves[d->id()] = d;
    }
    m_df->detach();

    emit units_refreshed();
}

QList<Squad*> DwarfModel::active_squads(){
    return m_df->squads();
}
Squad* DwarfModel::get_squad(int id){
    return m_df->get_squad(id);
}

void DwarfModel::update_header_info(int id, COLUMN_TYPE type){
    int index = 0;
    foreach(ViewColumnSet *set, m_gridview->sets()) {
        foreach(ViewColumn *col, set->columns()) {
            index ++;
            if(col->type() == type){
                if(col->type()==CT_LABOR){
                    LaborColumn *l = static_cast<LaborColumn*>(col);
                    if(l->labor_id()==id){
                        l->update_count(); //tell this column to update it's count
                        QStandardItem* header = this->horizontalHeaderItem(index);
                        header->setData(col->bg_color(), Qt::BackgroundColorRole);
                        header->setData(set->name(), Qt::UserRole);
                        if(m_show_labor_counts){
                            header->setText(QString("%1 %2")
                                            .arg(l->count(),2,10,QChar('0'))
                                            .arg(col->title()).trimmed());
                        }
                        header->setToolTip(build_col_tooltip(col));
                        return;
                    }
                }
            }
        }
    }
}

void DwarfModel::draw_headers(){
    int start_col = 1;
    QStandardItem *name_col = new QStandardItem();
    name_col->setToolTip(tr("Right click to sort."));
    setHorizontalHeaderItem(0, name_col);
    emit clear_spacers();

    QString max_title = "";
    foreach(ViewColumnSet *set, m_gridview->sets()) {
        /*QStandardItem *set_header = new QStandardItem(set->name());
        set_header->setData(set->bg_color(), Qt::BackgroundColorRole);
        set_header->setData(true);
        setHorizontalHeaderItem(start_col++, set_header);
        emit set_index_as_spacer(start_col - 1);
        emit preferred_header_size(start_col - 1, width);*/
        foreach(ViewColumn *col, set->columns()) {
            QString h_name = col->title();
            if(col->type()==CT_LABOR){
                if(m_show_labor_counts)
                    h_name = QString("%1 %2")
                            .arg(col->count(),2,10,QChar('0'))
                            .arg(col->title()).trimmed();
            }

            if(h_name.length() > max_title.length())
                max_title = h_name;

            QStandardItem *header = new QStandardItem(h_name);
            header->setToolTip(build_col_tooltip(col));
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
                emit preferred_header_size(start_col - 1, m_cell_width);
            }
        }
    }
    emit preferred_header_height(max_title);
}

QString DwarfModel::build_col_tooltip(ViewColumn *col){
    QStringList tooltip;

    QString title = col->title();
    //the weapon column has a list of weapons as the title, but we'll include that after the generic title below
    if(col->type() == CT_WEAPON)
        title = tr("Weapon");

    tooltip.append(tr("<center><h4 style=\"margin:0;\">%1</h4></center>").arg(title));

    //add any additional column information here
    if(col->type()==CT_LABOR){
        LaborColumn *l = static_cast<LaborColumn*>(col);
        l->update_count(); //tell this column to update it's count
        tooltip.append(tr("%1 have this labor enabled.</br>").arg(QString::number(col->count())));
    }else if(col->type() == CT_WEAPON){
        tooltip.append(col->title());
    }

    if(col->get_sortable_types().count() > 0)
        tooltip.append(tr("Right click to change sort method."));

    return tooltip.join("<br/>");
}

void DwarfModel::build_rows() {
    m_grouped_dwarves.clear();

    foreach(ViewColumnSet *set, m_gridview->sets()) {
        foreach(ViewColumn *col, set->columns()) {
            col->clear_cells();
        }
    }
    clear();
    m_total_row_count = 0;

    draw_headers();

    if(m_dwarves.count() <= 0)
        return;

    // populate dwarf maps
    bool only_animals = m_gridview->show_animals();
    int n_adults=0;
    int n_children=0;
    int n_babies=0;
    QString race_name = "";

    GameDataReader *gdr = GameDataReader::ptr();

    if(only_animals)
        race_name = tr("Animals");
    else if(m_df){
        Race* r = m_df->get_race(m_df->dwarf_race_id());
        if(r)
            race_name = r->plural_name();
    }

    foreach(Dwarf *d, m_dwarves) {
        //shared groupings for both animals and the fortress race
        if((only_animals && d->is_animal()) || (!d->is_animal() && !only_animals)){
            if(m_group_by == GB_NOTHING){
                m_grouped_dwarves[0].append(d);
            }else if(m_group_by == GB_PROFESSION){
                m_grouped_dwarves[d->profession()].append(d);
            }else if(m_group_by == GB_SEX){
                m_grouped_dwarves[d->get_gender_orient_desc()].append(d);
            }else if(m_group_by == GB_MIGRATION_WAVE){
                m_grouped_dwarves[d->get_migration_desc()].append(d);
            }else if(m_group_by == GB_AGE){
                if(d->is_baby()){
                    m_grouped_dwarves[d->profession()].append(d);
                }else{
                    int age = d->get_age();
                    if(age < 10){
                        m_grouped_dwarves[d->get_age_formatted()].append(d);
                    }else{
                        int base = (age / 10) * 10;
                        QString age_range = tr("%1 - %2 Years").arg(base)
                                .arg(base + 9);
                        m_grouped_dwarves[age_range].append(d);
                    }
                }
            }else if(m_group_by == GB_CASTE){
                m_grouped_dwarves[d->caste_name(true)].append(d);
            }else if(m_group_by == GB_CASTE_TAG){
                //strip off the underscores, male and female parts of the tag to group genders together
                QString tag = d->caste_tag();
                tag.replace("_", " ");
                tag.replace(tr("FEMALE")," ");
                tag.replace(tr("MALE")," ");
                if(tag.trimmed().isEmpty())
                    tag = race_name;
                m_grouped_dwarves[capitalizeEach(tag.toLower())].append(d);
            }else if(m_group_by == GB_RACE){
                m_grouped_dwarves[capitalizeEach(d->race_name(true,true))].append(d);
            }else if(m_group_by == GB_HAS_NICKNAME){
                if (d->nickname().isEmpty()) {
                    m_grouped_dwarves[tr("No Nickname")].append(d);
                } else {
                    m_grouped_dwarves[tr("Has Nickname")].append(d);
                }
            }else if(m_group_by == GB_HEALTH && (m_animal_health || (!m_animal_health && !d->is_animal()))){
                int treatments = d->get_unit_health().get_treatment_summary(false,false).count();
                int statuses = d->get_unit_health().get_status_summary(false,false).count();
                int wounds = d->get_unit_health().get_wound_details().count();

                bool critical_wounds = d->get_unit_health().has_critical_wounds();
                if(critical_wounds){
                    m_grouped_dwarves[tr("Critical Health Issues")].append(d);
                }else if(treatments || statuses || wounds){
                    m_grouped_dwarves[tr("Minor Health Issues")].append(d);
                }else{
                    m_grouped_dwarves[tr("No Health Issues")].append(d);
                }
            }else{
                if(only_animals && d->is_animal())
                    m_grouped_dwarves["N/A"].append(d);
            }

            //update our counts for the display
            if(d->is_child())
                n_children ++;
            else if(d->is_baby())
                n_babies ++;
            else
                n_adults ++;
        }

        //groups specific to the actual race we're playing (ie. dwarfs)
        if(!d->is_animal() && !only_animals)
        {
            //if hiding children/babies, don't group them, otherwise the aggregate row total in the heading will be off
            if(!DT->hide_non_adults() || (!d->is_child() && !d->is_baby())){

                if(m_group_by == GB_LEGENDARY){
                    int legendary_skills = 0;
                    foreach(Skill s, d->get_skills()->values()) {
                        if (s.capped_level() >= 15)
                            legendary_skills++;
                    }
                    if (legendary_skills)
                        m_grouped_dwarves[tr("Legends")].append(d);
                    else
                        m_grouped_dwarves[tr("Losers")].append(d);
                }else if(m_group_by == GB_HAPPINESS){
                    m_grouped_dwarves[d->happiness_name(d->get_happiness())].append(d);
                }else if(m_group_by == GB_GOALS){
                    m_grouped_dwarves[tr("%1 Goals Realized").arg(d->goals_realized())].append(d);
                }else if(m_group_by == GB_SKILL_RUST){
                    m_grouped_dwarves[Skill::get_rust_level_desc(d->rust_level())].append(d);
                }else if(m_group_by == GB_CURRENT_JOB){
                    QString job_desc = GameDataReader::ptr()->get_job(d->current_job_id())->description;
                    //if the job is some kind of reaction that doesn't use a material, use the reaction's name
                    //otherwise, use the base job name, prior to replacing ?? with a material so they're grouped
                    if(d->current_job_id() != 212 && !job_desc.contains("??"))
                        job_desc = d->current_job();
                    m_grouped_dwarves[job_desc.replace(" ??","")].append(d);
                }else if(m_group_by == GB_MILITARY_STATUS){
                    if (d->is_baby() || d->is_child()) {
                        m_grouped_dwarves[tr("Juveniles")].append(d);
                    } else if (d->active_military() && !d->can_set_labors()) { //master level military elites
                        m_grouped_dwarves[tr("Champions")].append(d);
                    } else if (!d->noble_position().isEmpty()) {
                        m_grouped_dwarves[tr("Nobles")].append(d);
                    } else if (d->active_military()) {
                        m_grouped_dwarves[tr("Military (On Duty)")].append(d);
                    } else if (d->squad_id() > -1){
                        m_grouped_dwarves[tr("Military (Off Duty)")].append(d);
                    }else {
                        m_grouped_dwarves[tr("Can Activate")].append(d);
                    }
                }else if(m_group_by == GB_HIGHEST_MOODABLE){
                    Skill highest = d->highest_moodable();
                    if(highest.capped_level() != -1 && !d->had_mood()){
                        m_grouped_dwarves[highest.name()].append(d);
                    }else{
                        if(d->had_mood())
                            m_grouped_dwarves["~Had Mood~"].append(d);
                        else
                            m_grouped_dwarves["~Craft (Bone/Stone/Wood)~"].append(d);
                    }
                }else if(m_group_by == GB_HIGHEST_SKILL){
                    Skill highest = d->highest_skill();
                    QString level = gdr->get_skill_level_name(highest.capped_level());
                    m_grouped_dwarves[level].append(d);
                }else if(m_group_by == GB_TOTAL_SKILL_LEVELS){
                    m_grouped_dwarves[tr("Levels: %1").arg(d->total_skill_levels())]
                            .append(d);
                }else if(m_group_by == GB_ASSIGNED_LABORS || m_group_by == GB_ASSIGNED_SKILLED_LABORS){
                    bool include_hauling = (m_group_by == GB_ASSIGNED_LABORS);
                    m_grouped_dwarves[tr("%1 Assigned Labors")
                            .arg(d->total_assigned_labors(include_hauling))].append(d);
                }else if(m_group_by == GB_SQUAD){
                    if(d->squad_name().isEmpty()) {
                        m_grouped_dwarves[tr("No Squad")].append(d);
                    } else {
                        m_grouped_dwarves[d->squad_name()].append(d);
                    }
                }
            }
        }
    }

    foreach(QString key, m_grouped_dwarves.uniqueKeys()) {
        build_row(key);
    }

    emit new_creatures_count(n_adults,n_children,n_babies,race_name);
}

void DwarfModel::build_row(const QString &key) {
    QIcon icn_gender;
    QStandardItem *agg_first_col = 0;
    QList<QStandardItem*> agg_items;
    if(!m_grouped_dwarves.contains(key)){
        LOGE << "Group by failed because key " << key << " wasn't found.";
        return;
    }
    if(m_grouped_dwarves.value(key).count() <= 0){
        LOGE << "Group by failed because there are no values for key " << key;
        return;
    }
    Dwarf *first_dwarf = m_grouped_dwarves.value(key).at(0);
    if (!first_dwarf) {
        LOGE << "'Group by'' set for" << key << "has a bad ref for its first " << "dwarf";
        return;
    }

    if (m_group_by != GB_NOTHING) {
        // we need a root element to hold group members...
        QString title = QString("%1 (%2)").arg(key).arg(m_grouped_dwarves.value(key).size());
        agg_first_col = new QStandardItem(title);
        //bold aggregate titles
        agg_first_col->setData(get_font(true), Qt::FontRole);
        //        agg_first_col->setData(build_gradient_brush(QColor(Qt::gray),125,0,QPoint(0,0),QPoint(1,0)),Qt::BackgroundRole);
        agg_first_col->setData(true, DR_IS_AGGREGATE);
        agg_first_col->setData(key, DR_GROUP_NAME);
        agg_first_col->setData(0, DR_RATING);
        //root->setData(title, DR_SORT_VALUE);
        // for integer based values we want to make sure they sort by the int
        // values instead of the string values
        if (m_group_by == GB_MIGRATION_WAVE) {
            agg_first_col->setData(first_dwarf->migration_wave(), DR_SORT_VALUE);
        } else if (m_group_by == GB_HIGHEST_SKILL) {
            agg_first_col->setData(first_dwarf->highest_skill().actual_exp(), DR_SORT_VALUE);
        } else if (m_group_by == GB_HIGHEST_MOODABLE) {
            //make sure to show the had mood and generic craft moods at the bottom of the list
            if(first_dwarf->had_mood() || first_dwarf->highest_moodable().capped_level() < 0)
                agg_first_col->setData(QChar(128), DR_SORT_VALUE);
            else
                agg_first_col->setData(first_dwarf->highest_moodable().name(), DR_SORT_VALUE);
        } else if (m_group_by == GB_TOTAL_SKILL_LEVELS) {
            agg_first_col->setData(first_dwarf->total_skill_levels(), DR_SORT_VALUE);
        } else if (m_group_by == GB_GOALS) {
            agg_first_col->setData(first_dwarf->goals_realized(), DR_SORT_VALUE);
        } else if (m_group_by == GB_SKILL_RUST) {
            agg_first_col->setData(first_dwarf->rust_level(), DR_SORT_VALUE);
        } else if (m_group_by == GB_HAPPINESS) {
            agg_first_col->setData(first_dwarf->get_happiness(), DR_SORT_VALUE);
        } else if (m_group_by == GB_ASSIGNED_LABORS || m_group_by == GB_ASSIGNED_SKILLED_LABORS) {
            bool include_hauling = (m_group_by == GB_ASSIGNED_LABORS);
            agg_first_col->setData(first_dwarf->total_assigned_labors(include_hauling), DR_SORT_VALUE);
        } else if (m_group_by == GB_PROFESSION) {
            agg_first_col->setData(first_dwarf->profession(), DR_SORT_VALUE);
        } else if (m_group_by == GB_RACE){
            agg_first_col->setData(first_dwarf->race_name(true,true), DR_SORT_VALUE);
        } else if (m_group_by == GB_CASTE) {
            agg_first_col->setData(first_dwarf->caste_name(true), DR_SORT_VALUE);
        } else if (m_group_by == GB_CASTE_TAG){
            agg_first_col->setData(first_dwarf->caste_tag(), DR_SORT_VALUE);
        } else if (m_group_by == GB_AGE){
            agg_first_col->setData(first_dwarf->get_age(), DR_SORT_VALUE);
        } else if (m_group_by == GB_SEX){
            agg_first_col->setData(first_dwarf->get_gender_orient_desc(), DR_SORT_VALUE);
        } else if (m_group_by == GB_SQUAD){
            int squad_id = first_dwarf->squad_id();
            if(squad_id != -1){
                Squad *s = m_df->get_squad(first_dwarf->squad_id());
                if(s){
                    int squad_count = s->assigned_count();
                    title = QString("%1 (%2)").arg(key).arg(squad_count);
                    agg_first_col->setText(title);
                    if(squad_count != m_grouped_dwarves.value(key).size()){
                        agg_first_col->setToolTip(tr("The count may be different as Dwarf Fortress keeps missing, dead dwarves in squads until they're found."));
                        agg_first_col->setIcon(QIcon(":img/exclamation-red-frame.png"));
                    }
                    agg_first_col->setData(squad_id, DR_SORT_VALUE);
                    agg_first_col->setData(squad_id,DR_ID);
                    agg_first_col->setData(key,DR_GROUP_NAME);
                }
            }else{
                //put non squads at the bottom of the groups when grouping by squad
                agg_first_col->setData(QChar(128), DR_SORT_VALUE);
            }
        } else if (m_group_by == GB_CURRENT_JOB){
            //put idle, on break and soldiers at the top/bottom
            if(first_dwarf->current_job_id() < 0)
                agg_first_col->setData(QString::number(first_dwarf->current_job_id()), DR_SORT_VALUE);
        }
        agg_first_col->setData(agg_first_col->data(DR_SORT_VALUE),DR_GLOBAL);
        agg_items << agg_first_col;
    }

    if (agg_first_col) { // we have a parent, so we should draw an aggregate row
        m_total_row_count += 1;
        foreach(ViewColumnSet *set, m_gridview->sets()) {
            foreach(ViewColumn *col, set->columns()) {
                QStandardItem *item = col->build_aggregate(key, m_grouped_dwarves[key]);
                agg_items << item;
            }
        }
    }

    foreach(Dwarf *d, m_grouped_dwarves.value(key)) {
        QStandardItem *i_name = new QStandardItem(d->nice_name());
        bool name_italic = false;

        if(m_decorate_nobles){
            if((m_group_by==GB_SQUAD && (m_df->get_squad(d->squad_id()) && d->squad_position()==0)) || d->noble_position() != ""){
                i_name->setText(QString("%1 %2 %1").arg(m_symbol).arg(i_name->text()));
                name_italic = true;
            }
        }

        //background gradients for nobles
        if(d && m_highlight_nobles){
            if(d->noble_position() != ""){
                QColor col = m_df->fortress()->get_noble_color(d->historical_id());
                i_name->setData(build_gradient_brush(col,col.alpha(),0,QPoint(0,0),QPoint(1,0)),Qt::BackgroundRole);
                i_name->setData(complement(col,0.25),Qt::ForegroundRole);
            }
        }

        //set cursed colors
        if(d && m_highlight_cursed){
            switch(d->get_curse_type()){
            case eCurse::VAMPIRE:
            case eCurse::WEREBEAST:
            {
                name_italic = true;
                i_name->setData(m_cursed_bg,Qt::BackgroundRole);
                i_name->setData(complement(m_curse_col,0.25),Qt::ForegroundRole);
            }
                break;
            case eCurse::OTHER:
            {
                name_italic = true;
                i_name->setData(m_cursed_bg_light,Qt::BackgroundRole);
            }
                break;
            default:
                break;
            }
        }

        i_name->setData(get_font(d->active_military(),name_italic),Qt::FontRole);
        if(m_show_tooltips){
            i_name->setToolTip(d->tooltip_text());
        }else{
            i_name->setData(d->tooltip_text(),DwarfModel::DR_TOOLTIP);
        }

        i_name->setStatusTip(d->nice_name());
        i_name->setData(false, DR_IS_AGGREGATE);
        i_name->setData(0, DR_RATING);
        i_name->setData(d->id(), DR_ID);

        //set the roles for the special right click sorting
        i_name->setData(d->get_age(), DR_AGE);
        i_name->setData(d->body_size(), DR_SIZE);
        i_name->setData(d->nice_name(), DR_NAME);

        i_name->setData(d->get_global_sort_key(m_group_by), DR_GLOBAL);

        //set the sorting within groups when grouping
        QVariant sort_val;
        switch(m_group_by) {
        case GB_PROFESSION:
            sort_val = d->raw_profession();
            break;
        case GB_HAPPINESS:
            sort_val = d->get_raw_happiness();
            break;
        case GB_SQUAD:
        {
            sort_val = d->squad_position();
            if(sort_val.toInt() < 0)
                sort_val = d->nice_name();
        }
            break;
        case GB_AGE:
            sort_val = d->get_age();
            break;
        case GB_NOTHING:
        default:
            sort_val = d->nice_name();
            break;
        }
        i_name->setData(sort_val, DR_SORT_VALUE);
//        i_name->setData(sort_val, DR_GLOBAL);

        //set gender icons
        if(m_show_gender){
            icn_gender.addFile(d->gender_icon_path());
            i_name->setIcon(icn_gender);
        }

        QList<QStandardItem*> items;
        items << i_name;
        int col_idx = 1;
        foreach(ViewColumnSet *set, m_gridview->sets()) {
            foreach(ViewColumn *col, set->columns()) {
                QStandardItem *item = col->build_cell(d);
                if(col_idx == GLOBAL_SORT_COL_IDX)//update the hidden global sort column's related cell global sort value
                    item->setData(d->get_global_sort_key(m_group_by),DwarfModel::DR_GLOBAL);
                items << item;
            }
        }

        if (agg_first_col) {
            agg_first_col->appendRow(items);
        } else {
            appendRow(items);
        }
        d->m_name_idx = indexFromItem(i_name);
        m_total_row_count += 1;
    }
    if (agg_first_col) {
        appendRow(agg_items);
    }
}
void DwarfModel::set_global_group_sort_info(int role, Qt::SortOrder order){
    m_global_group_sort_info.insert(m_group_by,qMakePair(role,order));
}
void DwarfModel::set_global_sort_col(QString grid_view_name, int col_idx){
    if(col_idx < 0)
        m_global_sort_info.remove(m_group_by);
    else
        m_global_sort_info.insert(m_group_by, qMakePair(grid_view_name,col_idx));
}
void DwarfModel::update_global_sort_col(int group_id){
    if(!m_global_sort_info.keys().contains(group_id))
        m_global_sort_info.insert(group_id,qMakePair(this->m_gridview->name(),0));
}

void DwarfModel::cell_activated(const QModelIndex &idx) {
    QStandardItem *item = itemFromIndex(idx);
    if(!item)
        return;
    bool is_aggregate = item->data(DR_IS_AGGREGATE).toBool();

    int dwarf_id = 0;
    if(!is_aggregate && item->data(DR_ID).canConvert<int>()){
        dwarf_id = item->data(DR_ID).toInt();
        if (!dwarf_id) {
            LOGW << "double clicked what should have been a dwarf name, but the ID wasn't set!";
            return;
        }
    }

    if (idx.column() == 0) {
        if (is_aggregate)
            return; // no double clicking aggregate names

        Dwarf *d = get_dwarf_by_id(dwarf_id);
        d->show_details();
        return;
    }

    COLUMN_TYPE type = static_cast<COLUMN_TYPE>(idx.data(DwarfModel::DR_COL_TYPE).toInt());
    if (type != CT_LABOR && type != CT_FLAGS && type != CT_ROLE && type != CT_SUPER_LABOR && type != CT_CUSTOM_PROFESSION)
        return;

    Q_ASSERT(item);

    int labor_id = item->data(DR_LABOR_ID).toInt();
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
    } else {
        if (type == CT_LABOR)
            m_dwarves[dwarf_id]->toggle_labor(labor_id);
        else if (type == CT_FLAGS)
            m_dwarves[dwarf_id]->toggle_flag_bit(labor_id);
        else if (type == CT_ROLE || type == CT_SUPER_LABOR || type == CT_CUSTOM_PROFESSION){
            Dwarf *d  = m_dwarves[dwarf_id];
            bool cp_applied = false;
            if(type == CT_CUSTOM_PROFESSION){
                QString new_prof_name = idx.data(DwarfModel::DR_CUSTOM_PROF).toString();
                if(!new_prof_name.isEmpty()){
                    CustomProfession *cp = DT->get_custom_profession(new_prof_name);
                    if(cp){
                        if(d->custom_profession_name() == cp->get_name()){
                            d->reset_custom_profession(true);
                        }else{
                            d->apply_custom_profession(cp);
                        }
                        cp_applied = true;
                    }
                }
            }
            if(!cp_applied){
                QVariantList labors = item->data(DwarfModel::DR_LABORS).toList();
                int limit = ceil((double)labors.count() / 2.0f);
                int enabled = 0;
                bool enabling = true;
                foreach(QVariant id, labors){
                    if(d->labor_enabled(id.toInt()))
                        enabled++;
                }
                if((enabled < limit && enabled > 0) || enabled == labors.count())
                    enabling = false;
                foreach(QVariant id, labors){
                    d->set_labor(id.toInt(),enabling,false);
                }
            }
        }

        QModelIndex left = index(idx.parent().row(), 0, idx.parent().parent());
        QModelIndex right = index(idx.parent().row(), columnCount(idx.parent()) - 1, idx.parent().parent());
        emit dataChanged(left, right); // update the agg row

        left = index(idx.row(), 0, idx.parent());
        right = index(idx.row(), columnCount(idx.parent()) - 1, idx.parent());
        emit dataChanged(left, right); // update the dwarf row
    }
}

void DwarfModel::set_group_by(int group_by) {
    LOGI << "group_by now set to" << group_by << " for view " << current_grid_view()->name();
    m_group_by = static_cast<GROUP_BY>(group_by);
    if(m_df){
        QTime t;
        t.start();
        build_rows();
        LOGI << "loaded rows for" << m_gridview->name() << t.elapsed() << "ms";
    }
}

void DwarfModel::calculate_pending() {
    int changes = 0;
    foreach(Dwarf *d, m_dwarves) {
        changes += d->pending_changes();
    }
    foreach(Squad *s, m_df->squads()){
        changes += s->pending_changes();
    }

    emit new_pending_changes(changes);
}

void DwarfModel::clear_pending() {
    //clear squads before we refresh dwarf data
    foreach(Squad *s, m_df->squads()){
        if(s->pending_changes())
            s->clear_pending();
    }

    foreach(Dwarf *d, m_dwarves) {
        if (d->pending_changes()) {
            d->clear_pending();
        }
    }
    //reset();
    DT->emit_labor_counts_updated();
    emit new_pending_changes(0);
    emit need_redraw();
}

void DwarfModel::commit_pending() {
    //before committing, commit squad name changes, and then refresh our squads
    m_df->fortress()->refresh_squads(); //update our fortress squad list first
    foreach(Squad *s, m_df->squads()){
        s->commit_pending();
    }
    m_df->load_squads(true);

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

void DwarfModel::dwarf_group_toggled(const QString &group_name) {
    QModelIndex agg_cell = findOne(group_name, DR_GROUP_NAME);
    QModelIndex left;
    QModelIndex right;
    if (agg_cell.isValid()) {
        left = agg_cell;
        right = index(agg_cell.row(), columnCount(agg_cell.parent()) -1, agg_cell.parent());
        emit dataChanged(left, right);
    }
    foreach (Dwarf *d, m_grouped_dwarves[group_name]) {
        foreach(QModelIndex idx, findAll(d->id(), DR_ID, 0, agg_cell)) {
            left = idx;
            right = index(idx.row(), columnCount(idx.parent()) - 1, idx.parent());
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

void DwarfModel::read_settings(){
    QSettings *s = DT->user_settings();

    //font
    m_font = s->value("options/grid/font", QFont(DefaultFonts::getRowFontName(), DefaultFonts::getRowFontSize())).value<QFont>();

    //noble symbol
    QFontMetrics fm(m_font);
    m_symbol = QChar(0x263C); //masterwork symbol in df
    if(!fm.inFont(m_symbol)){
        m_symbol = QChar(0x2261); //3 horizontal lines
        if(!fm.inFont(m_symbol))
            m_symbol = QChar(0x002A); //asterisk
    }

    m_animal_health = DT->user_settings()->value("options/animal_health",false).toBool();
    m_show_gender = s->value("options/grid/show_gender_icons",true).toBool();
    m_decorate_nobles = s->value("options/grid/decorate_noble_names",false).toBool();
    m_highlight_nobles = s->value("options/highlight_nobles",true).toBool();
    m_highlight_cursed = s->value("options/highlight_cursed",false).toBool();
    m_curse_col = s->value("options/colors/cursed", FortressEntity::get_default_color(FortressEntity::CURSED)).value<QColor>();
    m_cursed_bg = build_gradient_brush(m_curse_col,m_curse_col.alpha(),0,QPoint(0,0),QPoint(1,0));
    m_cursed_bg_light = build_gradient_brush(m_curse_col, 50,0,QPoint(0,0),QPoint(1,0)); //keep a weakly highlighted version
    m_show_labor_counts = s->value("options/grid/show_labor_counts",false).toBool();
    m_show_tooltips = s->value("options/grid/show_tooltips",true).toBool();

    m_cell_width = s->value("options/grid/cell_size", DEFAULT_CELL_SIZE).toInt();
    m_cell_padding = s->value("options/grid/cell_padding", 0).toInt();
    m_cell_width += (m_cell_padding*2)+2;
}

QFont DwarfModel::get_font(bool bold, bool italic, bool underline){
    QFont ret = m_font;
    if(bold)
        ret.setBold(true);
    if(italic)
        ret.setItalic(true);
    if(underline)
        ret.setUnderline(true);
    return ret;
}
