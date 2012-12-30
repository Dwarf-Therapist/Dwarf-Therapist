#include "optimizereditor.h"
#include "ui_optimizereditor.h"
#include "gamedatareader.h"

#include "dwarftherapist.h"
#include "viewmanager.h"
#include "dwarfmodel.h"
#include "dwarfmodelproxy.h"

#include "laboroptimizer.h"
#include "laboroptimizerplan.h"
#include "plandetail.h"

#include "sortabletableitems.h"

#include "labor.h"
#include "math.h"

optimizereditor::optimizereditor(QString name, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::optimizereditor),
    is_editing(true)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose,true);

    m_original_plan = GameDataReader::ptr()->get_opt_plans().take(name);
    if(!m_original_plan){
        m_plan = new laborOptimizerPlan();
        m_plan->name = "New Plan";
        is_editing = false;
    }else{
        m_plan = new laborOptimizerPlan(*m_original_plan);
    }

    m_optimizer = new LaborOptimizer(m_plan,this);

    ui->sb_max_jobs->setMaximum(GameDataReader::ptr()->get_ordered_labors().count());
    ui->le_name->setText(m_plan->name);
    ui->chk_military->setChecked(m_plan->exclude_military);
    ui->chk_nobles->setChecked(m_plan->exclude_nobles);
    ui->chk_auto->setChecked(m_plan->auto_haulers);
    ui->chk_injured->setChecked(m_plan->exclude_injured);
    ui->sb_max_jobs->setValue(m_plan->max_jobs_per_dwarf);
    ui->sb_pop_percent->setValue(m_plan->pop_percent);
    ui->sb_hauler_percent->setValue(m_plan->hauler_percent);

    ui->lbl_jobs->setToolTip("The total number of possible job slots available (workers x jobs per worker).");
    ui->lbl_workers->setToolTip("The number of job slots assigned.");
    ui->lbl_counts->setStatusTip("The current population numbers are dependent on the current view, including any filters or selections.");

    //job/labor table
    ui->tw_labors->setEditTriggers(QTableWidget::AllEditTriggers);
    ui->tw_labors->verticalHeader()->hide();
    ui->tw_labors->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
    ui->tw_labors->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_labors->horizontalHeader()->setResizeMode(2, QHeaderView::ResizeToContents);
    ui->tw_labors->horizontalHeader()->setResizeMode(3, QHeaderView::ResizeToContents);
    ui->tw_labors->horizontalHeader()->setResizeMode(4, QHeaderView::ResizeToContents);    
    //ui->tw_labors->setHorizontalHeaderLabels(QStringList() << "Job" << "Role" << "Priority" << "Max Workers (%)" << "Worker Count");
    ui->tw_labors->setHorizontalHeaderLabels(QStringList() << "Job" << "Role" << "Priority" << "Ratio" << "Worker Count");

    ui->tw_labors->horizontalHeaderItem(2)->setToolTip(tr("The role or skill used to rank dwarves for the job."));
    ui->tw_labors->horizontalHeaderItem(2)->setToolTip(tr("Determines how important assigning workers to the job is, in relation to other jobs."));    
    ui->tw_labors->horizontalHeaderItem(3)->setToolTip(tr("Represents the ratio of the population which should be assigned to the job. For example setting 1 for each job will distribute workers evenly among all jobs."));
    ui->tw_labors->horizontalHeaderItem(4)->setToolTip(tr("The actual number of dwarves which will be assigned to the job"));

    find_target_population();

    loading = true;
    foreach(PlanDetail *d, m_plan->plan_details){
        insert_row(d);
    }
    ui->tw_labors->resizeRowsToContents();
    loading = false;

    refresh_job_counts();


    connect(ui->btnOptimize, SIGNAL(clicked()), this, SLOT(test_optimize()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(save_pressed()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(cancel_pressed()));

    connect(ui->sb_max_jobs, SIGNAL(valueChanged(int)), this, SLOT(max_jobs_changed(int)));
    connect(ui->sb_pop_percent, SIGNAL(valueChanged(int)), this, SLOT(pop_percent_changed(int)));

    connect(ui->chk_military, SIGNAL(stateChanged(int)), this, SLOT(filter_option_changed()));
    connect(ui->chk_nobles, SIGNAL(stateChanged(int)), this, SLOT(filter_option_changed()));
    connect(ui->chk_injured, SIGNAL(stateChanged(int)), this, SLOT(filter_option_changed()));

    connect(ui->sb_hauler_percent, SIGNAL(valueChanged(int)), this, SLOT(hauler_percent_changed(int)));
    connect(ui->chk_auto, SIGNAL(stateChanged(int)), this, SLOT(auto_haul_changed(int)));

    connect(ui->btnImport, SIGNAL(clicked()), this, SLOT(import_details()));
    connect(ui->btnExport, SIGNAL(clicked()), this, SLOT(export_details()));

    connect(ui->tw_labors, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(draw_labor_context_menu(QPoint)));
    connect(ui->tw_labors->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), ui->tw_labors, SLOT(resizeColumnsToContents()));
    connect(ui->tw_labors->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), ui->tw_labors, SLOT(resizeRowsToContents()));

    connect(ui->treeMessages, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
        DT->get_main_window()->get_view_manager(), SLOT(jump_to_dwarf(QTreeWidgetItem *, QTreeWidgetItem *)));

    //update some of the tooltips
    hauler_percent_changed(ui->sb_hauler_percent->value());
    max_jobs_changed(m_plan->max_jobs_per_dwarf);

    //setup the splitter options, and decorate the handle
    ui->splitter->setStretchFactor(0,200);
    ui->splitter->setStretchFactor(1,1);

    QSplitterHandle *h = ui->splitter->handle(1);
    QVBoxLayout *layout = new QVBoxLayout(h);
    layout->setSpacing(0);
    layout->setMargin(0);
    QFrame *line = new QFrame(h);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);
    QLabel *l = new QLabel(h);
    l->setText("Optimization Log");
    l->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    l->setMaximumHeight(20);

    QPalette pal = h->palette();
    pal.setColor(h->backgroundRole(), QColor(139,137,137));
    h->setPalette(pal);

    layout->addWidget(l);
    ui->splitter->setHandleWidth(l->height());
    ui->splitter->setChildrenCollapsible(false);

}

