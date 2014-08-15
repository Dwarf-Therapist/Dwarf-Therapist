#include <QMessageBox>
#include <QJSEngine>
#include "roledialog.h"
#include "utils.h"
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
#include "itemweaponsubtype.h"
#include "roleaspect.h"
#include "preference.h"
#include "item.h"
#include "material.h"
#include "plant.h"
#include "races.h"
#include "dwarf.h"
#include "sortabletableitems.h"

roleDialog::~roleDialog()
{
    m_df = 0;
    m_dwarf = 0;
    m_role = 0;

    qDeleteAll(m_pref_list);
    m_pref_list.clear();

    ui->tw_attributes->clear();
    ui->tw_prefs->clear();
    ui->tw_skills->clear();
    ui->tw_traits->clear();
    ui->treePrefs->clear();
    delete ui;
}

roleDialog::roleDialog(DFInstance *dfi, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::roleDialog)
{
    ui->setupUi(this);
    //this->setAttribute(Qt::WA_DeleteOnClose,true);
    m_df = dfi;
    m_role = 0;
    m_dwarf = 0;

    //attributes table
    ui->tw_attributes->setEditTriggers(QTableWidget::AllEditTriggers);
    ui->tw_attributes->verticalHeader()->hide();
    ui->tw_attributes->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tw_attributes->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_attributes->setHorizontalHeaderLabels(QStringList() << "Attribute" << "Weight");
    //skills table
    ui->tw_skills->setEditTriggers(QTableWidget::AllEditTriggers);
    ui->tw_skills->verticalHeader()->hide();
    ui->tw_skills->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tw_skills->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_skills->setHorizontalHeaderLabels(QStringList() << "Skill" << "Weight");
    //traits table
    ui->tw_traits->setEditTriggers(QTableWidget::AllEditTriggers);
    ui->tw_traits->verticalHeader()->hide();
    ui->tw_traits->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tw_traits->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_traits->setHorizontalHeaderLabels(QStringList() << "Trait" << "Weight");
    //preference table
    ui->tw_prefs->setColumnCount(4);
    ui->tw_prefs->setEditTriggers(QTableWidget::AllEditTriggers);
    ui->tw_prefs->verticalHeader()->hide();
    ui->tw_prefs->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tw_prefs->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_prefs->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tw_prefs->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->tw_prefs->setHorizontalHeaderLabels(QStringList() << "Preference" << "Weight" << "Category" << "Item");

    connect(ui->btn_cancel, SIGNAL(clicked()), SLOT(close_pressed()));
    connect(ui->btn_save, SIGNAL(clicked()), SLOT(save_pressed()));
    connect(ui->btn_copy, SIGNAL(clicked()), SLOT(copy_pressed()));

    connect(ui->tw_attributes, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_attribute_context_menu(const QPoint &)));
    connect(ui->tw_traits, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_trait_context_menu(const QPoint &)));
    connect(ui->tw_skills, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_skill_context_menu(const QPoint &)));    
    connect(ui->tw_prefs, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_prefs_context_menu(const QPoint &)));

    connect(ui->le_role_name, SIGNAL(textChanged(QString)), SLOT(name_changed(QString)));

    connect(ui->le_search, SIGNAL(textChanged(QString)), this, SLOT(search_prefs(QString)));
    connect(ui->btnCloseSearch, SIGNAL(clicked()),this,SLOT(clear_search()));

    connect(ui->btnRefreshRatings, SIGNAL(clicked()), this, SLOT(selection_changed()));

    color_override = QColor::fromRgb(153,102,34,255);
    color_default = QColor::fromRgb(255,255,255,255);

    //main splitter between aspects and script
    ui->splitter_main->setStretchFactor(0,50);
    ui->splitter_main->setStretchFactor(1,1);
    decorate_splitter(ui->splitter_main);

    //setup the splitter options, and decorate the handle
    //main splitter between aspects and prefs
    ui->splitter_aspects_main->setStretchFactor(0,1);
    ui->splitter_aspects_main->setStretchFactor(1,1);
    decorate_splitter(ui->splitter_aspects_main);

    //splitters between aspects on left side
    ui->splitter_aspects_left->setStretchFactor(0,3);
    ui->splitter_aspects_left->setStretchFactor(1,2);
    ui->splitter_aspects_left->setStretchFactor(2,2);
    decorate_splitter(ui->splitter_aspects_left);

    //preference splitter
    ui->splitter_prefs->setStretchFactor(0,2);
    ui->splitter_prefs->setStretchFactor(1,3);
    decorate_splitter(ui->splitter_prefs);

    build_pref_tree();
}

