#include "roledialog.h"
#include "ui_roledialog.h"
#include "gamedatareader.h"
#include "attribute.h"
#include "skill.h"
#include "trait.h"
#include "dwarftherapist.h"
#include "viewcolumn.h"
#include "viewcolumnset.h"
#include "viewmanager.h"
#include "mainwindow.h"
#include "gridview.h"
#include "rolecolumn.h"
roleDialog::~roleDialog()
{
    delete ui;
}

roleDialog::roleDialog(QWidget *parent, QString name) :
    QDialog(parent),
    ui(new Ui::roleDialog)
{
    ui->setupUi(this);
    m_override = false;
    m_role = GameDataReader::ptr()->get_roles().take(name);
    if(!m_role){
        m_role = new Role();
        m_role->is_custom = true;
        m_role->name = "";
    }

    //attributes table
    ui->tw_attributes->setEditTriggers(QTableWidget::AllEditTriggers);
    ui->tw_attributes->verticalHeader()->hide();
    ui->tw_attributes->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
    ui->tw_attributes->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_attributes->setHorizontalHeaderLabels(QStringList() << "Attribute" << "Weight");
    //skills table
    ui->tw_skills->setEditTriggers(QTableWidget::AllEditTriggers);
    ui->tw_skills->verticalHeader()->hide();
    ui->tw_skills->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
    ui->tw_skills->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_skills->setHorizontalHeaderLabels(QStringList() << "Skill" << "Weight");
    //traits table
    ui->tw_traits->setEditTriggers(QTableWidget::AllEditTriggers);
    ui->tw_traits->verticalHeader()->hide();
    ui->tw_traits->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
    ui->tw_traits->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_traits->setHorizontalHeaderLabels(QStringList() << "Trait" << "Weight");

    //setup copy combo
    QList<QPair<QString, Role*> > roles = GameDataReader::ptr()->get_ordered_roles();
    QPair<QString, Role*> role_pair;
    foreach(role_pair, roles){
        if(role_pair.first != m_role->name)
            ui->cmb_copy->addItem(role_pair.first);
    }

    //load data
    load_role_data();

    connect(ui->btn_cancel, SIGNAL(clicked()), SLOT(close_pressed()));
    connect(ui->btn_save, SIGNAL(clicked()), SLOT(save_pressed()));
    connect(ui->btn_copy, SIGNAL(clicked()), SLOT(copy_pressed()));

    connect(ui->tw_attributes, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_attribute_context_menu(const QPoint &)));
    connect(ui->tw_traits, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_trait_context_menu(const QPoint &)));
    connect(ui->tw_skills, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_skill_context_menu(const QPoint &)));

    connect(ui->le_role_name, SIGNAL(textChanged(QString)), SLOT(name_changed(QString)));

    //set status tip info for weights
    QString stat = tr("This weight is the importance of %1 relative to %2 and %3. A higher weight gives more value. The current default is %4.");
    ui->dsb_attributes_weight->setStatusTip(stat.arg("attributes").arg("traits").arg("skills")
                                            .arg(DT->user_settings()->value("options/default_attributes_weight",1.0).toString()));
    ui->dsb_skills_weight->setStatusTip(stat.arg("skills").arg("attributes").arg("traits")
                                            .arg(DT->user_settings()->value("options/default_skills_weight",1.0).toString()));
    ui->dsb_traits_weight->setStatusTip(stat.arg("traits").arg("attributes").arg("skills")
                                            .arg(DT->user_settings()->value("options/default_traits_weight",1.0).toString()));

    color_override = QColor::fromRgb(153,102,34,255);
    color_default = QColor::fromRgb(255,255,255,255);
    name_changed(ui->le_role_name->text());
}

