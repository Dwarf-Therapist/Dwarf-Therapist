#include <QMessageBox>
#include <QMenu>

#include <QJSEngine>

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
#include "rolepreference.h"
#include "rolepreferencemodel.h"
#include "recursivefilterproxymodel.h"
#include "races.h"
#include "roleaspect.h"
#include "sortabletableitems.h"
#include "trait.h"
#include "ui_roledialog.h"
#include "utils.h"
#include "viewmanager.h"

Q_DECLARE_METATYPE(const RolePreference *)

roleDialog::~roleDialog()
{
}

roleDialog::roleDialog(RolePreferenceModel *pref_model, QWidget *parent)
    : QDialog(parent)
    , ui(std::make_unique<Ui::roleDialog>())
    , m_pref_model(pref_model)
    , m_proxy_model(new RecursiveFilterProxyModel(this))
{
    ui->setupUi(this);
    //this->setAttribute(Qt::WA_DeleteOnClose,true);
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

    // Preference view
    ui->tree_prefs->set_model(pref_model);
    connect(ui->tree_prefs, &SearchFilterTreeView::item_activated,
            this, &roleDialog::preference_activated);

    connect(ui->btn_cancel, SIGNAL(clicked()), SLOT(close_pressed()));
    connect(ui->btn_save, SIGNAL(clicked()), SLOT(save_pressed()));
    connect(ui->btn_copy, SIGNAL(clicked()), SLOT(copy_pressed()));

    connect(ui->tw_attributes, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_attribute_context_menu(const QPoint &)));
    connect(ui->tw_traits, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_trait_context_menu(const QPoint &)));
    connect(ui->tw_skills, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_skill_context_menu(const QPoint &)));
    connect(ui->tw_prefs, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_prefs_context_menu(const QPoint &)));

    connect(ui->le_role_name, SIGNAL(textChanged(QString)), SLOT(name_changed(QString)));

    connect(ui->btnRefreshRatings, SIGNAL(clicked()), this, SLOT(selection_changed()));

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

    for (const auto &p: m_role->prefs)
        insert_pref_row(p.get());
}

void roleDialog::load_aspects_data(QTableWidget &table, const std::map<QString, RoleAspect> &aspects){
    table.setSortingEnabled(false);
    for (const auto &p: aspects){
        insert_row(table, p.second, p.first);
    }
    table.setSortingEnabled(true);
    table.sortItems(0,Qt::AscendingOrder);
}

void roleDialog::insert_row(QTableWidget &table, const RoleAspect &a, QString key){
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
    sortableTableWidgetItem* sItem = new sortableTableWidgetItem;
    table.setItem(row,1,sItem);

    table.setSortingEnabled(true);
}

void roleDialog::insert_pref_row(const RolePreference *p){
    ui->tw_prefs->setSortingEnabled(false);
    int row = ui->tw_prefs->rowCount();
    ui->tw_prefs->insertRow(row);
    ui->tw_prefs->setRowHeight(row,18);

    QTableWidgetItem *name = new QTableWidgetItem();
    name->setData(0,p->get_name());
    name->setData(Qt::UserRole, QVariant::fromValue(p));
    name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->tw_prefs->setItem(row,0,name);

    QDoubleSpinBox *dbs = new QDoubleSpinBox();
    dbs->setMinimum(-100);
    dbs->setMaximum(100);
    dbs->setSingleStep(0.25);
    dbs->setValue(p->aspect.is_neg ? 0-p->aspect.weight : p->aspect.weight);
    ui->tw_prefs->setCellWidget(row,1,dbs);
    sortableTableWidgetItem* sItem = new sortableTableWidgetItem;
    ui->tw_prefs->setItem(row,1,sItem);

    QTableWidgetItem *ptype = new QTableWidgetItem();
    ptype->setData(0, Preference::get_pref_desc(p->get_pref_category()));
    ptype->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->tw_prefs->setItem(row,2,ptype);

    QTableWidgetItem *itype = new QTableWidgetItem();
    if (auto ip = dynamic_cast<const ItemRolePreference *>(p))
        itype->setData(0, Item::get_item_name_plural(ip->get_item_type()));
    else
        itype->setData(0, tr("N/A"));
    itype->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->tw_prefs->setItem(row,3,itype);

    ui->tw_prefs->setSortingEnabled(true);
}