void roleDialog::load_role(QString role_name){
    m_override = false;

    m_role = GameDataReader::ptr()->get_roles().take(role_name);
    if(!m_role){
        m_role = new Role();
        m_role->is_custom = true;
        m_role->name = "";
    }

    //refresh copy combo
    ui->cmb_copy->clear();
    QList<QPair<QString, Role*> > roles = GameDataReader::ptr()->get_ordered_roles();
    QPair<QString, Role*> role_pair;
    foreach(role_pair, roles){
        if(role_pair.first != m_role->name)
            ui->cmb_copy->addItem(role_pair.first);
    }

    //clear stuffs
    ui->le_role_name->clear();
    ui->te_script->clear();
    clear_table(*ui->tw_attributes);
    clear_table(*ui->tw_prefs);
    clear_table(*ui->tw_skills);
    clear_table(*ui->tw_traits);
    clear_search();

    //load data
    load_role_data();

    //refresh status tip info for weights
    QString stat = tr("This weight is the importance of %1 relative to %2, %3 and %4. A higher weight gives more value. The current default is %5.");
    ui->dsb_attributes_weight->setStatusTip(stat.arg("attributes").arg("traits").arg("skills").arg("preferences")
                                            .arg(DT->user_settings()->value("options/default_attributes_weight",1.0).toString()));
    ui->dsb_skills_weight->setStatusTip(stat.arg("skills").arg("attributes").arg("traits").arg("preferences")
                                            .arg(DT->user_settings()->value("options/default_skills_weight",1.0).toString()));
    ui->dsb_traits_weight->setStatusTip(stat.arg("traits").arg("attributes").arg("skills").arg("preferences")
                                            .arg(DT->user_settings()->value("options/default_traits_weight",1.0).toString()));
    ui->dsb_prefs_weight->setStatusTip(stat.arg("preferences").arg("attributes").arg("skills").arg("traits")
                                            .arg(DT->user_settings()->value("options/default_prefs_weight",1.0).toString()));

    //refresh example
    selection_changed();
    //refresh name background color
    name_changed(ui->le_role_name->text());

    //if there's a script, enlarge the window
    QList<int> sizes;
    if(!m_role->script.isEmpty()){
        sizes.append(50);
        sizes.append(this->height() - 50);
    }else{
        sizes.append(this->height() - 50);
        sizes.append(50);
    }
    ui->splitter_main->setSizes(sizes);
}

void roleDialog::decorate_splitter(QSplitter *s){
    QSplitterHandle *h = s->handle(1);

    QFrame *line = new QFrame(h);
    line->setFrameShadow(QFrame::Sunken);

    if(s->orientation() == Qt::Vertical){
        QHBoxLayout *layout = new QHBoxLayout(h);
        layout->setSpacing(0);
        layout->setMargin(0);
        layout->addWidget(line);

        line->setFrameShape(QFrame::HLine);
    }else{
        QVBoxLayout *layout = new QVBoxLayout(h);
        layout->setSpacing(0);
        layout->setMargin(0);
        layout->addWidget(line);

        line->setFrameShape(QFrame::VLine);
    }

    QPalette pal = h->palette();
    pal.setColor(h->backgroundRole(), QColor(139,137,137));
    h->setPalette(pal);
}


void roleDialog::load_role_data(){
    ui->le_role_name->setText(m_role->name);
    ui->te_script->setPlainText(m_role->script);
    //global weights
    ui->dsb_attributes_weight->setValue(m_role->attributes_weight.weight);
    ui->dsb_traits_weight->setValue(m_role->traits_weight.weight);
    ui->dsb_skills_weight->setValue(m_role->skills_weight.weight);
    ui->dsb_prefs_weight->setValue(m_role->prefs_weight.weight);

    //load aspects
    load_aspects_data(*ui->tw_attributes, m_role->attributes);
    load_aspects_data(*ui->tw_traits, m_role->traits);
    load_aspects_data(*ui->tw_skills, m_role->skills);

    foreach(Preference *p, m_role->prefs){
        insert_pref_row(p);
    }
}

void roleDialog::load_aspects_data(QTableWidget &table, QHash<QString, RoleAspect*> aspects){
    RoleAspect *a;
    table.setSortingEnabled(false);
    foreach(QString key, aspects.uniqueKeys()){
        a = aspects.value(key);
        insert_row(table,a,key);
    }
    table.setSortingEnabled(true);
    table.sortItems(0,Qt::AscendingOrder);
}

void roleDialog::insert_row(QTableWidget &table, RoleAspect *a, QString key){
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
    dbs->setValue(a->is_neg ? 0-a->weight : a->weight);
    table.setCellWidget(row,1,dbs);
    sortableTableWidgetItem* sItem = new sortableTableWidgetItem;
    table.setItem(row,1,sItem);

    table.setSortingEnabled(true);
}