void roleDialog::load_role_data(){
    ui->le_role_name->setText(m_role->name);
    //global weights
    ui->dsb_attributes_weight->setValue(m_role->attributes_weight.weight);
    ui->dsb_traits_weight->setValue(m_role->traits_weight.weight);
    ui->dsb_skills_weight->setValue(m_role->skills_weight.weight);

    //load aspects
    load_aspects_data(*ui->tw_attributes, m_role->attributes);
    load_aspects_data(*ui->tw_traits, m_role->traits);
    load_aspects_data(*ui->tw_skills, m_role->skills);
}

void roleDialog::load_aspects_data(QTableWidget &table, QHash<QString,Role::aspect> aspects){
    Role::aspect a;
    table.setSortingEnabled(false);
    foreach(QString key, aspects.uniqueKeys()){
        a = aspects.value(key);
        insert_row(table,a,key);
    }
    table.setSortingEnabled(true);
    table.sortItems(0,Qt::AscendingOrder);
}

void roleDialog::insert_row(QTableWidget &table, Role::aspect a, QString key){
    table.setSortingEnabled(false);
    QString title;
    int row = table.rowCount();
    table.insertRow(row);
    table.setRowHeight(row,18);

    if(&table==ui->tw_skills)
        title=GameDataReader::ptr()->get_skill_name(key.toShort());
    else if(&table==ui->tw_traits)
        title = GameDataReader::ptr()->get_trait_name(key.toShort());
    else
        title = key.toLower();

    title[0]=title[0].toUpper();
    QTableWidgetItem *name = new QTableWidgetItem();
    name->setData(0,title);
    name->setData(Qt::UserRole,key);
    name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    table.setItem(row,0,name);

    QDoubleSpinBox *dbs = new QDoubleSpinBox();
    dbs->setMinimum(-100);
    dbs->setMaximum(100);
    dbs->setSingleStep(0.25);
    dbs->setValue(a.is_neg ? 0-a.weight : a.weight);
    table.setCellWidget(row,1,dbs);
    table.setSortingEnabled(true);
}

void roleDialog::add_aspect(QString id, QTableWidget &table, QHash<QString, Role::aspect> &list){
    Role::aspect asp;
    asp.is_neg = false;
    asp.weight = 1.0;
    list.insert(id.toLower(), asp);
    insert_row(table,asp,id.toLower());
}

void roleDialog::save_pressed(){
    QString new_name = ui->le_role_name->text().trimmed();
    if(new_name.trimmed().isEmpty()){
        QMessageBox::critical(this,"Invalid Role Name","Role names cannot be blank.");
        return;
    }
    //update the role
    m_role->is_custom = true;

    //attributes
    m_role->attributes_weight.weight = ui->dsb_attributes_weight->value();
    save_aspects(*ui->tw_attributes, m_role->attributes);

    //skills
    m_role->skills_weight.weight = ui->dsb_skills_weight->value();
    save_aspects(*ui->tw_skills,m_role->skills);

    //traits
    m_role->traits_weight.weight = ui->dsb_traits_weight->value();
    save_aspects(*ui->tw_traits,m_role->traits);

    //if we're updating/adding a role to replace a default remove the default role first
    if(m_override)
        GameDataReader::ptr()->get_roles().remove(m_role->name);

    m_role->name = new_name;
    m_role->create_role_details(*DT->user_settings());
    GameDataReader::ptr()->get_roles().insert(new_name,m_role);

    //exit
    this->accept();
}

void roleDialog::save_aspects(QTableWidget &table, QHash<QString,Role::aspect> &list){
    Role::aspect a;
    for(int i= 0; i<table.rowCount(); i++){
        QString key = table.item(i,0)->data(Qt::UserRole).toString();
        float weight = static_cast<QDoubleSpinBox*>(table.cellWidget(i,1))->value();
        a.weight = fabs(weight);
        a.is_neg = weight < 0 ? true : false;
        list.insert(key,a);
    }
}

void roleDialog::close_pressed(){
    //if we were editing and cancelled, put the role back!
    if(m_role && !m_role->name.trimmed().isEmpty())
        GameDataReader::ptr()->get_roles().insert(m_role->name,m_role);
    this->reject();
}