void optimizereditor::max_jobs_changed(int val){
    m_plan->max_jobs_per_dwarf = val;
    refresh_job_counts();
}

void optimizereditor::hauler_percent_changed(int val){
    m_plan->hauler_percent = val;
}

void optimizereditor::auto_haul_changed(int val){
    m_plan->auto_haulers = val;
}

void optimizereditor::pop_percent_changed(int val){    
    m_plan->pop_percent = val;
    populationChanged();
}

void optimizereditor::insert_row(PlanDetail *d){
    QPair<QString, Role*> role_pair;
    Labor *l = GameDataReader::ptr()->get_labor(d->labor_id);

    ui->tw_labors->setSortingEnabled(false);
    QString title;
    int row = ui->tw_labors->rowCount();
    ui->tw_labors->insertRow(row);    
    ui->tw_labors->setRowHeight(row,18);

    title = l->name;
    title[0]=title[0].toUpper();
    QTableWidgetItem *name = new QTableWidgetItem();
    name->setData(0,title);
    name->setData(Qt::UserRole,d->labor_id);
    name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->tw_labors->setItem(row,0,name);

    QComboBox *cmb_roles = new QComboBox();
    cmb_roles->addItem("None (Use Labor's Skill)", "");
    foreach(role_pair, GameDataReader::ptr()->get_ordered_roles()){
        cmb_roles->addItem(role_pair.first, role_pair.first);
    }
    int index = cmb_roles->findText(d->role_name);
    if(index != -1)
        cmb_roles->setCurrentIndex(index);
    else
        cmb_roles->setCurrentIndex(0);
    connect(cmb_roles, SIGNAL(currentIndexChanged(QString)), this, SLOT(role_changed(QString)));
    ui->tw_labors->setCellWidget(row, 1, cmb_roles);
    sortableComboItem* cmbitem = new sortableComboItem;
    ui->tw_labors->setItem(row, 1, cmbitem);

    QDoubleSpinBox *sb_priority = new QDoubleSpinBox();
    sb_priority->setMinimum(0.01);
    sb_priority->setMaximum(1);
    sb_priority->setSingleStep(0.1);
    sb_priority->setDecimals(4);
    sb_priority->setWrapping(true);
    sb_priority->setValue(d->priority);
    connect(sb_priority, SIGNAL(valueChanged(double)), this, SLOT(priority_changed(double)));
    ui->tw_labors->setCellWidget(row,2,sb_priority);
    sortableTableWidgetItem* item = new sortableTableWidgetItem;
    ui->tw_labors->setItem(row, 2, item);

    QDoubleSpinBox *sb_max = new QDoubleSpinBox();
    sb_max->setMinimum(0.01);
    sb_max->setMaximum(100.0);
    sb_max->setSingleStep(0.1);
    sb_max->setWrapping(true);
    sb_max->setValue(d->ratio);
    connect(sb_max, SIGNAL(valueChanged(double)), this, SLOT(ratio_changed(double)));
    ui->tw_labors->setCellWidget(row,3,sb_max);
    item = new sortableTableWidgetItem;
    ui->tw_labors->setItem(row, 3, item);

    QTableWidgetItem *count = new QTableWidgetItem();
    count->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    count->setTextAlignment(Qt::AlignHCenter);
    ui->tw_labors->setItem(row,4,count);

    ui->tw_labors->setSortingEnabled(true);

    //refresh_all_counts();
    if(!loading){
        refresh_job_counts();
    }
}