void roleDialog::insert_pref_row(Preference *p){
    ui->tw_prefs->setSortingEnabled(false);
    int row = ui->tw_prefs->rowCount();
    ui->tw_prefs->insertRow(row);
    ui->tw_prefs->setRowHeight(row,18);

    QTableWidgetItem *name = new QTableWidgetItem();
    name->setData(0,p->get_name());
    name->setData(Qt::UserRole, vPtr<Preference>::asQVariant(p));
    name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->tw_prefs->setItem(row,0,name);

    QDoubleSpinBox *dbs = new QDoubleSpinBox();
    dbs->setMinimum(-100);
    dbs->setMaximum(100);
    dbs->setSingleStep(0.25);
    dbs->setValue(p->pref_aspect->is_neg ? 0-p->pref_aspect->weight : p->pref_aspect->weight);
    ui->tw_prefs->setCellWidget(row,1,dbs);
    sortableTableWidgetItem* sItem = new sortableTableWidgetItem;
    ui->tw_prefs->setItem(row,1,sItem);

    QTableWidgetItem *ptype = new QTableWidgetItem();
    ptype->setData(0, Preference::get_pref_desc(p->get_pref_category()));
    ptype->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->tw_prefs->setItem(row,2,ptype);

    QTableWidgetItem *itype = new QTableWidgetItem();
    itype->setData(0, Item::get_item_name_plural(p->get_item_type()));
    itype->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->tw_prefs->setItem(row,3,itype);

    ui->tw_prefs->setSortingEnabled(true);
}

void roleDialog::add_aspect(QString id, QTableWidget &table, QHash<QString, RoleAspect *> &list){
    RoleAspect *a = new RoleAspect();
    a->is_neg = false;
    a->weight = 1.0;
    list.insert(id.toLower(), a);
    insert_row(table,a,id.toLower());
}

void roleDialog::save_pressed(){
    QString new_name = ui->le_role_name->text().trimmed();
    if(new_name.trimmed().isEmpty()){
        QMessageBox::critical(this,"Invalid Role Name","Role names cannot be blank.");
        return;
    }
    //update the role
    m_role->is_custom = true;

    save_role(m_role);

    //if we're updating/adding a role to replace a default remove the default role first
    if(m_override)
        GameDataReader::ptr()->get_roles().remove(m_role->name);

    m_role->name = new_name;
    m_role->create_role_details(*DT->user_settings());
    GameDataReader::ptr()->get_roles().insert(new_name,m_role);

    //exit
    this->accept();
}

void roleDialog::save_role(Role *r){
    //save any script
    r->script = ui->te_script->toPlainText();

    //attributes
    r->attributes_weight.weight = ui->dsb_attributes_weight->value();
    save_aspects(*ui->tw_attributes, r->attributes);

    //skills
    r->skills_weight.weight = ui->dsb_skills_weight->value();
    save_aspects(*ui->tw_skills, r->skills);

    //traits
    r->traits_weight.weight = ui->dsb_traits_weight->value();
    save_aspects(*ui->tw_traits,r->traits);

    //preferences
    r->prefs_weight.weight = ui->dsb_prefs_weight->value();
    save_prefs(r);
}

void roleDialog::save_aspects(QTableWidget &table, QHash<QString, RoleAspect*> &list){
    for(int i= 0; i<table.rowCount(); i++){
        RoleAspect *a = new RoleAspect();
        QString key = table.item(i,0)->data(Qt::UserRole).toString();
        float weight = static_cast<QDoubleSpinBox*>(table.cellWidget(i,1))->value();
        a->weight = fabs(weight);
        a->is_neg = weight < 0 ? true : false;
        list.insert(key,a);
    }
}

void roleDialog::save_prefs(Role *r){    
    for(int i= 0; i<ui->tw_prefs->rowCount(); i++){
        float weight = static_cast<QDoubleSpinBox*>(ui->tw_prefs->cellWidget(i,1))->value();
        Preference *p = vPtr<Preference>::asPtr(ui->tw_prefs->item(i,0)->data(Qt::UserRole));
        //save the weight of the preference for the next use
        p->pref_aspect->weight = fabs(weight);
        p->pref_aspect->is_neg = weight < 0 ? true : false;

        //update the values of this preference in the role
        Preference *rp = r->has_preference(p->get_name());
        if(!rp){
            rp = new Preference(*p);
            r->prefs.append(rp);
        }
        rp->pref_aspect->weight = p->pref_aspect->weight;
        rp->pref_aspect->is_neg = p->pref_aspect->is_neg;
    }
}

