#include <QMessageBox>
#include <QMenu>

#include <QJSEngine>
#include <QRegularExpression>

#include "roledialog.h"
#include "contextmenuhelper.h"
#include "dwarf.h"
#include "dwarftherapist.h"
#include "gamedatareader.h"
#include "item.h"
#include "itemweaponsubtype.h"
#include "mainwindow.h"
#include "material.h"
#include "plant.h"
#include "preference.h"
#include "races.h"
#include "roleaspect.h"
#include "sortabletableitems.h"
#include "trait.h"
#include "ui_roledialog.h"
#include "utils.h"
#include "viewmanager.h"

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
        m_role->is_custom(true);
        m_role->name("");
    }

    //refresh copy combo
    ui->cmb_copy->clear();
    QList<QPair<QString, Role*> > roles = GameDataReader::ptr()->get_ordered_roles();
    QPair<QString, Role*> role_pair;
    foreach(role_pair, roles){
        if(role_pair.first != m_role->name())
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
    m_dwarf = 0;
    selection_changed();
    //refresh name background color
    name_changed(ui->le_role_name->text());

    //if there's a script, enlarge the window
    QList<int> sizes;
    if(!m_role->script().isEmpty()){
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
    ui->le_role_name->setText(m_role->name());
    ui->te_script->setPlainText(m_role->script());
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
    RoleAspect *a = new RoleAspect(this);
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
    m_role->is_custom(true);

    save_role(m_role);

    //if we're updating/adding a role to replace a default remove the default role first
    if(m_override)
        GameDataReader::ptr()->get_roles().remove(m_role->name());

    m_role->name(new_name);
    m_role->create_role_details(*DT->user_settings());
    GameDataReader::ptr()->get_roles().insert(new_name,m_role);

    //exit
    this->accept();
}

void roleDialog::save_role(Role *r){
    //save any script
    r->script(ui->te_script->toPlainText());

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
        RoleAspect *a = new RoleAspect(this);
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
    if(m_role && !m_role->name().trimmed().isEmpty())
        GameDataReader::ptr()->get_roles().insert(m_role->name(),m_role);
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

    QList<QPair<ATTRIBUTES_TYPE, QString> > atts = gdr->get_ordered_attribute_names();
    QPair<ATTRIBUTES_TYPE, QString> att_pair;
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
    QPair<int, QPair<QString,QString> > skill_pair;
    foreach(skill_pair, gdr->get_ordered_skills()) {
        if(!m_role->skills.contains((QString)skill_pair.first)){
            QMenu *menu_to_use = cmh.find_menu(m,skill_pair.second.first);
            a = menu_to_use->addAction(skill_pair.second.first, this, SLOT(add_skill()));
            a->setData(skill_pair.first);
            a->setToolTip(tr("Include %1 (ID %2) as an aspect for this role.)").arg(skill_pair.second.first).arg(skill_pair.first));
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
            QMenu *menu_to_use = cmh.find_menu(m,t->get_name());
            a = menu_to_use->addAction(t->get_name(), this, SLOT(add_trait()));
            a->setData(trait_pair.first);
            a->setToolTip(tr("Include %1 (ID %2) as an aspect for this role.").arg(t->get_name()).arg(trait_pair.first));
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
            name = m_role->name();
        m_role->name(name);

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

struct mat_flags_t {
    std::vector<MATERIAL_FLAGS> flags;
    operator const std::vector<MATERIAL_FLAGS> &() const { return flags; }

    template<typename... Flags>
    inline bool check (Material *m, MATERIAL_FLAGS flag, Flags... others) {
        if (m->flags().has_flag((int)flag) && check(m, others...)) {
            flags.push_back (flag);
            return true;
        }
        return false;
    }

    inline bool check (Material *) { return true; }
};

void roleDialog::load_material_prefs(QVector<Material*> mats){
    auto add_pref = [this] (QTreeWidgetItem *parent,
                            Material *m,
                            const std::vector<MATERIAL_FLAGS> &flags,
                            MATERIAL_STATES mat_state = SOLID) {
        QString name = m->get_material_name(mat_state).trimmed ();
        Preference *p = new Preference(LIKE_MATERIAL,name,this);
        for (auto flag: flags)
            p->add_flag((int)flag);
        p->set_exact(true);
        add_pref_to_tree(parent,p);
    };

    foreach(Material *m, mats){
        if(m->is_generated())
            continue;

        mat_flags_t flags;

        //check specific flags
        if (flags.check(m, IS_GEM))
            add_pref(m_gems, m, flags);
        else if (flags.check(m, IS_GLASS) || flags.check(m, CRYSTAL_GLASSABLE))
            add_pref(m_glass, m, flags);
        else if (flags.check(m, IS_METAL))
            add_pref(m_metals, m, flags);
        else if(flags.check(m, IS_WOOD))
            add_pref(m_wood, m, flags);
        else if(flags.check(m, IS_STONE)) {
            if (flags.check(m, ITEMS_QUERN, NO_STONE_STOCKPILE))
                add_pref(m_glazes_wares, m, flags);
            else
                add_pref(m_stone, m, flags);
        }
        else if(flags.check(m, THREAD_PLANT)) {
            add_pref(m_fabrics, m, flags, SOLID);
            add_pref(m_papers, m, flags, PRESSED);
        }
        else if(flags.check(m, IS_DYE))
            add_pref(m_fabrics, m, flags, POWDER);
        else if(flags.check(m, ITEMS_DELICATE))
            add_pref(m_general_material, m, flags); //check for coral and amber
        else { //check the include mats flags
            for (auto f: mats_include) {
                if (m->flags().has_flag((int)f)) {
                    add_pref(m_general_material, m, {f});
                }
            }
        }
    }
}

void roleDialog::load_plant_prefs(QVector<Plant*> plants){
    QString name;
    foreach(Plant *p, plants){
        name = capitalize(p->name_plural());
        Preference *plant_pref = new Preference(LIKE_PLANT,name,this);
        plant_pref->set_pref_flags(p);

        if(p->flags().has_flag(P_SAPLING) || p->flags().has_flag(P_TREE)){
            add_pref_to_tree(m_trees,plant_pref);
        }else{
            add_pref_to_tree(m_plants,plant_pref);

            if(p->flags().has_flag(P_DRINK)){
                add_pref_to_tree(m_plants_alcohol,plant_pref);
            }
            if(p->flags().has_flag(P_CROP)){
                add_pref_to_tree(m_plants_crops,plant_pref);
                if(p->flags().has_flag(P_SEED)){
                    add_pref_to_tree(m_plants_crops_plantable,plant_pref);
                }
            }
        }

        if(p->flags().has_flag(P_MILL)){
            add_pref_to_tree(m_plants_mill,plant_pref);
        }
        if(p->flags().has_flag(P_HAS_EXTRACTS)){
            add_pref_to_tree(m_plants_extract,plant_pref);
        }

        load_material_prefs(p->get_plant_materials());
    }
}

void roleDialog::load_items(){
    QList<int> flags;
    flags << IS_CLOTHING;
    add_general_node(tr("Clothing (Any)"),LIKE_ITEM,flags,m_general_equip);
    flags << IS_ARMOR;
    add_general_node(tr("Armor (Any)"),LIKE_ITEM,flags,m_general_equip);

    flags << IS_TRADE_GOOD;
    add_general_node(tr("Trade Goods"),LIKE_ITEM,flags,m_general_item);

    //setup a list of item exclusions. these are item types that are not found in item preferences
    //weapons are also ignored because we'll handle them manually to split them into ranged and melee categories
    item_ignore << BAR << SMALLGEM << BLOCKS << ROUGH << BOULDER << WOOD << CORPSE << CORPSEPIECE << REMAINS
                << FISH_RAW << VERMIN << IS_PET << SKIN_TANNED << THREAD << CLOTH << BALLISTAARROWHEAD
                << TRAPPARTS << FOOD << GLOB << ROCK << PIPE_SECTION << ORTHOPEDIC_CAST << EGG << BOOK << SHEET << WEAPON;

    //additionally ignore food types, since they can only be a preference as a consumable
    item_ignore << MEAT << FISH << CHEESE << PLANT << DRINK << POWDER_MISC << LEAVES_FRUIT << LIQUID_MISC << SEEDS;

    QTreeWidgetItem *item_parent;
    QTreeWidgetItem *clothing_parent;

    QHash<ITEM_TYPE, QVector<VIRTADDR> > item_list = m_df->get_all_item_defs();
    int count;

    QStringList added_subtypes;

    PREF_TYPES pType = LIKE_ITEM;
    for(int idx=0; idx < NUM_OF_ITEM_TYPES; idx++){
        ITEM_TYPE itype = static_cast<ITEM_TYPE>(idx);

        if(!item_ignore.contains(itype)){
            count = item_list.value(itype).count();
            QString name = Item::get_item_name_plural(itype);

            pType = LIKE_ITEM;
            bool is_armor_type = Item::is_armor_type(itype,false);

            //add all item types as a group to the general categories
            Preference *p = new Preference(pType, itype,this);
            p->set_name(name);
            if(Item::is_trade_good(itype)){
                p->add_flag(IS_TRADE_GOOD);
                add_pref_to_tree(m_general_trade_good,p);
            }else if(is_armor_type || Item::is_supplies(itype) ||
                     Item::is_melee_equipment(itype) || Item::is_ranged_equipment(itype)){
                add_pref_to_tree(m_general_equip,p);
            }else{
                add_pref_to_tree(m_general_item,p);
            }

            if(is_armor_type){
                Preference *pc = new Preference(pType,Item::get_item_clothing_name(itype),this);
                pc->set_item_type(itype);
                pc->add_flag(IS_CLOTHING);
                add_pref_to_tree(m_general_equip, pc);
            }

            //specific items
            if(count > 1){
                added_subtypes.clear();
                //create a node for the specific item type
                item_parent = init_parent_node(name);
                QString item_name;
                if(is_armor_type){
                    clothing_parent = init_parent_node(Item::get_item_clothing_name(itype));
                }else{
                    clothing_parent = 0;
                }
                for(int sub_id = 0; sub_id < count; sub_id++){
                    Preference *pi = new Preference(pType,itype,this);
                    pi->set_exact(true);
                    //check for clothing
                    if(is_armor_type){
                        ItemSubtype *stype = m_df->get_item_subtype(itype,sub_id);
                        item_name = stype->name_plural();
                        if(added_subtypes.contains(item_name))
                            continue;
                        pi->set_name(item_name);
                        pi->set_pref_flags(stype);

                        if(stype->flags().has_flag(IS_ARMOR)){
                            add_pref_to_tree(item_parent,pi);
                        }
                        if(stype->flags().has_flag(IS_CLOTHING)){
                            if(clothing_parent){
                                add_pref_to_tree(clothing_parent,pi);
                            }else{
                                LOGE << "Missing parent node for clothing item type" << itype << "subtype" << sub_id;
                            }
                        }
                        added_subtypes.append(item_name);
                    }else{
                        pi->set_name(capitalize(m_df->get_preference_item_name(itype,sub_id)));
                        add_pref_to_tree(item_parent,pi);
                    }
                }
            }
        }
    }
}

void roleDialog::load_creatures(){
    auto add_general_creature_node = [this] (const QString &suffix,
                                             const std::vector<int> &flags,
                                             QTreeWidgetItem *&parent_node) {
        QString title = tr("Creatures (%1)").arg(suffix);
        parent_node = init_parent_node(title);

        Preference *p = new Preference(LIKE_CREATURE, NONE, this);
        for (int f: flags){
            p->add_flag(f);
        }
        p->set_name(title);
        add_pref_to_tree(m_general_creature, p);
    };

    //add general categories for groups of creatures
    add_general_creature_node(tr("Hateable"), {HATEABLE}, m_hateable);
    add_general_creature_node(tr("Fish Extracts"), {VERMIN_FISH}, m_extracts_fish);
    add_general_creature_node(tr("Trainable"), {TRAINABLE_HUNTING,TRAINABLE_WAR}, m_trainable);
    add_general_creature_node(tr("Milkable"), {MILKABLE}, m_milkable);
    add_general_creature_node(tr("Fishable"), {FISHABLE}, m_fishable);
    add_general_creature_node(tr("Shearable"), {SHEARABLE}, m_shearable);
    add_general_creature_node(tr("Extracts"), {HAS_EXTRACTS}, m_extracts);
    add_general_creature_node(tr("Butcherable"), {BUTCHERABLE}, m_butcher);
    add_general_creature_node(tr("Domestic"), {DOMESTIC}, m_domestic);

    foreach(Race *r, m_df->get_races()){
        if(r->flags().has_flag(WAGON))
            continue;

        Preference *p = new Preference(LIKE_CREATURE, capitalize(r->plural_name()),this);
        p->set_pref_flags(r);

        if(r->caste_flag(DOMESTIC)){
            add_pref_to_tree(m_domestic,p);
        }

        if(r->flags().has_flag(HATEABLE)){
            add_pref_to_tree(m_hateable,p);
        }else{
            add_pref_to_tree(m_creatures,p);
        }
        if(r->caste_flag(FISHABLE)){
            add_pref_to_tree(m_fishable,p);
        }
        if(r->caste_flag(TRAINABLE)){
            add_pref_to_tree(m_trainable,p);
        }
        if(r->caste_flag(MILKABLE)){
            add_pref_to_tree(m_milkable,p);
        }
        if(r->caste_flag(SHEARABLE)){
            add_pref_to_tree(m_shearable,p);
        }
        if(r->caste_flag(BUTCHERABLE)){
            add_pref_to_tree(m_butcher,p);
        }
        if(r->caste_flag(HAS_EXTRACTS)){
            if(r->caste_flag(FISHABLE)){
                add_pref_to_tree(m_extracts_fish,p);
            }else{
                add_pref_to_tree(m_extracts,p);
            }
        }

        for (Material *m: r->get_creature_materials().values()) {
            for (auto t: {std::make_tuple(LEATHER, m_leathers),
                          std::make_tuple(YARN, m_fabrics),
                          std::make_tuple(SILK, m_fabrics)}) {
                auto flag = std::get<0>(t);
                auto parent = std::get<1>(t);
                if (m->flags().has_flag(flag)) {
                    Preference *p = new Preference(LIKE_MATERIAL, m->get_material_name(SOLID), this);
                    p->add_flag(flag);
                    p->set_exact(true);
                    add_pref_to_tree(parent, p);
                }
            }
        }
    }
}

void roleDialog::load_weapons(){
    Preference *p;
    //add parent categories
    QTreeWidgetItem *melee = init_parent_node(tr("Weapons (Melee)"));
    QTreeWidgetItem *ranged = init_parent_node(tr("Weapons (Ranged)"));

    //add category to general items
    QList<int> flags;
    flags << ITEMS_WEAPON_RANGED;
    add_general_node(ranged->text(0),LIKE_ITEM,flags,m_general_equip,WEAPON);
    flags << ITEMS_WEAPON;
    add_general_node(melee->text(0),LIKE_ITEM,flags,m_general_equip,WEAPON);

    foreach(ItemSubtype *i, m_df->get_item_subtypes(WEAPON)){
        ItemWeaponSubtype *w = qobject_cast<ItemWeaponSubtype*>(i);
        p = new Preference(LIKE_ITEM,w->name_plural(),this); //unfortunately a crescent halberd != halberd
        p->set_pref_flags(w);
        if(w->flags().has_flag(ITEMS_WEAPON_RANGED)){
            add_pref_to_tree(ranged,p);
        }else{
            add_pref_to_tree(melee,p);
        }
    }
}

void roleDialog::build_pref_tree(){
    ui->treePrefs->setSortingEnabled(false);
    //setup general categories
    m_general_item = init_parent_node(tr("~General Items"));
    m_general_equip = init_parent_node(tr("~General Equipment"));
    m_general_material = init_parent_node(tr("~General Materials"));
    m_general_creature = init_parent_node(tr("~General Creatures"));
    m_general_trade_good = init_parent_node(tr("~General Trade Goods"));
    m_general_other = init_parent_node(tr("~General Other"));
    m_general_plant_tree = init_parent_node(tr("~General Plants & Trees"));

    //setup other groups
    m_gems = init_parent_node(tr("Gems"));
    m_glass = init_parent_node(tr("Glass & Crystals"));
    m_metals = init_parent_node(tr("Metals"));
    m_stone = init_parent_node(tr("Stone & Ores"));
    m_wood = init_parent_node(tr("Wood"));
    m_glazes_wares = init_parent_node(tr("Glazes & Stoneware"));
    m_plants = init_parent_node(tr("Plants"));
    m_plants_alcohol = init_parent_node(tr("Plants (Alcohol)"));
    m_plants_crops = init_parent_node(tr("Plants (Crops)"));
    m_plants_crops_plantable = init_parent_node(tr("Plants (Crops Plantable)"));
    m_plants_mill = init_parent_node(tr("Plants (Mill)"));
    m_plants_extract = init_parent_node(tr("Plants (Extracts)"));
    m_trees = init_parent_node(tr("Trees"));
    m_fabrics = init_parent_node(tr("Fabrics & Dyes"));
    m_papers = init_parent_node(tr("Papers"));
    m_leathers = init_parent_node(tr("Leathers"));
    m_creatures = init_parent_node(tr("Creatures (Other)"));


    //also add trees to general category. don't need a flag as trees is a pref category
    Preference *p_trees = new Preference(LIKE_TREE,NONE,this);
    //p_trees->add_flag(77); //is tree flag
    p_trees->set_name(tr("Trees"));
    add_pref_to_tree(m_general_plant_tree, p_trees);

    //any material types that we want to add to the general category section go here
    mats_include << BONE << TOOTH << HORN << PEARL << SHELL << LEATHER << SILK << IS_GLASS
                 << IS_WOOD << THREAD_PLANT << YARN;

    QList<int> flags;
    foreach(MATERIAL_FLAGS f, mats_include){
        flags << f;
        add_general_node(Material::get_material_flag_desc(f),LIKE_MATERIAL,flags,m_general_material);
    }

    //general category for plants used for alcohol
    flags << P_DRINK;
    add_general_node(tr("Plants (Alcohol)"),LIKE_PLANT,flags,m_general_plant_tree);

    //general category for crops plant or gather
    flags << P_CROP;
    add_general_node(tr("Plants (Crops)"),LIKE_PLANT,flags,m_general_plant_tree);

    //general category for plantable crops
    flags << P_CROP << P_SEED;
    add_general_node(tr("Plants (Crops Plantable)"),LIKE_PLANT,flags,m_general_plant_tree);

    //general category for millable plants
    flags << P_MILL;
    add_general_node(tr("Plants (Millable)"),LIKE_PLANT,flags,m_general_plant_tree);

    //general category for plants used for processing/threshing
    flags << P_HAS_EXTRACTS;
    add_general_node(tr("Plants (Extracts)"),LIKE_PLANT,flags,m_general_plant_tree);

    //special custom preference for outdoors
    flags << 999;
    add_general_node(tr("Outdoors"),LIKE_OUTDOORS,flags,m_general_other);

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

void roleDialog::add_general_node(const QString title, PREF_TYPES ptype, QList<int> &flags, QTreeWidgetItem *parent, ITEM_TYPE itype){
    Preference *p = new Preference(ptype, itype,this);
    p->set_name(title);
    foreach(int flag, flags){
        p->add_flag(flag);
    }
    add_pref_to_tree(parent, p);
    flags.clear();
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
    QRegularExpression filter(val, QRegularExpression::CaseInsensitiveOption);
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
        QJSValue ret = m_engine.evaluate(script);
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
            float rating = m_dwarf->get_raw_role_rating(m_role->name());
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