void optimizereditor::priority_changed(double val){
    QWidget *w = QApplication::focusWidget();
    if(w){
        QModelIndex idx = ui->tw_labors->indexAt(w->pos());
        PlanDetail *det = m_plan->job_exists(ui->tw_labors->item(idx.row(),0)->data(Qt::UserRole).toInt());
        if(det)
            det->priority = val;
    }

    refresh_job_counts();
}

void optimizereditor::ratio_changed(double val){
    QWidget *w = QApplication::focusWidget();
    if(w){
        QModelIndex idx = ui->tw_labors->indexAt(w->pos());
        PlanDetail *det = m_plan->job_exists(ui->tw_labors->item(idx.row(),0)->data(Qt::UserRole).toInt());
        if(det)
            det->ratio = val;        
    }

    refresh_job_counts();
}

void optimizereditor::role_changed(QString val){
    QWidget *w = QApplication::focusWidget();
    if(w){
        QModelIndex idx = ui->tw_labors->indexAt(w->pos());
        PlanDetail *det = m_plan->job_exists(ui->tw_labors->item(idx.row(),0)->data(Qt::UserRole).toInt());
        if(det)
            det->role_name = val;
    }
}

void optimizereditor::refresh_job_counts(){
    refresh_actual_counts();

    ui->lbl_jobs->setText("Total Jobs: " + QString::number(m_optimizer->total_raw_jobs()));
    ui->lbl_workers->setText("Assigned: ~" + QString::number(m_optimizer->assigned_jobs()));

    QString pops = QString::number(m_optimizer->targeted_population()) + "/" + QString::number(m_optimizer->total_population());
    QString msg = "Approximately " + pops + " dwarves will be assigned the majority of jobs.";
    ui->lbl_counts->setText(pops);
    ui->lbl_counts->setToolTip(msg);

    ui->tw_labors->horizontalHeaderItem(3)->setToolTip(tr("Represents the ratio of the total job slots (%1) which should be assigned to the job.")
                                                       .arg(QString::number(m_optimizer->total_raw_jobs())));
}

void optimizereditor::filter_option_changed(){
    m_plan->exclude_injured = ui->chk_injured->isChecked();
    m_plan->exclude_military = ui->chk_military->isChecked();
    m_plan->exclude_nobles = ui->chk_nobles->isChecked();

    populationChanged();
}

void optimizereditor::populationChanged(){
    find_target_population(); //update population and refresh counts
    refresh_job_counts();
}


void optimizereditor::refresh_actual_counts(){
    m_optimizer->update_ratios();

    for(int i = 0; i < ui->tw_labors->rowCount(); i++){
        PlanDetail *det = m_plan->job_exists(ui->tw_labors->item(i,0)->data(Qt::UserRole).toInt());
        if(det)
            ui->tw_labors->item(i,4)->setData(0, det->max_count);
    }
}

QList<Dwarf *> optimizereditor::get_dwarfs(){
    QList<Dwarf*> user_selection = DT->get_main_window()->get_view_manager()->get_selected_dwarfs();

    if(user_selection.count() <= 0)
        user_selection = DT->get_main_window()->get_proxy()->get_filtered_dwarves();

    return user_selection;
}

void optimizereditor::find_target_population(){
    m_optimizer->update_population(get_dwarfs());
    m_optimizer->calc_population();
    refresh_job_counts();
}

