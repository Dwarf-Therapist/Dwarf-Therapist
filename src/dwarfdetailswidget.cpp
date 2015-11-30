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

#include "dwarfdetailswidget.h"
#include "ui_dwarfdetailswidget.h"
#include "dwarftherapist.h"
#include "gamedatareader.h"
#include "dwarf.h"
#include "trait.h"
#include "utils.h"
#include "dfinstance.h"
#include "fortressentity.h"
#include "skill.h"
#include "attribute.h"
#include "sortabletableitems.h"
#include "unithealth.h"
#include "healthinfo.h"
#include "belief.h"
#include <QProgressBar>

#if QT_VERSION < 0x050000
# define setSectionResizeMode setResizeMode
#endif

QColor DwarfDetailsWidget::color_low = QColor(168, 10, 44, 255);
QColor DwarfDetailsWidget::color_high = QColor(0, 60, 128, 255);
QColor DwarfDetailsWidget::color_mood_had = QColor(125, 125, 125, 255);
QColor DwarfDetailsWidget::color_mood_high = QColor(60, 148, 19, 255);

DwarfDetailsWidget::DwarfDetailsWidget(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , ui(new Ui::DwarfDetailsWidget)
    , m_current_id(-1)
{
    ui->setupUi(this);

    //ui->splitter->setOpaqueResize(true);
    //ui->splitter->setObjectName("details_splitter"); //important!! this name is used to find the splitter and save it's state!!

    int default_size = 60;

    ui->tw_skills->setColumnCount(4);
    ui->tw_skills->setEditTriggers(QTableWidget::NoEditTriggers);
    ui->tw_skills->setGridStyle(Qt::NoPen);
    ui->tw_skills->setAlternatingRowColors(true);
    ui->tw_skills->setHorizontalHeaderLabels(QStringList() << "Skill" << "Level" << "Bonus" << "Progress");
    ui->tw_skills->verticalHeader()->hide();
    ui->tw_skills->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->tw_skills->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    ui->tw_skills->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    ui->tw_skills->horizontalHeader()->setStretchLastSection(true);
    ui->tw_skills->horizontalHeader()->resizeSection(0,100);
    ui->tw_skills->horizontalHeader()->resizeSection(1,default_size);
    ui->tw_skills->horizontalHeader()->resizeSection(2,default_size);

    ui->tw_attributes->setColumnCount(4);
    ui->tw_attributes->setEditTriggers(QTableWidget::NoEditTriggers);
    ui->tw_attributes->setWordWrap(true);
    ui->tw_attributes->setShowGrid(false);
    ui->tw_attributes->setGridStyle(Qt::NoPen);
    ui->tw_attributes->setAlternatingRowColors(true);
    ui->tw_attributes->setHorizontalHeaderLabels(QStringList() << "Attribute" << "Value" << "Max" << "Message");
    ui->tw_attributes->verticalHeader()->hide();
    ui->tw_attributes->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->tw_attributes->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    ui->tw_attributes->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    ui->tw_attributes->horizontalHeader()->setStretchLastSection(true);
    ui->tw_attributes->horizontalHeader()->resizeSection(0,100);
    ui->tw_attributes->horizontalHeader()->resizeSection(1,default_size);
    ui->tw_attributes->horizontalHeader()->resizeSection(2,default_size);

    ui->tw_traits->setColumnCount(3);
    ui->tw_traits->setEditTriggers(QTableWidget::NoEditTriggers);
    ui->tw_traits->setWordWrap(true);
    ui->tw_traits->setShowGrid(false);
    ui->tw_traits->setGridStyle(Qt::NoPen);
    ui->tw_traits->setAlternatingRowColors(true);
    ui->tw_traits->setHorizontalHeaderLabels(QStringList() << "Item" << "Value" << "Message");
    ui->tw_traits->verticalHeader()->hide();
    ui->tw_traits->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->tw_traits->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
//    ui->tw_traits->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    ui->tw_traits->horizontalHeader()->setStretchLastSection(true);
    ui->tw_traits->horizontalHeader()->resizeSection(0,100);
    ui->tw_traits->horizontalHeader()->resizeSection(1,default_size);
//    ui->tw_traits->horizontalHeader()->resizeSection(2,default_size);

    ui->tw_roles->setColumnCount(2);
    ui->tw_roles->setEditTriggers(QTableWidget::NoEditTriggers);
    ui->tw_roles->setWordWrap(true);
    ui->tw_roles->setShowGrid(false);
    ui->tw_roles->setGridStyle(Qt::NoPen);
    ui->tw_roles->setAlternatingRowColors(true);
    ui->tw_roles->setHorizontalHeaderLabels(QStringList() << "Role" << "Rating");
    ui->tw_roles->verticalHeader()->hide();
    ui->tw_roles->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->tw_roles->horizontalHeader()->setStretchLastSection(true);
    ui->tw_roles->horizontalHeader()->resizeSection(0,100);

    ui->tw_prefs->setColumnCount(2);
    ui->tw_prefs->setEditTriggers(QTableWidget::NoEditTriggers);
    ui->tw_prefs->setWordWrap(true);
    ui->tw_prefs->setShowGrid(false);
    ui->tw_prefs->setGridStyle(Qt::NoPen);
    ui->tw_prefs->setAlternatingRowColors(true);
    ui->tw_prefs->setHorizontalHeaderLabels(QStringList() << "Type" << "Preferences");
    ui->tw_prefs->verticalHeader()->hide();
    ui->tw_prefs->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->tw_prefs->horizontalHeader()->setStretchLastSection(true);
    ui->tw_prefs->horizontalHeader()->resizeSection(0,100);


    ui->tw_health->setColumnCount(2);

    //splitter
    m_ui_state = DT->user_settings()->value("gui_options/unit_detail_state").toByteArray();
    //ui->splitter->restoreState(m_ui_state);

    //skill sorts
    m_sorting << qMakePair(1,Qt::DescendingOrder);
    ui->tw_skills->sortItems(1, Qt::DescendingOrder);
    //attribute sorts
    m_sorting << qMakePair(0,Qt::AscendingOrder);
    ui->tw_attributes->sortItems(0, Qt::AscendingOrder);
    //trait sorts
    m_sorting << qMakePair(1,Qt::DescendingOrder);
    ui->tw_traits->sortItems(1, Qt::DescendingOrder);
    //role sorts
    m_sorting << qMakePair(1,Qt::DescendingOrder);
    ui->tw_roles->sortItems(1, Qt::DescendingOrder);
    //pref sorts
    m_sorting << qMakePair(0,Qt::DescendingOrder);
    ui->tw_prefs->sortItems(0, Qt::DescendingOrder);
    //health sorts
    m_sorting << qMakePair(0,Qt::AscendingOrder);
    ui->tw_health->sortItems(0, Qt::AscendingOrder);

    QMainWindow *dock_area = new QMainWindow(this);
    dock_area->setObjectName("unit_details_dock_area");
    dock_area->setWindowFlags(Qt::Widget);
    dock_area->setCentralWidget(0);
    dock_area->setTabPosition(Qt::AllDockWidgetAreas,QTabWidget::North);
    ui->verticalLayout->addWidget(dock_area);

    foreach(QDockWidget *dw, this->findChildren<QDockWidget*>()){
        dock_area->addDockWidget(Qt::TopDockWidgetArea,dw,Qt::Vertical);
    }

    if(m_ui_state.count() > 0){
        dock_area ->restoreState(m_ui_state);
    }else{
        QDockWidget *first_dock = 0;
        foreach(QDockWidget *dw, dock_area->findChildren<QDockWidget*>()){
            if(first_dock == 0){
                first_dock = dw;
            }else{
                dock_area->tabifyDockWidget(first_dock,dw);
            }
        }
        ui->dock_unit_skills->raise();
    }

}

DwarfDetailsWidget::~DwarfDetailsWidget() {
    delete ui;
}

void DwarfDetailsWidget::clear(){
    m_current_id = -1;
    //clear tables
    clear_table(*ui->tw_skills);
    clear_table(*ui->tw_attributes);
    clear_table(*ui->tw_traits);
    clear_table(*ui->tw_roles);
    clear_table(*ui->tw_prefs);
}

void DwarfDetailsWidget::show_dwarf(Dwarf *d) {
    if(d->id() == m_current_id)
        return;

    ui->lbl_gender->setText(QString("<img src='%1'>").arg(d->gender_icon_path()));
    ui->lbl_gender->setToolTip(d->get_gender_orient_desc());

    ui->lbl_dwarf_name->setText(d->nice_name());
    ui->lbl_dwarf_name->setToolTip(tr("Name: %1").arg(ui->lbl_dwarf_name->text()));

    ui->lbl_age->setText(d->get_age_formatted());
    ui->lbl_age->setToolTip(d->get_migration_desc());

    QString trans_name = d->translated_name();
    ui->lbl_translated_name->setText(QString("%1").arg(trans_name.isEmpty() ? "" : "(" + trans_name + ")"));
    if(!trans_name.isEmpty())
        ui->lbl_translated_name->setToolTip(tr("Translated Name: %1").arg(trans_name));

    ui->lbl_profession->setText(QString("%2 %1").arg(embedPixmap(d->profession_icon())).arg(d->profession()));
    ui->lbl_profession->setToolTip(tr("Profession: %1").arg(ui->lbl_profession->text()));

    if(d->noble_position().isEmpty()){
        ui->lbl_noble_position->hide();
        ui->lbl_noble->hide();
    }
    else{
        ui->lbl_noble->setText(tr("<b>Noble Position%1</b>").arg(d->noble_position().contains(",") ? "s" : ""));
        ui->lbl_noble_position->setText(d->noble_position());
        ui->lbl_noble_position->show();
        ui->lbl_noble->show();
    }
    ui->lbl_noble_position->setToolTip(ui->lbl_noble_position->text());

    if(d->artifact_name().isEmpty()){
        ui->lbl_artifact->hide();
    }else{
        ui->lbl_artifact->setText(tr("Creator of '%1'").arg(d->artifact_name()));
        ui->lbl_artifact->show();
    }

    if(d->squad_name().isEmpty()){
        ui->lbl_squad_name->hide();
    }
    else{
        ui->lbl_squad_name->setText(tr("<b>Member of Squad <i>'%1'</i></b>").arg(d->squad_name()));
        ui->lbl_squad_name->show();
    }

    if(!DT->multiple_castes){
        ui->lbl_caste->hide();
    }else{
        ui->lbl_caste->setText(tr("<b>Caste: %1</b>").arg(d->caste_name()));
        ui->lbl_caste->show();
        ui->lbl_caste->setToolTip(d->caste_desc());
    }

    ui->lbl_artifact->setToolTip(ui->lbl_artifact->text());

    ui->lbl_current_job->setText(QString("%1").arg(d->current_job()));
    ui->lbl_current_job->setToolTip(tr("Job ID: %1").arg(QString::number(d->current_job_id())));

    GameDataReader *gdr = GameDataReader::ptr();

    ui->lbl_happiness->setText(QString("%1").arg(d->get_happiness_desc()));
    if(d->in_stressed_mood()){
        ui->lbl_happiness_title->setText(tr("Happiness:%1").arg(embedPixmap(QPixmap(":img/exclamation-red-frame.png"))));
        QString desc = QString("<center><b>%1</b><br/>%2</center>")
                .arg(gdr->get_mood_name(d->current_mood(),true))
                .arg(gdr->get_mood_desc(d->current_mood(),true));
        ui->lbl_happiness_title->setToolTip(desc);
        ui->lbl_happiness->setToolTip(QString("%1<br/>%2").arg(desc).arg(d->get_emotions_desc()));
    }else{
        ui->lbl_happiness_title->setText(tr("Happiness: "));
        ui->lbl_happiness_title->setToolTip("");
        ui->lbl_happiness->setToolTip(d->get_emotions_desc());
    }

    QPalette p;
    QColor color = DT->user_settings()->value(QString("options/colors/happiness/%1").arg(static_cast<int>(d->get_happiness()))).value<QColor>();
    QColor color2 = p.window().color();
    ui->lbl_happiness->setStyleSheet(label_gradient(color,color2));

    if(DT->user_settings()->value("options/highlight_nobles",true).toBool() && d->noble_position() != ""){
        color = DT->get_DFInstance()->fortress()->get_noble_color(d->historical_id());
        ui->lbl_noble_position->setStyleSheet(label_gradient(color,color2));
    }


    //clear tables
    clear_table(*ui->tw_skills);
    clear_table(*ui->tw_attributes);
    clear_table(*ui->tw_traits);
    clear_table(*ui->tw_roles);
    clear_table(*ui->tw_prefs);
    ui->tw_health->clear();

    QFont bold_item_font = ui->tw_skills->font();
    bold_item_font.setBold(true);

    // SKILLS TABLE
    QHash<int,Skill> *skills = d->get_skills();
    ui->tw_skills->setSortingEnabled(false);
    int real_count = 0;
    int raw_bonus_xp = 100;
    int bonus_xp = 0;
    QString tooltip = "";
    bool no_bonuses = true;
    foreach(Skill s, skills->values()){
        if(s.capped_level() > -1)
        {
            real_count = ui->tw_skills->rowCount();
            ui->tw_skills->insertRow(real_count);
            ui->tw_skills->setRowHeight(real_count, 18);

            QTableWidgetItem *item_skill = new QTableWidgetItem(s.name());
            tooltip = tr("<center><h4>%1</h4></center>").arg(s.name());
            if(s.id()==d->highest_moodable().id()){
                if(d->had_mood()){
                    item_skill->setForeground(color_mood_had);
                    item_skill->setFont(bold_item_font);
                    tooltip.append(tr("<p>Has already had a mood!</p>"));
                }
                else{
                    item_skill->setForeground(color_mood_high);
                    item_skill->setFont(bold_item_font);
                    tooltip.append(tr("<p>This is the highest moodable skill.</p>"));
                }
            }
            item_skill->setToolTip(tooltip);

            sortableNumericTableWidgetItem *item_level = new sortableNumericTableWidgetItem();
            item_level->setText(QString::number(d->get_skill_level(s.id())));
            item_level->setData(Qt::UserRole, (float)d->get_skill_level(s.id(),true,true));
            item_level->setTextAlignment(Qt::AlignHCenter);
            if(s.rust_rating() != ""){
                QColor col_rust = s.rust_color();
                col_rust.setAlpha(215);
                item_level->setForeground(col_rust);
                item_level->setFont(bold_item_font);
            }
            item_level->setToolTip(s.to_string(true,true,false));


            raw_bonus_xp = s.skill_rate();
            bonus_xp = raw_bonus_xp - 100;
            if(no_bonuses && bonus_xp != 0)
                no_bonuses = false;
            sortableNumericTableWidgetItem *item_bonus = new sortableNumericTableWidgetItem();
            if(bonus_xp != 0)
                item_bonus->setText(QString::number(bonus_xp,'f',0)+"%");
            else
                item_bonus->setText("");
            item_bonus->setData(Qt::UserRole, bonus_xp);
            item_bonus->setTextAlignment(Qt::AlignHCenter);
            if(bonus_xp != 0)
                item_bonus->setToolTip(tr("Receives %1% <b>%2</b> experience than normal. (RAW: %3%)")
                                       .arg(abs(bonus_xp))
                                       .arg(bonus_xp > 0 ? "more" : "less")
                                       .arg(raw_bonus_xp));
            if(bonus_xp < 0){
                item_bonus->setForeground(color_low);
            }

            QProgressBar *pb = new QProgressBar(ui->tw_skills);
            pb->setRange(s.exp_for_current_level(), s.exp_for_next_level());
            if(s.is_losing_xp())
                pb->setValue(s.exp_for_next_level());
            else
                pb->setValue(s.actual_exp());
            pb->setDisabled(true);// this is to keep them from animating and looking all goofy
            pb->setToolTip(s.exp_summary());

            ui->tw_skills->setItem(real_count, 0, item_skill);
            ui->tw_skills->setItem(real_count, 1, item_level);
            ui->tw_skills->setItem(real_count, 2, item_bonus);
            ui->tw_skills->setCellWidget(real_count, 3, pb);
        }
    }
    ui->tw_skills->setSortingEnabled(true);
    if(!DT->show_skill_learn_rates || no_bonuses)
        ui->tw_skills->hideColumn(2);
    else
        ui->tw_skills->showColumn(2);

    // ATTRIBUTES TABLE
    QVector<Attribute> *attributes = d->get_attributes();
    ui->tw_attributes->setSortingEnabled(false);

    for (int row = 0; row < attributes->size(); ++row) {
        ui->tw_attributes->insertRow(0);
        ui->tw_attributes->setRowHeight(0, 18);
        Attribute a = attributes->at(row);

        QTableWidgetItem *attribute_name = new QTableWidgetItem(a.get_name());

        sortableNumericTableWidgetItem *attribute_rating = new sortableNumericTableWidgetItem;
        attribute_rating->setTextAlignment(Qt::AlignHCenter);
        attribute_rating->setText(QString::number(a.display_value()));
        attribute_rating->setData(Qt::UserRole, a.display_value());
        attribute_rating->setToolTip(a.get_value_display());

        if(a.syndrome_names().count() > 0){
            attribute_name->setForeground(Attribute::color_affected_by_syns());
            tooltip = tr("<span><center><h4>%1</h4></center>%2</span>").arg(a.get_name()).arg(a.get_syndrome_desc());
        }else{
            tooltip = tr("<span><center><h4>%1</h4></center></span>").arg(a.get_name());
        }
        attribute_name->setToolTip(tooltip);

        sortableNumericTableWidgetItem *attribute_max = new sortableNumericTableWidgetItem;
        attribute_max->setTextAlignment(Qt::AlignHCenter);
        attribute_max->setText(QString::number(a.max()));
        attribute_max->setData(Qt::UserRole, a.max());
        attribute_max->setToolTip(QString("%1 (%2)").arg((int)a.max()).arg(tr("This is the maximum attainable value.")));

        //don't show the 'average for a <caste>' in the details pane
        QString lvl_msg;
        if(a.get_descriptor_rank() == 4)
            lvl_msg = "";
        else
            lvl_msg = a.get_descriptor();

        QTableWidgetItem *attribute_msg = new QTableWidgetItem(lvl_msg);
        attribute_msg->setToolTip(lvl_msg);

        if(a.get_descriptor_rank() <= 3) { //3 is the last bin before the median group
            attribute_name->setForeground(color_low);
            attribute_rating->setForeground(color_low);
            attribute_max->setForeground(color_low);
            attribute_msg->setForeground(color_low);
        }

        ui->tw_attributes->setItem(0, 0, attribute_name);
        ui->tw_attributes->setItem(0, 1, attribute_rating);
        ui->tw_attributes->setItem(0, 2, attribute_max);
        ui->tw_attributes->setItem(0, 3, attribute_msg);
    }
    ui->tw_attributes->setSortingEnabled(true);


    // PERSONALITY TABLE
    QHash<int, short> traits = d->traits();
    ui->tw_traits->setSortingEnabled(false);
    QColor trait_color;
    QList<int> conflicted_beliefs;
    foreach(int trait_id, traits.uniqueKeys()) {
        if (d->trait_is_active(trait_id))
        {
            trait_color = QColor(Qt::black);
            Trait *t = gdr->get_trait(trait_id);
            short val = traits.value(trait_id);
            //build the info message
            QStringList msg_items;
            msg_items << QString("%1%2").arg(t->level_message(val)).arg(t->belief_conficts_msgs(val,d->trait_conflicts(trait_id))) << t->skill_conflicts_msgs(val) << t->special_messages(val);
            msg_items.removeAll("");

            //build the tooltip
            if(d->trait_is_conflicted(trait_id)){
                trait_color = color_low;
                foreach(UnitBelief ub, d->trait_conflicts(trait_id)){
                    add_belief_row(ub.belief_id(),d,true); //add the conflicting cultural belief to compare
                    conflicted_beliefs.append(ub.belief_id());
                }
            }
            QString msg = msg_items.join(". ");
            QString tooltip = msg;
            QString name = t->get_name();
            if(t->valued_inversely()){
                tooltip.append(Trait::inverted_message);
                name.append('*').append(Trait::inverted_message);
            }
            add_personality_row(name,val,msg,tooltip,trait_color);
        }
    }
    //also append goals
    foreach(int id, d->goals().uniqueKeys()){
        QString name = "~" + gdr->get_goal_name(id);
        QString desc = gdr->get_goal_desc(id,d->goals().value(id));
        add_personality_row(name,d->goals().value(id)*100,desc,desc,Trait::goal_color);
    }
    //add any personal beliefs that haven't already been added when the conflicts were checked
    foreach(UnitBelief ub, d->beliefs()){
        if(d->belief_is_active(ub.belief_id()) && !conflicted_beliefs.contains(ub.belief_id()))
            add_belief_row(ub.belief_id(),d);
    }

    ui->tw_traits->setSortingEnabled(true);

    // ROLES TABLE
    QList<Role::simple_rating> roles = d->sorted_role_ratings();
    ui->tw_roles->setSortingEnabled(false);
    QString name = "";
    float val = 0.0;
    int max = DT->user_settings()->value("options/role_count_pane",10).toInt();
    if(max > roles.count())
        max = roles.count();
    for(int i = 0; i < max; i++){
        name = roles.at(i).name;
        val = roles.at(i).rating;

        ui->tw_roles->insertRow(0);
        ui->tw_roles->setRowHeight(0, 18);

        QTableWidgetItem *role_name = new QTableWidgetItem(name);
        role_name->setToolTip(tr("<center><h4>%1</h4></center>").arg(name));
        sortableNumericTableWidgetItem *role_rating = new sortableNumericTableWidgetItem();
        role_rating->setText(QString::number(val,'f',2)+"%");
        role_rating->setData(Qt::UserRole,max - i);
        role_rating->setTextAlignment(Qt::AlignHCenter);

        if (val < 50) {
            role_rating->setForeground(color_low);
        }

        ui->tw_roles->setItem(0, 0, role_name);
        ui->tw_roles->setItem(0, 1, role_rating);

        Role *r = gdr->get_role(name);
        if(r)
            role_rating->setToolTip(r->get_role_details(d));
    }
    ui->tw_roles->setSortingEnabled(true);


    // PREFERENCES TABLE
    ui->tw_prefs->setSortingEnabled(false);

    QString prefs;
    foreach(QString name, d->get_grouped_preferences().uniqueKeys()){

        prefs = capitalize(d->get_grouped_preferences().value(name)->join(", "));

        ui->tw_prefs->insertRow(0);
        ui->tw_prefs->setRowHeight(0, 18);

        QTableWidgetItem *pref_type = new QTableWidgetItem(name);
        pref_type->setToolTip(tr("<center><h4>%1</h4></center>").arg(name));
        QTableWidgetItem *pref_values = new QTableWidgetItem(prefs);

        ui->tw_prefs->setItem(0, 0, pref_type);
        ui->tw_prefs->setItem(0, 1, pref_values);

        pref_values->setToolTip("<span>" + prefs + "</span>");
    }
    ui->tw_prefs->setSortingEnabled(true);

    // HEALTH TABLE
    UnitHealth dHealth = d->get_unit_health();
    if(!dHealth.isEmpty()){
        //3 main parent nodes for treatment, status and wounds
        QTreeWidgetItem *statuses = new QTreeWidgetItem;
        statuses->setData(0, Qt::UserRole, tr("Status"));
        statuses->setText(0,  tr("Status"));

        QTreeWidgetItem *treatments = new QTreeWidgetItem;
        treatments->setData(0, Qt::UserRole, tr("Treatments"));
        treatments->setText(0,  tr("Treatments"));

        QTreeWidgetItem *wounds = new QTreeWidgetItem;
        wounds->setData(0, Qt::UserRole, tr("Wounds"));
        wounds->setText(0,  tr("Wounds"));

        //add the treatments
        QHash<eHealth::H_INFO, QList<HealthInfo*> > t_info = dHealth.get_treatment_info();
        foreach(eHealth::H_INFO h, t_info.uniqueKeys()){
            QList<HealthInfo*> infos = t_info.value(h);
            foreach(HealthInfo* hi, infos){
                QTreeWidgetItem *node = new QTreeWidgetItem(treatments);
                node->setData(0, Qt::UserRole, hi->symbol(false));
                node->setData(0,Qt::TextColorRole, hi->color());
                node->setText(0, hi->symbol(false));

                node->setData(1, Qt::UserRole, hi->description(false));
                node->setText(1,hi->description(false));
            }
        }

        //add the statuses
        QHash<eHealth::H_INFO, QList<HealthInfo*> > s_info = dHealth.get_status_info();
        foreach(eHealth::H_INFO h, s_info.uniqueKeys()){
            QList<HealthInfo*> infos = s_info.value(h);
            foreach(HealthInfo* hi, infos){
            //HealthInfo *hi = dHealth.get_most_severe(h);
                QTreeWidgetItem *node = new QTreeWidgetItem(statuses);
                node->setData(0, Qt::UserRole, hi->symbol(false));
                node->setData(0,Qt::TextColorRole, hi->color());
                node->setText(0, hi->symbol(false));

                node->setData(1, Qt::UserRole, hi->description(false));
                node->setText(1,hi->description(false));
            }
        }

        //add each wounded body part as a node, and then each wound as a node of the body part
        QHash<QString, QList<HealthInfo*> > wounds_info  = dHealth.get_wound_details();
        QList<QTreeWidgetItem*> wound_nodes;
        foreach(QString bp, wounds_info.uniqueKeys()){
            QList<HealthInfo*> infos = wounds_info.value(bp);
            //add the body part as the parent node
            QString name = capitalizeEach(bp);
            QTreeWidgetItem *body_part_node = new QTreeWidgetItem(wounds);
            body_part_node->setData(0, Qt::UserRole, name);
            body_part_node->setText(0, name);
            wound_nodes.append(body_part_node);

            foreach(HealthInfo* hi, infos){
                QTreeWidgetItem *node = new QTreeWidgetItem(body_part_node);
                node->setData(0, Qt::UserRole, hi->symbol(false));
                node->setData(0,Qt::TextColorRole, hi->color());
                node->setText(0, hi->symbol(false));

                node->setData(1, Qt::UserRole, hi->description(false));
                node->setText(1,hi->description(false));
            }
        }

        if(statuses->childCount() > 0)
            ui->tw_health->addTopLevelItem(statuses);
        if(treatments->childCount() > 0)
            ui->tw_health->addTopLevelItem(treatments);
        if(wounds->childCount() > 0){
            ui->tw_health->addTopLevelItem(wounds);
            foreach(QTreeWidgetItem *i, wound_nodes){
                i->setFirstColumnSpanned(true);
            }
            wound_nodes.clear();
        }
    }
    if(ui->tw_health->topLevelItemCount() <= 0){
        QTreeWidgetItem *noIssues = new QTreeWidgetItem;
        noIssues->setData(0, Qt::UserRole, tr("No Health Issues."));
        noIssues->setText(0,  tr("No Health Issues."));
        ui->tw_health->addTopLevelItem(noIssues);
        noIssues->setFirstColumnSpanned(true);
    }

    ui->tw_health->setSortingEnabled(true);
    ui->tw_health->sortItems(0,Qt::AscendingOrder);
    ui->tw_health->expandAll();



    m_current_id = d->id();
    d = 0;
}

void DwarfDetailsWidget::add_belief_row(int belief_id, Dwarf *d, bool is_cultural){
    Belief *b = GameDataReader::ptr()->get_belief(belief_id);
    short val = d->get_unit_belief(belief_id).belief_value();
    QString name = "~" + b->name;
    QString desc = b->level_message(val);
    QString tooltip = desc;
    QColor col = Trait::belief_color;
    if(is_cultural){
        col = color_low;
        name.append(QString("*<h5 style=\"margin:0px;\">%1</h5>").arg(tr("This is a conflicting cultural belief.")));
    }
    add_personality_row(name,val,desc,tooltip,col);
}

void DwarfDetailsWidget::add_personality_row(QString title, int raw_value, QString info, QString tooltip, QColor override){
    ui->tw_traits->insertRow(0);
    ui->tw_traits->setRowHeight(0, 18);

    QTableWidgetItem *trait_name = new QTableWidgetItem(title.mid(0,title.indexOf("<")));
    trait_name->setToolTip(tr("<center><h4 style=\"margin:0px;\">%1</h4></center>").arg(title));

    QTableWidgetItem *trait_raw = new QTableWidgetItem;
    trait_raw->setTextAlignment(Qt::AlignHCenter);
    trait_raw->setData(0, raw_value);

    QTableWidgetItem *trait_msg = new QTableWidgetItem();
    trait_msg->setText(info);
    trait_msg->setToolTip("<span>" + tooltip + "</span>");

    if(override != Qt::black){
        trait_name->setForeground(override);
        trait_raw->setForeground(override);
        trait_msg->setForeground(override);
    }

    ui->tw_traits->setItem(0, 0, trait_name);
    ui->tw_traits->setItem(0, 1, trait_raw);
    ui->tw_traits->setItem(0, 2, trait_msg);
}

void DwarfDetailsWidget::clear_table(QTableWidget &t){
    for(int i = t.rowCount(); i >=0; i--){
        t.removeRow(i);
    }
}

QString DwarfDetailsWidget::label_gradient(QColor c1, QColor c2){
    return QString("QLabel {background: QLinearGradient(x1:0,y1:0,x2:0.9,y2:0,stop:0 rgba(%1,%2,%3,%4), stop:1 rgba(%5,%6,%7,%8)); color: %9;} QLabel QTooltip {color: %10;}")
            .arg(c1.red()).arg(c1.green()).arg(c1.blue()).arg(c1.alpha())
            .arg(c2.red()).arg(c2.green()).arg(c2.blue()).arg(c2.alpha())
            .arg(complement(c1).name())
            .arg(QApplication::palette().toolTipText().color().name());
}