//ATTRIBUTE CONTEXT MENUS
void roleDialog::draw_attribute_context_menu(const QPoint &p) {
    QMenu *m = new QMenu("Add Attribute",this);
    QModelIndex idx = ui->tw_attributes->indexAt(p);
    if (idx.isValid()) { // context on a row
        m->addAction(QIcon(":img/delete.png"), tr("Remove Selected"), this, SLOT(remove_attribute()));
        m->addSeparator();
    }
    QAction *a;
    GameDataReader *gdr = GameDataReader::ptr();

    QList<QPair<int, Attribute*> > atts = gdr->get_ordered_attributes();
    QPair<int, Attribute*> att_pair;
    foreach(att_pair, atts){
        if(!m_role->attributes.contains(att_pair.second->name.toLatin1().toLower())){
            a = m->addAction(tr(att_pair.second->name.toLatin1()), this, SLOT(add_attribute()));
            a->setData(att_pair.second->name.toLatin1());
            a->setToolTip(tr("Include %1 as an aspect for this role.").arg(att_pair.second->name));
        }
    }

    m->exec(ui->tw_attributes->viewport()->mapToGlobal(p));
}

void roleDialog::add_attribute(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    QString name = a->data().toString();
    add_aspect(name,*ui->tw_attributes,m_role->attributes);
}

void roleDialog::remove_attribute(){
    for(int i=ui->tw_attributes->rowCount()-1; i>=0; i--){
        if(ui->tw_attributes->item(i,0)->isSelected()){
            m_role->attributes.remove(ui->tw_attributes->item(i,0)->data(Qt::UserRole).toString().toLower());
            ui->tw_attributes->removeRow(i);
        }
    }
}

//SKILLS CONTEXT MENUS
void roleDialog::draw_skill_context_menu(const QPoint &p) {
    QMenu *m = new QMenu("Add Skill",this);
    QModelIndex idx = ui->tw_skills->indexAt(p);
    if (idx.isValid()) { // context on a row
        m->addAction(QIcon(":img/delete.png"), tr("Remove Selected"), this, SLOT(remove_skill()));
        m->addSeparator();
    }
    QAction *a;
    GameDataReader *gdr = GameDataReader::ptr();

    QMenu *skill_a_l = m->addMenu(tr("A-I"));
    QMenu *skill_j_r = m->addMenu(tr("J-R"));
    QMenu *skill_m_z = m->addMenu(tr("S-Z"));
    QPair<int, QString> skill_pair;
    foreach(skill_pair, gdr->get_ordered_skills()) {
        if(!m_role->skills.contains((QString)skill_pair.first)){
            QMenu *menu_to_use = skill_a_l;
            if (skill_pair.second.at(0).toLower() > 'i')
                menu_to_use = skill_j_r;
            if (skill_pair.second.at(0).toLower() > 'r')
                menu_to_use = skill_m_z;
            a = menu_to_use->addAction(skill_pair.second, this, SLOT(add_skill()));
            a->setData(skill_pair.first);
            a->setToolTip(tr("Include %1 (ID %2) as an aspect for this role.)").arg(skill_pair.second).arg(skill_pair.first));
        }
    }

    m->exec(ui->tw_skills->viewport()->mapToGlobal(p));
}

void roleDialog::add_skill(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    QString id = a->data().toString();
    add_aspect(id,*ui->tw_skills,m_role->skills);
}
void roleDialog::remove_skill(){
    for(int i=ui->tw_skills->rowCount()-1; i>=0; i--){
        if(ui->tw_skills->item(i,0)->isSelected()){
            m_role->skills.remove(ui->tw_skills->item(i,0)->data(Qt::UserRole).toString().toLower());
            ui->tw_skills->removeRow(i);
        }
    }    
}