void optimizereditor::draw_labor_context_menu(const QPoint &p){
    QMenu *m = new QMenu("",this);
    QModelIndex idx = ui->tw_labors->indexAt(p);
    if (idx.isValid()) { // context on a row
        m->addAction(QIcon(":img/delete.png"), tr("Remove Selected"), this, SLOT(remove_labor()));
        m->addSeparator();
    }
    QAction *a;
    GameDataReader *gdr = GameDataReader::ptr();

    QIcon icn(":img/add.png");

    QMenu *labor_a_l = m->addMenu(icn, tr("A-I"));
    labor_a_l->setTearOffEnabled(true);
    QMenu *labor_j_r = m->addMenu(icn, tr("J-R"));
    labor_j_r->setTearOffEnabled(true);
    QMenu *labor_s_z = m->addMenu(icn, tr("S-Z"));
    labor_s_z->setTearOffEnabled(true);

    bool exists;
    qDeleteAll(m_remaining_labors);
    m_remaining_labors.clear();
    foreach(Labor *l, gdr->get_ordered_labors()) {
        exists = false;
        for(int i = 0; i < ui->tw_labors->rowCount(); i++){
            if(ui->tw_labors->item(i,0)->data(Qt::UserRole).toInt() == l->labor_id)
                exists = true;
        }
        if(!exists && !l->is_hauling){
            QMenu *menu_to_use = labor_a_l;
            if (l->name.at(0).toLower() > 'i')
                menu_to_use = labor_j_r;
            if (l->name.at(0).toLower() > 'r')
                menu_to_use = labor_s_z;
            a = menu_to_use->addAction(l->name, this, SLOT(add_job()));
            a->setData(l->labor_id);
            //save the labor in case of add remaining
            m_remaining_labors << l;
        }
    }

    QString msg = tr("A-I (%1 jobs)").arg(QString::number(labor_a_l->children().count()));
    labor_a_l->setWindowTitle(msg);
    labor_a_l->setTitle(msg);

    msg = tr("J-R (%1 jobs)").arg(QString::number(labor_j_r->children().count()));
    labor_j_r->setWindowTitle(msg);
    labor_j_r->setTitle(msg);

    msg = tr("S-Z (%1 jobs)").arg(QString::number(labor_s_z->children().count()));
    labor_s_z->setWindowTitle(msg);
    labor_s_z->setTitle(msg);

    m->addSeparator();
    a = m->addAction(icn,tr("Assign remaining %1 jobs.")
                     .arg(QString::number(m_remaining_labors.count())),this, SLOT(add_remaining_jobs()));

    m->exec(ui->tw_labors->viewport()->mapToGlobal(p));
}

void optimizereditor::add_remaining_jobs(){
    foreach(Labor *l, m_remaining_labors){
        add_new_detail(l->labor_id);
    }
}

void optimizereditor::add_job(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    int id = a->data().toInt();
    add_new_detail(id);
    ui->tw_labors->resizeColumnsToContents();
    ui->tw_labors->resizeRowsToContents();
}
void optimizereditor::remove_labor(){
    for(int i=ui->tw_labors->rowCount()-1; i>=0; i--){
        if(ui->tw_labors->item(i,0)->isSelected()){
            m_plan->remove_job(ui->tw_labors->item(i,0)->data(Qt::UserRole).toInt());
            ui->tw_labors->removeRow(i);
        }
    }
    refresh_job_counts();
    ui->tw_labors->resizeColumnsToContents();
    ui->tw_labors->resizeRowsToContents();
}

void optimizereditor::add_new_detail(int id){
    if(!m_plan->job_exists(id)){
        PlanDetail *d = new PlanDetail();
        d->assigned_laborers = 0;
        d->labor_id = id;
        d->ratio = 1;
        d->priority = 1;
        d->role_name = find_role(id);

        m_plan->plan_details.append(d);
        insert_row(d);
    }else{
        display_message(GameDataReader::ptr()->get_labor(id)->name + " has not been added because it already exists!");
    }
}

QString optimizereditor::find_role(int id){
    Labor *l = GameDataReader::ptr()->get_labor(id);

    //find first related role
    QVector<Role*> roles = GameDataReader::ptr()->get_skill_roles().value(l->skill_id);
    if(roles.count() > 0){
        return roles.at(0)->name;
    }else{
        return "";
    }
}