void roleDialog::add_aspect(QString id, QTableWidget &table, std::map<QString, RoleAspect> &list){
    RoleAspect a;
    a.is_neg = false;
    a.weight = 1.0;
    list.emplace(id.toLower(), a);
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

void roleDialog::save_aspects(QTableWidget &table, std::map<QString, RoleAspect> &list){
    for(int i= 0; i<table.rowCount(); i++){
        RoleAspect a;
        QString key = table.item(i,0)->data(Qt::UserRole).toString();
        float weight = static_cast<QDoubleSpinBox*>(table.cellWidget(i,1))->value();
        a.weight = fabs(weight);
        a.is_neg = weight < 0 ? true : false;
        list.emplace(key,a);
    }
}

void roleDialog::save_prefs(Role *r){
    for(int i= 0; i<ui->tw_prefs->rowCount(); i++){
        float weight = static_cast<QDoubleSpinBox*>(ui->tw_prefs->cellWidget(i,1))->value();
        auto p = ui->tw_prefs->item(i,0)->data(Qt::UserRole).value<const RolePreference *>()->copy();
        //save the weight of the preference for the next use
        p->aspect.weight = fabs(weight);
        p->aspect.is_neg = weight < 0 ? true : false;

        //update the values of this preference in the role
        RolePreference *rp = r->has_preference(p->get_name());
        if(!rp){
            r->prefs.emplace_back(std::move(p));
        }
        else {
            rp->aspect.weight = p->aspect.weight;
            rp->aspect.is_neg = p->aspect.is_neg;
        }
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
        if(!m_role->attributes.count(att_pair.second.toLatin1().toLower())){
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
            m_role->attributes.erase(ui->tw_attributes->item(i,0)->data(Qt::UserRole).toString().toLower());
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

    cmh.add_sub_menus(m,gdr->get_total_skill_count()/15,false);
    for (auto skill: gdr->get_ordered_skills()) {
        if(!m_role->skills.count((QString)skill->id)){
            QMenu *menu_to_use = cmh.find_menu(m,skill->noun);
            a = menu_to_use->addAction(skill->noun, this, SLOT(add_skill()));
            a->setData(skill->id);
            a->setToolTip(tr("Include %1 (ID %2) as an aspect for this role.)").arg(skill->noun).arg(skill->id));
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
            m_role->skills.erase(ui->tw_skills->item(i,0)->data(Qt::UserRole).toString().toLower());
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
        if(!m_role->traits.count((QString)trait_pair.first)){
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
            m_role->traits.erase(ui->tw_traits->item(i,0)->data(Qt::UserRole).toString().toLower());
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
            auto it = std::find_if(m_role->prefs.begin(), m_role->prefs.end(), [this, i] (const auto &p) {

                return p->get_name().toLower() == ui->tw_prefs->item(i,0)->data(Qt::UserRole).value<const RolePreference *>()->get_name().toLower();
            });
            if (it != m_role->prefs.end())
                m_role->prefs.erase(it);
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
        QPalette pal;
        if(GameDataReader::ptr()->get_default_roles().contains(text.trimmed())){
            ui->le_role_name->setStatusTip("This role has the same name as a default role and will override it.");
            pal.setColor(QPalette::Base, QColor::fromRgb(230,161,92,255));
            pal.setColor(QPalette::Text, Qt::black);
            m_override = true;
        }else{
            ui->le_role_name->setStatusTip("Name of the role.");
            m_override = false;
        }
        ui->le_role_name->setPalette(pal);
        emit event(new QStatusTipEvent(ui->le_role_name->statusTip()));
    }
}

void roleDialog::preference_activated(const QModelIndex &index){
    const RolePreference *p = m_pref_model->getPreference(index);
    if(p && !m_role->has_preference(p->get_name())){
        insert_pref_row(p);
    }
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

void roleDialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);

    m_pref_model->load_pref_from_raws(this);
}