//TRAIT CONTEXT MENUS
void roleDialog::draw_trait_context_menu(const QPoint &p) {
    QMenu *m = new QMenu("Add Trait",this);
    QModelIndex idx = ui->tw_traits->indexAt(p);
    if (idx.isValid()) { // context on a row
        m->addAction(QIcon(":img/delete.png"), tr("Remove Selected"), this, SLOT(remove_trait()));
        m->addSeparator();
    }
    QAction *a;
    GameDataReader *gdr = GameDataReader::ptr();

    QList<QPair<int, Trait*> > traits = gdr->get_ordered_traits();
    QPair<int, Trait*> trait_pair;
    foreach(trait_pair, traits) {
        if(!m_role->traits.contains((QString)trait_pair.first)){
            Trait *t = trait_pair.second;
            a = m->addAction(t->name, this, SLOT(add_trait()));
            a->setData(trait_pair.first);
            a->setToolTip(tr("Include %1 (ID %2) as an aspect for this role.").arg(t->name).arg(trait_pair.first));
        }
    }

    m->exec(ui->tw_traits->viewport()->mapToGlobal(p));
}

void roleDialog::add_trait(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    QString id = a->data().toString();
    add_aspect(id,*ui->tw_traits,m_role->traits);
}
void roleDialog::remove_trait(){
    for(int i=ui->tw_traits->rowCount()-1; i>=0; i--){
        if(ui->tw_traits->item(i,0)->isSelected()){
            m_role->traits.remove(ui->tw_traits->item(i,0)->data(Qt::UserRole).toString().toLower());
            ui->tw_traits->removeRow(i);
        }
    }    
}

void roleDialog::copy_pressed(){
    int answer = QMessageBox::Yes;

    if(ui->tw_attributes->rowCount()>0 || ui->tw_traits->rowCount()>0 || ui->tw_skills->rowCount()>0){
        answer = QMessageBox::question(
                    0, tr("Confirm Copy"),
                    tr("Copying this role will replace all current aspects with the selected role's aspects. Continue?"),
                    QMessageBox::Yes | QMessageBox::No);
    }

    if (answer == QMessageBox::Yes){
        QString name = ui->le_role_name->text().trimmed();
        //remove the current role if editing
        //GameDataReader::ptr()->get_roles.remove(name);

        //load up the copied role, with our custom name, if no name exists keep the name the same as the
        //copy assuming they want to override the default role
        Role *copy = GameDataReader::ptr()->get_role(ui->cmb_copy->currentText());
        m_role = new Role(*copy);
        if(name=="")
            name = m_role->name;
        m_role->name = name;

        //clear tables
        clear_table(*ui->tw_attributes);
        clear_table(*ui->tw_skills);
        clear_table(*ui->tw_traits);

        //load new data
        load_role_data();
    }
}

void roleDialog::clear_table(QTableWidget &t){
    for(int i = t.rowCount(); i >=0; i--){
        t.removeRow(i);
    }
}

bool roleDialog::event(QEvent *evt) {
    if (evt->type() == QEvent::StatusTip) {
        ui->txt_status_tip->setHtml(static_cast<QStatusTipEvent*>(evt)->tip());
        return true; // we've handled it, don't pass it
    }
    return QWidget::event(evt); // pass the event along the chain
}

void roleDialog::name_changed(QString text){
    if(ui->le_role_name){
        QPalette pal = ui->le_role_name->palette();
        if(GameDataReader::ptr()->get_default_roles().contains(text.trimmed())){
            ui->le_role_name->setStatusTip("This role has the same name as a default role and will override it.");
            pal.setColor(ui->le_role_name->backgroundRole(),color_override);
            m_override = true;
        }else{
            ui->le_role_name->setStatusTip("Name of the role.");
            pal.setColor(ui->le_role_name->backgroundRole(),color_default);
            m_override = false;
        }
        ui->le_role_name->setPalette(pal);
        emit event(new QStatusTipEvent(ui->le_role_name->statusTip()));
    }
}