void optimizereditor::save(laborOptimizerPlan *p){
    p->exclude_military = ui->chk_military->isChecked();
    p->exclude_nobles = ui->chk_nobles->isChecked();
    p->exclude_injured = ui->chk_injured->isChecked();
    p->max_jobs_per_dwarf = ui->sb_max_jobs->value();
    p->hauler_percent = ui->sb_hauler_percent->value();
    p->pop_percent = ui->sb_pop_percent->value();
    p->auto_haulers = ui->chk_auto->isChecked();    
    p->name = ui->le_name->text();
    //save_details(p);
}

void optimizereditor::test_optimize(){
    clear_log();

    save(m_plan);

    connect(m_optimizer, SIGNAL(optimize_message(QVector<QPair<int, QString> >)), this, SLOT(display_message(QVector<QPair<int, QString> >)));

    find_target_population();

    m_optimizer->optimize_labors(get_dwarfs());
    DT->get_main_window()->get_model()->calculate_pending();
    DT->emit_labor_counts_updated();
    disconnect(m_optimizer, SIGNAL(optimize_message(QVector<QPair<int, QString> >)), this, SLOT(display_message(QVector<QPair<int, QString> >)));
}

void optimizereditor::display_message(QString msg){
    QVector<QPair<int, QString> > new_msg;
    new_msg.append(QPair<int,QString>(0,msg));
    display_message(new_msg);
}

void optimizereditor::display_message(QVector<QPair<int, QString> > messages){
    ui->treeMessages->blockSignals(true);

    QTreeWidgetItem *p_item = new QTreeWidgetItem;
    p_item->setData(0, Qt::UserRole, messages.at(0).first);
    p_item->setText(0, messages.at(0).second);

    if(messages.count() > 1){
        for(int i = 1; i < messages.count(); i++){
            QTreeWidgetItem *item = new QTreeWidgetItem(p_item);
            item->setData(0, Qt::UserRole, messages.at(i).first);
            item->setText(0, messages.at(i).second);
        }
    }

    ui->treeMessages->addTopLevelItem(p_item);
    ui->treeMessages->collapseAll();

    if(ui->treeMessages->height() <= 5){
        QList<int> sizes;
        sizes.append(ui->treeMessages->height() - 15);
        sizes.append(15);
        ui->splitter->setSizes(sizes);
    }

    ui->treeMessages->blockSignals(false);
}

void optimizereditor::clear_log(){
    ui->treeMessages->clear();
}

void optimizereditor::save_pressed(){
    if(GameDataReader::ptr()->get_opt_plan(ui->le_name->text())){
        if(QMessageBox::question(
                    0, tr("Confirm Copy"),
                    tr("An optimization plan with this name already exists, continue and overwrite?"),
                    QMessageBox::Yes | QMessageBox::No) == QMessageBox::No){
            return;
        }
    }

    save(m_plan);

    if(ui->le_name->text().trimmed().isEmpty()){
        QMessageBox::critical(this,tr("Invalid Plan Name"),tr("Please enter a name for this optimization plan."));
        return;
    }

    //update the DT list
    GameDataReader::ptr()->get_opt_plans().insert(ui->le_name->text(),m_plan);

    //notify we're done
    this->accept();
    disconnect();
}

void optimizereditor::cancel_pressed(){
    //if we were editing and cancelled, put the original back!
    if(is_editing)
        GameDataReader::ptr()->get_opt_plans().insert(m_original_plan->name, m_original_plan);
    this->reject();
    disconnect();
}