void roleDialog::close_pressed(){
    m_dwarf = 0;
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
        m->addAction(QIcon(":img/minus-circle.png"), tr("Remove Selected"), this, SLOT(remove_attribute()));
        m->addSeparator();
    }
    QAction *a;
    GameDataReader *gdr = GameDataReader::ptr();

    QList<QPair<int, QString> > atts = gdr->get_ordered_attribute_names();
    QPair<int, QString> att_pair;
    foreach(att_pair, atts){
        if(!m_role->attributes.contains(att_pair.second.toLatin1().toLower())){
            a = m->addAction(tr(att_pair.second.toLatin1()), this, SLOT(add_attribute()));
            a->setData(att_pair.second.toLatin1());
            a->setToolTip(tr("Include %1 as an aspect for this role.").arg(att_pair.second));
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
        m->addAction(QIcon(":img/minus-circle.png"), tr("Remove Selected"), this, SLOT(remove_skill()));
        m->addSeparator();
    }
    QAction *a;
    GameDataReader *gdr = GameDataReader::ptr();
    ContextMenuHelper cmh;

    cmh.add_sub_menus(m,gdr->get_ordered_skills().count()/15,false);
    QPair<int, QString> skill_pair;
    foreach(skill_pair, gdr->get_ordered_skills()) {
        if(!m_role->skills.contains((QString)skill_pair.first)){
            QMenu *menu_to_use = cmh.find_menu(m,skill_pair.second);
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
        m->addAction(QIcon(":img/minus-circle.png"), tr("Remove Selected"), this, SLOT(remove_trait()));
        m->addSeparator();
    }
    QAction *a;
    GameDataReader *gdr = GameDataReader::ptr();
    ContextMenuHelper cmh;
    cmh.add_sub_menus(m,2,false);
    QList<QPair<int, Trait*> > traits = gdr->get_ordered_traits();
    QPair<int, Trait*> trait_pair;
    foreach(trait_pair, traits) {
        if(!m_role->traits.contains((QString)trait_pair.first)){            
            Trait *t = trait_pair.second;
            QMenu *menu_to_use = cmh.find_menu(m,t->name);
            a = menu_to_use->addAction(t->name, this, SLOT(add_trait()));
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

//PREFERENCE CONTEXT MENU
void roleDialog::draw_prefs_context_menu(const QPoint &p) {
    QMenu *m = new QMenu("",this);
    QModelIndex idx = ui->tw_prefs->indexAt(p);
    if (idx.isValid()) { // context on a row
        m->addAction(QIcon(":img/minus-circle.png"), tr("Remove Selected"), this, SLOT(remove_pref()));
        m->addSeparator();
    }
    m->exec(ui->tw_prefs->viewport()->mapToGlobal(p));
}

void roleDialog::remove_pref(){
    for(int i=ui->tw_prefs->rowCount()-1; i>=0; i--){
        if(ui->tw_prefs->item(i,0)->isSelected()){
            for(int j = 0; j < m_role->prefs.count(); j++){
                if(m_role->prefs.at(j)->get_name().toLower() ==
                        vPtr<Preference>::asPtr(ui->tw_prefs->item(i,0)->data(Qt::UserRole))->get_name().toLower()){
                    m_role->prefs.remove(j);
                    break;
                }
            }
            ui->tw_prefs->removeRow(i);
        }
    }
}

void roleDialog::copy_pressed(){
    int answer = QMessageBox::Yes;

    if(ui->tw_attributes->rowCount()>0 || ui->tw_traits->rowCount()>0 || ui->tw_skills->rowCount()>0 || ui->tw_prefs->rowCount()>0){
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
        clear_table(*ui->tw_prefs);

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
            pal.setColor(QPalette::Base,color_override);
            m_override = true;
        }else{
            ui->le_role_name->setStatusTip("Name of the role.");
            pal.setColor(QPalette::Base,color_default);
            m_override = false;
        }
        ui->le_role_name->setAutoFillBackground(true);
        ui->le_role_name->setPalette(pal);
        emit event(new QStatusTipEvent(ui->le_role_name->statusTip()));
    }
}

void roleDialog::load_material_prefs(QVector<Material*> mats, QString prefix_name){
    QTreeWidgetItem *parent;
    //Preference *p;
    QString name = "";
    PREF_TYPES pType;
    foreach(Material *m, mats){
        //set defaults
        if(!prefix_name.isEmpty()) //prefix indicates plant/animal mats
            name = prefix_name + " " + m->get_material_name(GENERIC);
        else
            name = m->get_material_name(SOLID).trimmed();

        pType = LIKE_MATERIAL;
        Preference *p = new Preference(pType,"",this);

        //check specific flags
        if(check_flag(m,p,IS_GEM)){
            parent = m_gems;
        }else if(check_flag(m,p,IS_GLASS) || check_flag(m,p,CRYSTAL_GLASSABLE)){
            parent = m_glass;
        }else if(check_flag(m,p,IS_METAL)){
            parent = m_metals;            
        }else if(check_flag(m,p,IS_WOOD)){
            parent = m_wood;
        }else if(check_flag(m,p,IS_STONE)){
            if(check_flag(m,p,ITEMS_QUERN) && check_flag(m,p,NO_STONE_STOCKPILE)){
                parent = m_glazes_wares;
            }else{
                parent = m_stone;
            }
        }else if(check_flag(m,p,THREAD_PLANT)){
            parent = m_fabrics;
        }else if(check_flag(m,p,EDIBLE_RAW) || check_flag(m,p,EDIBLE_COOKED)){
            if(check_flag(m,p,ALCOHOL_PLANT) || check_flag(m,p,LIQUID_MISC_PLANT)){
                parent = m_drinks;
                name = m->get_material_name(LIQUID);
                pType = LIKE_FOOD;
            }
            if(check_flag(m,p,POWDER_MISC_PLANT) || check_flag(m,p,CHEESE_PLANT)){
                parent = m_food;
                name = m->get_material_name(POWDER);
                pType = LIKE_FOOD;
            }
        }else if(check_flag(m,p,IS_DYE)){
            parent = m_fabrics;
            name = m->get_material_name(POWDER);
        }else{
            if(m->is_inorganic()){
                parent = m_inorganic_other;
                //LOGW << "INORGANIC OTHER: " << m->get_material_name(SOLID) << " " << m->flags()->output_flag_string();
            }else{
                //check for coral and amber
                if(check_flag(m,p,ITEMS_DELICATE)){
                    parent = m_general_material;
                }
//                else
//                    LOGW << "skipping material " << m->get_material_name(SOLID) << " " << m->flags()->output_flag_string();
                //parent = m_plant_other;
                //name = m->get_material_name(SOLID);
            }
        }

        //check the include mats flags
        if(!parent){
            foreach(MATERIAL_FLAGS f, mats_include){
                if(check_flag(m,p,f)){
                    if(prefix_name.isEmpty()){
                        name = m->get_material_name(SOLID);
                        parent = m_general_material;
                    }else{
                        name = prefix_name;
                        name.append(" ").append(m->get_material_name(SOLID));
                        parent = m_creature_mats;
                    }
                }
            }
        }

        if(parent && !m->flags().no_flags() && !m->flags().has_flag(BLOOD_MAP_DESCRIPTOR) && !m->flags().has_flag(ICHOR_MAP_DESCRIPTOR) &&
                !m->flags().has_flag(GOO_MAP_DESCRIPTOR) && !m->flags().has_flag(SLIME_MAP_DESCRIPTOR) &&
                !m->flags().has_flag(PUS_MAP_DESCRIPTOR) && !m->flags().has_flag(ENTERS_BLOOD)){

            p->set_category(pType);
            p->set_name(capitalize(name));            
            p->set_exact(true);
            add_pref_to_tree(parent,p);
        }
//        else{
//            if(!m->flags()->no_flags())
//                LOGW << "ignoring " << m->get_material_name(SOLID) << " due to invalid flags";
//        }

        parent = 0;
    }
}

void roleDialog::load_plant_prefs(QVector<Plant*> plants){
    QString name;
    foreach(Plant *p, plants){
        name = capitalize(p->name_plural());

        if(p->flags().has_flag(7)){
            Preference *alcohol_pref = new Preference(LIKE_PLANT,name,this);
            alcohol_pref->add_flag(7);
            alcohol_pref->set_exact(true);
            add_pref_to_tree(m_plants_alcohol,alcohol_pref);
        }
        else if(p->flags().has_flag(77) || p->flags().has_flag(78)){
            Preference *tree_pref = new Preference(LIKE_TREE,name,this);
            tree_pref->set_exact(true);
            add_pref_to_tree(m_trees,tree_pref);
        }else{
            Preference *shrub_pref = new Preference(LIKE_PLANT,name,this);
            shrub_pref->set_exact(true);
            add_pref_to_tree(m_plants,shrub_pref);
        }

        name = capitalize(p->leaf_plural());
        Preference *leaf_pref = new Preference(LIKE_FOOD,name,this);
        leaf_pref->set_exact(true);
        add_pref_to_tree(m_plants,leaf_pref);

        name = capitalize(p->seed_plural());
        Preference *seed_pref = new Preference(LIKE_FOOD,name,this);
        seed_pref->set_exact(true);
        add_pref_to_tree(m_seeds,seed_pref);

        load_material_prefs(p->get_plant_materials(),p->name());
    }
}

void roleDialog::load_items(){
    //setup a list of item exclusions. these are item types that are not found in item preferences
    //weapons are also ignored because we'll handle them manually to split them into ranged and melee categories
    item_ignore << BAR << SMALLGEM << BLOCKS << ROUGH << BOULDER << WOOD << CORPSE << CORPSEPIECE << REMAINS
              << FISH_RAW << VERMIN << IS_PET << SKIN_TANNED << THREAD << CLOTH << BALLISTAARROWHEAD
              << TRAPPARTS << FOOD << GLOB << ROCK << PIPE_SECTION << ORTHOPEDIC_CAST << EGG << BOOK << WEAPON;

    //some items should be mapped to food preference category
    item_food << MEAT << FISH << SEEDS << PLANT << POWDER_MISC << CHEESE << LEAVES_FRUIT;
    //and some to drink preference category (same as food in df for now)
    item_drink << DRINK << LIQUID_MISC;
    //add craft items to separate category to change the menu
    item_crafts << BRACELET << RING << SCEPTER << INSTRUMENT << CROWN << FIGURINE << AMULET << EARRING << TOY << GOBLET << TOTEM;

    QTreeWidgetItem *parent;
    QHash<ITEM_TYPE, QVector<VIRTADDR> > item_list = m_df->get_item_def();
    int count;

    PREF_TYPES pType = LIKE_ITEM;
    for(int i=0; i < NUM_OF_ITEM_TYPES; i++){
        ITEM_TYPE itype = static_cast<ITEM_TYPE>(i);

        if(!item_ignore.contains(itype)){
            count = item_list.value(itype).count();
            QString name = Item::get_item_name_plural(itype);

            if(item_food.contains(itype) || item_drink.contains(itype))
                pType = LIKE_FOOD;
            else
                pType = LIKE_ITEM;

            //add all item types as a group to the general categories
            Preference *p = new Preference(pType, itype,this);
            p->set_name(name);            
            if(item_crafts.contains(itype))
                add_pref_to_tree(m_general_craft,p);
            else if(item_food.contains(itype) || item_drink.contains(itype))
                add_pref_to_tree(m_general_food,p);
            else
                add_pref_to_tree(m_general_item,p);

            //specific items
            if(count > 1){
                parent = init_parent_node(name);
                for(int j = 0; j < count; j++){
                    Preference *p = new Preference(pType,itype,this);
                    p->set_name(capitalize(m_df->get_preference_item_name(itype,j)));
                    p->set_exact(true);
                    add_pref_to_tree(parent,p);
                }
            }
        }
    }
}

void roleDialog::load_creatures(){    
    //special category for vermin creatures
    m_hateable = init_parent_node("Creatures (Hateable)");
    Preference *p = new Preference(LIKE_CREATURE, NONE,this);
    p->add_flag(HATEABLE);
    p->set_name("Creatures (Hateable)");
    add_pref_to_tree(m_general_creature, p);

    //category for vermin fish (fish dissector extracts)
    p = new Preference(LIKE_CREATURE, NONE,this);
    p->add_flag(VERMIN_FISH);
    p->set_name("Creatures (Fish Extracts)");
    add_pref_to_tree(m_general_creature, p);

    //special category for trainable creatures
    m_trainable = init_parent_node("Creatures (Trainable)");
    p = new Preference(LIKE_CREATURE, NONE,this);
    p->add_flag(TRAINABLE_HUNTING);
    p->add_flag(TRAINABLE_WAR);
    p->set_name("Creatures (Trainable)");
    add_pref_to_tree(m_general_creature, p);

    //special category for milkable creatures
    m_milkable = init_parent_node("Creatures (Milkable)");
    p = new Preference(LIKE_CREATURE, NONE,this);
    p->add_flag(MILKABLE);
    p->set_name("Creatures (Milkable)");
    add_pref_to_tree(m_general_creature, p);

    //special category for extracts (honey, venom, etc)
    m_extracts = init_parent_node("Creatures (Extractable)");
    p = new Preference(LIKE_CREATURE, NONE,this);
    p->add_flag(HAS_EXTRACTS);
    p->set_name("Creatures (Extractable)");
    add_pref_to_tree(m_general_creature, p);

    foreach(Race *r, m_df->get_races()){
        p = new Preference(LIKE_CREATURE, capitalize(r->name()),this);
        p->set_exact(true);

        bool hated = false;

        if(r->flags().has_flag(HATEABLE)){
            p->add_flag(HATEABLE);
            add_pref_to_tree(m_hateable,p);
            hated = true;
        }
        if(r->flags().has_flag(VERMIN_FISH)){
            p->add_flag(VERMIN_FISH);
        }
        if(r->is_trainable()){            
            p->add_flag(TRAINABLE_HUNTING);
            p->add_flag(TRAINABLE_WAR);
            add_pref_to_tree(m_trainable,p);
        }
        if(r->is_milkable()){            
            p->add_flag(MILKABLE);
            add_pref_to_tree(m_milkable,p);
        }
        if(r->is_vermin_extractable()){            
            p->add_flag(HAS_EXTRACTS);
            add_pref_to_tree(m_extracts,p);
        }                  
         if(!hated)
             add_pref_to_tree(m_creatures,p);
    }
}

void roleDialog::load_weapons(){
    Preference *p;
    //add parent categories
    QTreeWidgetItem *melee = init_parent_node("Weapons (Melee)");
    QTreeWidgetItem *ranged = init_parent_node("Weapons (Ranged)");
    //add category to general items
    p = new Preference(LIKE_ITEM,WEAPON,this);
    p->set_name("Weapons (Ranged)");
    p->add_flag(ITEMS_WEAPON_RANGED);
    add_pref_to_tree(m_general_item, p);

    p = new Preference(LIKE_ITEM,WEAPON,this);
    p->add_flag(ITEMS_WEAPON);
    p->set_name("Weapons (Melee)");
    add_pref_to_tree(m_general_item, p);

    foreach(ItemWeaponSubtype *w, m_df->get_weapon_defs()){
        p = new Preference(LIKE_ITEM,w->name_plural(),this);
        if(w->is_ranged()){
            p->add_flag(ITEMS_WEAPON_RANGED);
            p->set_exact(true);
            add_pref_to_tree(ranged,p);
        }
        else{
            p->add_flag(ITEMS_WEAPON);
            p->set_exact(true);
            add_pref_to_tree(melee,p);
        }
    }
}

void roleDialog::build_pref_tree(){
    ui->treePrefs->setSortingEnabled(false);
    //setup general categories
    m_general_item = init_parent_node("~General Items");
    m_general_material = init_parent_node("~General Materials");
    m_general_creature = init_parent_node("~General Creatures");
    m_general_craft = init_parent_node("~General Crafts");
    m_general_food = init_parent_node("~General Food");
    m_general_other = init_parent_node("~General Other");

    //setup other groups
    m_inorganic_other = init_parent_node("Inorganic Other");
    m_gems = init_parent_node("Gems");
    m_glass = init_parent_node("Glass & Crystals");
    m_metals = init_parent_node("Metals");
    m_stone = init_parent_node("Stone & Ores");
    m_wood = init_parent_node("Wood");
    m_glazes_wares = init_parent_node("Glazes & Stoneware");
    m_seeds = init_parent_node("Seeds");
    m_plant_other = init_parent_node("Plants (Misc)");
    m_plants = init_parent_node("Plants");
    m_plants_alcohol = init_parent_node("Plants (Alcohol)");
    m_trees = init_parent_node("Trees");    
    m_food = init_parent_node("Food");
    m_drinks = init_parent_node("Drink");
    m_fabrics = init_parent_node("Fabrics & Dyes");
    m_creatures = init_parent_node("Creatures (Other)");
    //m_creature_mats = init_parent_node(LIKE_MATERIAL,NONE,"Creature Materials");

    //also add trees to general category. don't need a flag as trees is a pref category
    Preference *p_trees = new Preference(LIKE_TREE,NONE,this);
    //p_trees->add_flag(77); //is tree flag
    p_trees->set_name("Trees");
    add_pref_to_tree(m_general_item, p_trees);

    //any material types that we want to add to the general category section go here
    mats_include << BONE << TOOTH << HORN << PEARL << SHELL << LEATHER << SILK << IS_GEM << IS_GLASS
                    << IS_WOOD << IS_STONE << IS_METAL << THREAD_PLANT << YARN;

    foreach(MATERIAL_FLAGS f, mats_include){
        Preference *p = new Preference(LIKE_MATERIAL,NONE,this);
        p->set_name(Material::get_material_flag_desc(f));
        p->add_flag(f);
        add_pref_to_tree(m_general_material,p);
    }

    //special category for plants used for alcohol
    Preference *p_alc_plant = new Preference(LIKE_PLANT, NONE,this);
    p_alc_plant->set_name("Plants (Alcohol)");
    p_alc_plant->add_flag(7);
    add_pref_to_tree(m_general_food, p_alc_plant);

    //special custom preference for outdoors
    Preference *p_outdoors = new Preference(LIKE_OUTDOORS,NONE,this);
    p_outdoors->set_name("Outdoors");
    p_outdoors->add_flag(999);
    add_pref_to_tree(m_general_other, p_outdoors);

    load_material_prefs(m_df->get_inorganic_materials());
    load_material_prefs(m_df->get_base_materials());
    load_plant_prefs(m_df->get_plants());
    load_items();
    load_creatures();
    load_weapons();

    QTreeWidgetItem *child;
    foreach(QTreeWidgetItem *parent, m_pref_list.uniqueKeys()){
        QVector<Preference*> *child_nodes = m_pref_list.value(parent);
        //LOGW << "loading parent " << parent->text(0) << " child count " << child_nodes->size();
        for(int j = 0; j < child_nodes->size(); j++){
            //LOGW << "loading child " << j << " " << child_nodes->at(j);
            child = new QTreeWidgetItem(parent);
            child->setText(0, child_nodes->at(j)->get_name());
            child->setData(0, Qt::UserRole, vPtr<Preference>::asQVariant(child_nodes->at(j)));
        }

        if(parent->childCount() > 0){
            parent->setText(0, tr("%1 (%2)").arg(parent->text(0)).arg(QString::number(parent->childCount())));
        }else{
            parent->setText(0, tr("%1").arg(parent->text(0)));
        }

        ui->treePrefs->addTopLevelItem(parent);
    }

    ui->treePrefs->setSortingEnabled(true);
    ui->treePrefs->sortItems(0,Qt::AscendingOrder);
    ui->treePrefs->collapseAll();

    connect(ui->treePrefs, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(item_double_clicked(QTreeWidgetItem*,int)));
}

QTreeWidgetItem* roleDialog::init_parent_node(QString title){
    QTreeWidgetItem *node = new QTreeWidgetItem;
    node->setData(0, Qt::UserRole, title);
    node->setText(0, title);
    m_pref_list.insert(node,new QVector<Preference*>());
    return node;
}

void roleDialog::add_pref_to_tree(QTreeWidgetItem *parent, Preference *p){
    if(!p->get_name().isEmpty()){
        //general items are never exact matches
//        if(parent != m_general_item)
//            p->set_exact(true);

        //set a default weight
        p->pref_aspect->weight = 0.5f;

        if(parent && m_pref_list.contains(parent)){
            if(m_pref_list.value(parent)->count() > 0){
                for(int i=0; i < m_pref_list.value(parent)->count();i++){
                    if(m_pref_list.value(parent)->at(i)->get_name()==p->get_name())
                        return;
                }
            }
            m_pref_list.value(parent)->append(p);
        }
    }
}

bool roleDialog::check_flag(Material *m, Preference *p, MATERIAL_FLAGS flag){
    if(m->flags().has_flag((int)flag)){
        p->add_flag((int)flag);
        return true;
    }else{
        return false;
    }
}

void roleDialog::item_double_clicked(QTreeWidgetItem *item, int col){
    if(item->childCount() <= 0){
        Preference *p = vPtr<Preference>::asPtr(item->data(col,Qt::UserRole));
        if(p && !m_role->has_preference(p->get_name())){
            insert_pref_row(p);
        }
    }
}

void roleDialog::search_prefs(QString val){
    val = "(" + val.replace(" ", "|") + ")";
    QRegExp filter = QRegExp(val,Qt::CaseInsensitive, QRegExp::RegExp);
    int hidden;
    for(int i = 0; i < ui->treePrefs->topLevelItemCount(); i++){
        hidden = 0;
        int count;
        for(count = 0; count < ui->treePrefs->topLevelItem(i)->childCount(); count++){
            if(!ui->treePrefs->topLevelItem(i)->child(count)->text(0).contains(filter)){
                ui->treePrefs->topLevelItem(i)->child(count)->setHidden(true);
                hidden++;
            }else{
                ui->treePrefs->topLevelItem(i)->child(count)->setHidden(false);
            }
        }
        if(hidden == count){
            ui->treePrefs->topLevelItem(i)->setHidden(true);
        }else{
            ui->treePrefs->topLevelItem(i)->setHidden(false);
            QString title = ui->treePrefs->topLevelItem(i)->data(0,Qt::UserRole).toString() + QString(" (%1)").arg(QString::number(count-hidden));
            ui->treePrefs->topLevelItem(i)->setText(0,title);
        }
    }
}

void roleDialog::clear_search(){
    ui->le_search->setText("");
    search_prefs("");
    ui->treePrefs->collapseAll();
}

void roleDialog::calc_new_role(){
    //if using a script, check the syntax
    QString script = ui->te_script->toPlainText().trimmed();
    if(!script.isEmpty()){
        QJSEngine m_engine;
        QJSValue d_obj = m_engine.newQObject(m_dwarf);
        m_engine.globalObject().setProperty("d", d_obj);
        QJSValue ret = m_engine.evaluate(m_role->script);
        if(!ret.isNumber()){
            QString err_msg;
            if(ret.isError()) {
                err_msg = tr("<font color=red>%1: %2<br/>%3</font>")
                                 .arg(ret.property("name").toString())
                                 .arg(ret.property("message").toString())
                                 .arg(ret.property("stack").toString().replace("\n", "<br/>"));
            }else{
                m_engine.globalObject().setProperty("__internal_role_return_value_check", ret);
                err_msg = tr("<font color=red>Script returned %1 instead of number</font>")
                                 .arg(m_engine.evaluate(QString("typeof __internal_role_return_value_check")).toString());
                m_engine.globalObject().deleteProperty("__internal_role_return_value_check");
            }
            ui->te_script->setStatusTip(err_msg);
            ui->txt_status_tip->setText(err_msg);
            return;
        }else{
            ui->te_script->setStatusTip(ui->te_script->whatsThis());
        }
    }else{
        ui->te_script->setStatusTip(ui->te_script->whatsThis());
    }

    Role *test = new Role(*m_role);
    save_role(test);
    ui->lbl_new->setText("New Raw Rating: " + QString::number(m_dwarf->calc_role_rating(test),'g',4) + "%");
}

void roleDialog::selection_changed(){
    if(m_role){
        QList<Dwarf*> dwarfs = DT->get_main_window()->get_view_manager()->get_selected_dwarfs();
        if(dwarfs.count() > 0)
            m_dwarf = dwarfs.at(0);
        if(m_dwarf){
            ui->lbl_name->setText(m_dwarf->nice_name());
            float rating = m_dwarf->get_role_rating(m_role->name, true);
            ui->lbl_current->setText("Current Raw Rating: " + QString::number(rating,'g',4) + "%");
            //ui->lbl_new->setText("New Raw Rating: " + QString::number(rating,'g',2) + "%");
            calc_new_role();
        }else{
            m_dwarf = 0;
            ui->lbl_name->setText("Select a dwarf to view ratings.");
            ui->lbl_current->setText("");
            ui->lbl_new->setText("");
        }
    }
}