void optimizereditor::import_details(){
    clear_log();
    QString path = QFileDialog::getOpenFileName(this, tr("CSV Import"),"",tr("CSV (*.csv);;All Files (*)"));

    QFile file(path);
     if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
         display_message("File could not be opened.");
         return;
     }

     //clear current rows
     for(int i=ui->tw_labors->rowCount()-1; i>=0; i--){
         ui->tw_labors->removeRow(i);
     }

     QTextStream s(&file);
     int linenum = 0;
     QString line;
     QStringList fields;     
     bool check;     
     loading = true;
     while (!s.atEnd()) {
         linenum++;
         line = s.readLine();
         fields = line.split(",");

         if(fields.count() == 4){
             fields.at(1).toInt(&check);
             if(!check){
                 display_message("Invalid labor id (2nd column) at line " + QString::number(linenum));
                 continue;
             }
             fields.at(2).toFloat(&check);
             if(!check){
                 display_message("Invalid priority (3rd column) at line " + QString::number(linenum));
                 continue;
             }
             fields.at(3).toFloat(&check);
             if(!check){
                 display_message("Invalid population percent (4th column) at line " + QString::number(linenum));
                 continue;
             }

             PlanDetail *d = new PlanDetail();
             d->role_name = fields.at(0);
             d->labor_id = fields.at(1).toInt();
             d->priority = fields.at(2).toFloat();
             d->ratio = fields.at(3).toFloat();
             d->assigned_laborers = 0;

             //if the csv has a role we don't have, find a match if possible and report
             if(GameDataReader::ptr()->get_role(d->role_name) == NULL){
                 QString role_name = find_role(d->labor_id);
                 QString msg = "";
                 if(!role_name.isEmpty())
                     msg = "Role [" + role_name + "] was used instead.";
                 display_message("The role for " + GameDataReader::ptr()->get_labor(d->labor_id)->name +
                                 " [" + d->role_name + "] could not be found. " +
                                         msg + " (line " + QString::number(linenum) + ")");
                 d->role_name = role_name;
             }

             if(!m_plan->job_exists(d->labor_id)){
                 m_plan->plan_details.append(d);
                 insert_row(d);
             }

         }else if(fields.count() == 8 && linenum == 1){
             ui->le_name->setText(fields.at(0));
             ui->sb_max_jobs->setValue(fields.at(1).toInt());
             ui->sb_pop_percent->setValue(fields.at(2).toFloat());
             ui->sb_hauler_percent->setValue(fields.at(3).toFloat());
             ui->chk_military->setChecked(fields.at(4).toInt());
             ui->chk_nobles->setChecked(fields.at(5).toInt());
             ui->chk_injured->setChecked(fields.at(6).toInt());
             ui->chk_auto->setChecked(fields.at(7).toInt());
             save(m_plan);
         }else{
             display_message(tr("CSV has an invalid number of columns (%1) at line %2.")
                             .arg(QString::number(fields.count())).arg(QString::number(linenum)));
         }

     }     
     loading = false;
     file.close();
     ui->tw_labors->resizeColumnsToContents();
     ui->tw_labors->resizeRowsToContents();

     refresh_job_counts();

     display_message("Import complete.");
}

void optimizereditor::export_details(){
    clear_log();
    QString path = QFileDialog::getSaveFileName(this, tr("CSV Export"),m_plan->name,tr("CSV (*.csv);;All Files (*)"));

    QFile file(path);
     if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
         display_message("File could not be opened.");
         return;
     }

     QTextStream s(&file);

     s << ui->le_name->text() + ",";
     s << QString::number(ui->sb_max_jobs->value()) + ",";
     s << QString::number(ui->sb_pop_percent->value()) + ",";
     s << QString::number(ui->sb_hauler_percent->value()) + ",";
     s << QString::number((int)ui->chk_military->isChecked()) + ",";
     s << QString::number((int)ui->chk_nobles->isChecked()) + ",";
     s << QString::number((int)ui->chk_injured->isChecked()) + ",";
     s << QString::number((int)ui->chk_auto->isChecked()) + "\n";

     for(int i = 0; i < ui->tw_labors->rowCount(); i++){
         QComboBox *c = static_cast<QComboBox*>(ui->tw_labors->cellWidget(i,1));
         s << c->itemData(c->currentIndex(), Qt::UserRole).toString() + ",";
         s << ui->tw_labors->item(i,0)->data(Qt::UserRole).toString() + ",";
         s << (float)static_cast<QDoubleSpinBox*>(ui->tw_labors->cellWidget(i,2))->value();
         s << ",";
         s << static_cast<QDoubleSpinBox*>(ui->tw_labors->cellWidget(i,3))->value();
         s << "\n";
     }
     file.close();
     display_message("Export complete.");
}

bool optimizereditor::event(QEvent *evt) {
    if (evt->type() == QEvent::StatusTip) {
        ui->txt_help->setHtml(static_cast<QStatusTipEvent*>(evt)->tip());
        return true; // we've handled it, don't pass it
    }
    return QWidget::event(evt); // pass the event along the chain
}

optimizereditor::~optimizereditor()
{
    delete ui;
}
