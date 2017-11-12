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

#include "gridviewdialog.h"
#include "ui_gridviewdialog.h"
#include "viewmanager.h"
#include "gridview.h"
#include "viewcolumnset.h"
#include "viewcolumn.h"
#include "spacercolumn.h"
#include "happinesscolumn.h"
#include "laborcolumn.h"
#include "skillcolumn.h"
#include "currentjobcolumn.h"
#include "traitcolumn.h"
#include "attributecolumn.h"
#include "rolecolumn.h"
#include "weaponcolumn.h"
#include "professioncolumn.h"
#include "highestmoodcolumn.h"
#include "trainedcolumn.h"
#include "healthcolumn.h"
#include "equipmentcolumn.h"
#include "superlaborcolumn.h"
#include "customprofessioncolumn.h"
#include "beliefcolumn.h"
#include "unitkillscolumn.h"

#include "contextmenuhelper.h"
#include "customprofession.h"
#include "defines.h"
#include "dfinstance.h"
#include "gamedatareader.h"
#include "healthcategory.h"
#include "item.h"
#include "itemweaponsubtype.h"
#include "labor.h"
#include "role.h"
#include "superlabor.h"
#include "trait.h"
#include "ui_vieweditor.h"
#include "unithealth.h"
#include "utils.h"
#include "viewcolumncolors.h"
#include "viewcolumnsetcolors.h"
#include "vieweditordialog.h"

#include <QMessageBox>
#include <QMenu>

GridViewDialog::GridViewDialog(ViewManager *mgr, GridView *view, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GridViewDialog)
    , m_view(view)
    , m_pending_view(new GridView((const GridView)*view))
    , m_manager(mgr)
    , m_is_editing(false)
    , m_set_model(new QStandardItemModel)
    , m_col_model(new QStandardItemModel)
    , m_temp_set(-1)
    , m_temp_col(-1)
    , m_active_set(0)
    , m_cmh(0)
{
    ui->setupUi(this);
    ui->list_sets->setModel(m_set_model);
    ui->list_columns->setModel(m_col_model);

    //remove the global sort column
    if(m_pending_view->sets().count() > 0)
        m_pending_view->get_set(0)->remove_column(0);

    connect(ui->list_sets->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        SLOT(set_selection_changed(const QItemSelection&, const QItemSelection&)));

    if (!m_pending_view->name().isEmpty()) { // looks like this is an edit...
        ui->le_name->setText(m_pending_view->name());
        ui->cb_animals->setChecked(m_pending_view->show_animals());
        draw_sets();
        ui->buttonBox->setEnabled(!m_pending_view->name().isEmpty());
        m_is_editing = true;
        m_original_name = m_pending_view->name();
    }

    /*/ TODO: show a STV with a preview using this gridview...
    StateTableView *stv = new StateTableView(this);
    QStandardItemModel *m = new QStandardItemModel(this);
    stv->setModel(m);
    ui->vbox->addWidget(stv, 10);
    */

    connect(ui->list_sets, SIGNAL(activated(const QModelIndex &)), SLOT(edit_set(const QModelIndex &)));
    connect(ui->list_sets, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_set_context_menu(const QPoint &)));
    connect(ui->list_columns, SIGNAL(activated(const QModelIndex &)), SLOT(edit_column(const QModelIndex &)));
    connect(ui->list_columns, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_column_context_menu(const QPoint &)));
    connect(ui->le_name, SIGNAL(textChanged(const QString &)), SLOT(check_name(const QString &)));

    connect(ui->list_columns->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(column_removed(QModelIndex, int, int)));
    connect(ui->list_sets->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(set_removed(QModelIndex, int, int)));
}

GridViewDialog::~GridViewDialog(){
    m_view = 0;
    m_pending_view = 0;
    m_cmh = 0;
    ui = 0;
}

QString GridViewDialog::name() {
     return ui->le_name->text();
}
void GridViewDialog::column_removed(QModelIndex, int, int){
    column_order_changed();
}
void GridViewDialog::set_removed(QModelIndex, int, int){
    set_order_changed();
}

//bool GridViewDialog::eventFilter(QObject *sender, QEvent *evt) {
//    if (evt->type() == QEvent::ChildRemoved) {
//        if (sender == ui->list_columns)
//            column_order_changed();
//        else if (sender == ui->list_sets)
//            set_order_changed();
//    }
//    return false; // don't actually interrupt anything
//}

void GridViewDialog::set_order_changed() {
    m_pending_view->reorder_sets(*m_set_model);
}

void GridViewDialog::column_order_changed() {
    m_active_set->reorder_columns(*m_col_model);
}

void GridViewDialog::draw_sets() {
    m_set_model->clear();
    m_col_model->clear();
    foreach(ViewColumnSet *set, m_pending_view->sets()) {
        QStandardItem *set_item = new QStandardItem(set->name());
        set_item->setBackground(set->bg_color());
        set_item->setForeground(complement(set->bg_color()));
        set_item->setDropEnabled(false);
        set_item->setData(set->name(), GPDT_TITLE);
        set_item->setData(set->bg_color(), GPDT_BG_COLOR);
        m_set_model->appendRow(set_item);
    }
    m_active_set = NULL;
    ui->list_sets->selectionModel()->select(m_set_model->index(0,0), QItemSelectionModel::SelectCurrent);
}

void GridViewDialog::set_selection_changed(const QItemSelection &selected, const QItemSelection &) {
    if (selected.indexes().size() != 1)
        return;
    QStandardItem *set_item = m_set_model->itemFromIndex(selected.indexes().at(0));
    foreach(ViewColumnSet *set, m_pending_view->sets()) {
        if (set->name() == set_item->data(GPDT_TITLE).toString()) {
            m_active_set = set;
            draw_columns_for_set(m_active_set);
        }
    }
}

void GridViewDialog::draw_columns_for_set(ViewColumnSet *set, bool set_changed) {
    m_col_model->clear();
    foreach(ViewColumn *vc, set->columns()) {
        QStandardItem *item = new QStandardItem(vc->title());
        item->setData(vc->title(), GPDT_TITLE);
        item->setData(vc->type(), GPDT_COLUMN_TYPE);

        QColor new_bg = vc->override_color() ? vc->bg_color() : set->bg_color();
        item->setBackground(new_bg);
        item->setData(new_bg,Qt::BackgroundColorRole);
        vc->set_bg_color(new_bg);

        item->setForeground(complement(new_bg));
        item->setData(complement(new_bg),Qt::TextColorRole);

        if(set_changed){
            vc->get_colors()->inherit_colors(*set->get_colors());
            vc->refresh_color_map();
        }

        item->setDropEnabled(false);
        m_col_model->appendRow(item);
    }
}

void GridViewDialog::check_name(const QString &name) {
    ui->buttonBox->setDisabled(name.isEmpty());
}

void GridViewDialog::add_set() {
    QRegExp rx = QRegExp("^New Set (\\d+)$");
    int highest_new_set_number = 0;
    foreach(ViewColumnSet *set, m_pending_view->sets()) {
        if (rx.indexIn(set->name()) != -1) {
            if (rx.cap(1).toInt() > highest_new_set_number)
                highest_new_set_number = rx.cap(1).toInt();
        }
    }

    ViewColumnSet *set = new ViewColumnSet(tr("New Set %1").arg(highest_new_set_number + 1), m_manager);
    m_pending_view->add_set(set);
    draw_sets();
    ui->list_sets->selectionModel()->select(m_set_model->index(m_pending_view->sets().count()-1,0), QItemSelectionModel::SelectCurrent);
}

void GridViewDialog::edit_set() {
    if (m_temp_set < 0)
        return;
    edit_set(m_set_model->index(m_temp_set, 0));
    ui->list_sets->selectionModel()->select(m_set_model->index(m_temp_set,0), QItemSelectionModel::SelectCurrent);
}

void GridViewDialog::edit_set(const QModelIndex &idx) {
    QStandardItem *item = m_set_model->itemFromIndex(idx);
    ViewEditorDialog *d = new ViewEditorDialog(m_active_set,this);

    if(d->exec()) {
        ViewColumnSet *set = m_pending_view->get_set(item->data(GPDT_TITLE).toString());
        set->set_name(d->ui->le_title->text());

        set->set_bg_color(d->background_color());
        item->setBackground(set->bg_color());
        item->setData(set->bg_color(),Qt::BackgroundColorRole);

        set->get_colors()->set_overrides_cell_colors(d->ui->cb_override_cell_colors->isChecked());
        if(d->ui->cb_override_cell_colors->isChecked()){
            for(int idx=0;idx < set->get_colors()->get_color_defs().count();idx++){
                set->get_colors()->set_color(idx,d->color(idx));
            }
        }else{
            set->get_colors()->use_defaults();
        }
        draw_sets();
        draw_columns_for_set(set,true);
    }
    delete d;
}

void GridViewDialog::remove_set() {
    QModelIndexList selected = ui->list_sets->selectionModel()->selectedIndexes();
    QList<ViewColumnSet*> sets_to_remove;
    foreach(QModelIndex idx, selected) {
        sets_to_remove << m_pending_view->get_set(idx.row());
    }
    foreach(ViewColumnSet *set, sets_to_remove) {
        m_pending_view->remove_set(set);
    }
    draw_sets();
}

void GridViewDialog::draw_set_context_menu(const QPoint &p) {
    QMenu m(this);
    QModelIndex idx = ui->list_sets->indexAt(p);
    if (idx.isValid()) {
        m.addAction(QIcon(":img/table--pencil.png"), tr("Edit..."), this, SLOT(edit_set()));
        m.addAction(QIcon(":img/minus-circle.png"), tr("Remove"), this, SLOT(remove_set()));
        m_temp_set = idx.row();
    } else {
        m.addAction(tr("Add New Set"), this, SLOT(add_set()));
    }
    m.exec(ui->list_sets->viewport()->mapToGlobal(p));
}


void GridViewDialog::edit_column() {
    if (m_temp_col < 0)
        return;
    edit_column(m_col_model->index(m_temp_col, 0));
    ui->list_columns->selectionModel()->select(m_col_model->index(m_temp_col,0), QItemSelectionModel::SelectCurrent);
}

void GridViewDialog::edit_column(const QModelIndex &idx) {
    ViewColumn *vc = m_active_set->column_at(idx.row());
    ViewEditorDialog *d = new ViewEditorDialog(vc,this);

    if (d->exec()) { //accepted
        vc->set_title(d->ui->le_title->text());
        if (d->ui->cb_override->isChecked()) {
            vc->set_bg_color(d->background_color());
        }else{
            vc->set_bg_color(m_active_set->bg_color());
        }

        vc->set_override_color(d->ui->cb_override->isChecked());
        vc->get_colors()->set_overrides_cell_colors(d->ui->cb_override_cell_colors->isChecked());
        if(d->ui->cb_override_cell_colors->isChecked()){
            for(int idx=0;idx < vc->get_colors()->get_color_defs().count();idx++){
                vc->get_colors()->set_color(idx,d->color(idx));
            }
        }else{
            vc->get_colors()->inherit_colors(*m_active_set->get_colors());
        }
        vc->refresh_color_map();

        if (vc->type() == CT_SPACER) {
            SpacerColumn *c = static_cast<SpacerColumn*>(vc);
            int w = d->ui->sb_width->value();
            if (w < 1)
                w = DEFAULT_SPACER_WIDTH;
            c->set_width(w);
        }
        draw_columns_for_set(m_active_set);
    }
    delete d;
}

void GridViewDialog::remove_column() {
    QModelIndexList selected = ui->list_columns->selectionModel()->selectedIndexes();
    QList<ViewColumn*> cols_to_remove;
    foreach(QModelIndex idx, selected) {
        cols_to_remove << m_active_set->column_at(idx.row());
    }
    foreach(ViewColumn *vc, cols_to_remove) {
        m_active_set->remove_column(vc);
    }
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::draw_column_context_menu(const QPoint &p) {
    QMenu *m = new QMenu(this);
    QModelIndex idx = ui->list_columns->indexAt(p);
    if (idx.isValid()) { // context on a column
        m->addAction(QIcon(":img/pencil.png"), tr("Edit Selected"), this, SLOT(edit_column()));
        m->addAction(QIcon(":img/minus-circle.png"), tr("Remove Selected"), this, SLOT(remove_column()));
        m->addSeparator();
        m_temp_col = idx.row();
    } //else { // in whitespace

    if (!m_active_set) { // can't do much without a parent for our cols
        QMessageBox::warning(this, tr("No Set Selected"),
                             tr("Please select an existing set on the left side pane before "
                                "attempting to modify columns. If there are no sets yet, "
                                "create one first."));
        return;
    }

    m_cmh = new ContextMenuHelper(this);
    connect(m_cmh,SIGNAL(all_clicked()),this,SLOT(all_clicked()));

    QAction *a;
    GameDataReader *gdr = GameDataReader::ptr();

    //ATTRIBUTE
    QMenu *m_attr = m_cmh->create_title_menu(m, tr("Attribute"),"");
    QList<QPair<ATTRIBUTES_TYPE, QString> > atts = gdr->get_ordered_attribute_names();
    QPair<ATTRIBUTES_TYPE, QString> att_pair;
    foreach(att_pair, atts){
        a = m_attr->addAction(tr(att_pair.second.toLatin1()), this, SLOT(add_attribute_column()));
        a->setData(att_pair.first);
    }

    //BELIEFS
    QMenu *m_belief = m_cmh->create_title_menu(m, tr("Belief"),tr("Belief columns show a read-only display of a dwarf's score in a particular Belief."));
    m_cmh->add_sub_menus(m_belief,2);
    QList<QPair<int, QString> > beliefs = gdr->get_ordered_beliefs();
    QPair<int, QString> b_pair;
    foreach(b_pair, beliefs) {
        QMenu *menu_to_use = m_cmh->find_menu(m_belief,b_pair.second);
        QAction *a = menu_to_use->addAction(b_pair.second, this, SLOT(add_belief_column()));
        a->setData(b_pair.first);
        a->setToolTip(tr("Add a column for Belief %1 (ID%2)").arg(b_pair.second).arg(b_pair.first));
    }

    //CURRENT JOB
    a = m->addAction(tr("Current Job"), this, SLOT(add_idle_column()));
    a->setToolTip(tr("Adds a single column that shows a the current idle state for a dwarf."));

    //CUSTOM PROFESSIONS
    QMenu *m_custom_profs = m_cmh->create_title_menu(m,tr("Custom Profession"),tr("Columns which can apply or remove a custom profession and its labors."));
    m_cmh->add_sub_menus(m_custom_profs,2);
    foreach(CustomProfession *cp, DT->get_custom_professions()){
            QMenu *menu_to_use = m_cmh->find_menu(m_custom_profs,cp->get_name());
            QAction *a = menu_to_use->addAction(cp->get_name(), this, SLOT(add_custom_prof_column()));
            a->setData(cp->get_name());
    }

    //EQUIPMENT
    a = m->addAction(tr("Equipment"), this, SLOT(add_equipment_column()));
    a->setToolTip(tr("Adds a color coded column that shows if a dwarf is fully clothed. Also shows all equipment in the tooltip grouped by body part."));

    //HAPPINESS
    a = m->addAction(tr("Happiness"), this, SLOT(add_happiness_column()));
    a->setToolTip(tr("Adds a single column that shows a color-coded happiness indicator for "
                     "each dwarf. You can customize the colors used in the options menu."));

    //HEALTH
    QMenu *m_health = m_cmh->create_title_menu(m,tr("Health Column"),tr("Health columns will show various information about status, treatment and wounds."));
    m_cmh->add_sub_menus(m_health,4);
    QList<QPair<eHealth::H_INFO,QString> > cat_names = UnitHealth::ordered_category_names();
    QPair<eHealth::H_INFO,QString> health_pair;
    foreach(health_pair, cat_names){
        QString name = health_pair.second;
        QMenu *menu_to_use = m_cmh->find_menu(m_health,name);
        QAction *a = menu_to_use->addAction(name,this,SLOT(add_health_column()));
        a->setData(health_pair.first);
        a->setToolTip(tr("Add a column for %1").arg(name));
    }

    //INVENTORY
    QMenu *m_inventory = m_cmh->create_title_menu(m,tr("Inventory"),tr("Shows the currently equipped inventory of a particular item category."));
    QList<ITEM_TYPE> item_cats;
    item_cats << AMMO << ARMOR << SHOES << GLOVES << HELM << PANTS << SHIELD << BACKPACK << FLASK << QUIVER << WEAPON;
    foreach(ITEM_TYPE itype, item_cats){
        QString name = Item::get_item_name_plural(itype);
        QAction *a = m_inventory->addAction(name.replace("&","&&"),this,SLOT(add_equipment_column()));
        a->setData(itype);
        a->setToolTip(tr("Add a column for %1").arg(name));
    }
    item_cats.clear();
    m_inventory->addSeparator();
    item_cats << SUPPLIES << RANGED_EQUIPMENT << MELEE_EQUIPMENT; //groups
    foreach(ITEM_TYPE itype, item_cats){
        QString name = Item::get_item_name_plural(itype);
        QAction *a = m_inventory->addAction(name.replace("&","&&"),this,SLOT(add_equipment_column()));
        a->setData(itype);
        a->setToolTip(tr("Add a column for %1").arg(name));
    }

    //KILLS
    a = m->addAction(tr("Kills"), this, SLOT(add_kills_column()));
    a->setToolTip(tr("Adds a single column that shows a unit's kills."));

    //LABOUR
    QMenu *m_labor = m_cmh->create_title_menu(m,tr("Labor"),tr("Labor columns function as toggle switches for individual labors on a dwarf."));
    m_cmh->add_sub_menus(m_labor,5);
    foreach(Labor *l, gdr->get_ordered_labors()) {
        QMenu *menu_to_use = m_cmh->find_menu(m_labor,l->name);
        QAction *a = menu_to_use->addAction(l->name, this, SLOT(add_labor_column()));
        a->setData(l->labor_id);
        a->setToolTip(tr("Add a column for labor %1 (ID%2)").arg(l->name).arg(l->labor_id));
    }

    //MOODABLE SKILL
    a = m->addAction(tr("Moodable Skill"), this, SLOT(add_highest_moodable_column()));
    a->setToolTip(tr("Adds a single column that shows an icon representing a dwarf's highest moodable skill."));

    //PROFESSION
    a = m->addAction(tr("Profession"), this, SLOT(add_profession_column()));
    a->setToolTip(tr("Adds a single column that shows an icon representing a dwarf's profession."));

    //ROLES
    QMenu *m_roles = m_cmh->create_title_menu(m,tr("Role"),tr("Role columns will show how well a dwarf can fill a particular role."));
    m_cmh->add_sub_menus(m_roles,gdr->get_ordered_roles().count() / 20);
    QList<QPair<QString, Role*> > roles = gdr->get_ordered_roles();
    QPair<QString, Role*> role_pair;
    foreach(role_pair, roles){
        Role *r = role_pair.second;
        QMenu *menu_to_use = m_cmh->find_menu(m_roles,r->name());
        QAction *a = menu_to_use->addAction(r->name(), this, SLOT(add_role_column()));
        a->setData(role_pair.first);
        a->setToolTip(tr("Add a column for role %1 (ID%2)").arg(r->name()).arg(role_pair.first));
    }

    //SKILL
    QMenu *m_skill = m_cmh->create_title_menu(m,tr("Skill"), tr("Skill columns function as a read-only display of a dwarf's skill in a particular area."));
    m_cmh->add_sub_menus(m_skill,gdr->get_total_skill_count() / 15);
    for (auto skill: gdr->get_ordered_skills()) {
        QMenu *menu_to_use = m_cmh->find_menu(m_skill,skill->noun);
        QAction *a = menu_to_use->addAction(skill->noun, this, SLOT(add_skill_column()));
        a->setData(skill->id);
        a->setToolTip(tr("Add a column for skill %1 (ID%2)").arg(skill->noun).arg(skill->id));
    }

    //SPACER
    a = m->addAction("Spacer", this, SLOT(add_spacer_column()));
    a->setToolTip(tr("Adds a non-selectable spacer to this set. You can set a custom width and color on spacer columns."));

    //SUPER LABORS
    QMenu *m_super_labors = m_cmh->create_title_menu(m,tr("Super Labor"),tr("Columns which toggle multiple labors at a time."));
    m_cmh->add_sub_menus(m_super_labors,2);
    foreach(SuperLabor *sl, DT->get_super_labors()) {
        QMenu *menu_to_use = m_cmh->find_menu(m_super_labors,sl->get_name());
        QAction *a = menu_to_use->addAction(sl->get_name(), this, SLOT(add_super_labor_column()));
        a->setData(sl->get_name());
    }

    //TRAINED (animals)
    a = m->addAction("Trained Level", this, SLOT(add_trained_column()));
    a->setToolTip(tr("Adds a column showing the trained level of an animal."));

    //TRAIT
    QMenu *m_trait = m_cmh->create_title_menu(m, tr("Trait"),tr("Trait columns show a read-only display of a dwarf's score in a particular trait."));
    m_cmh->add_sub_menus(m_trait,2);
    QList<QPair<int, Trait*> > traits = gdr->get_ordered_traits();
    QPair<int, Trait*> trait_pair;
    foreach(trait_pair, traits) {
        Trait *t = trait_pair.second;
        QMenu *menu_to_use = m_cmh->find_menu(m_trait,t->get_name());
        QAction *a = menu_to_use->addAction(t->get_name(), this, SLOT(add_trait_column()));
        a->setData(trait_pair.first);
        a->setToolTip(tr("Add a column for trait %1 (ID%2)").arg(t->get_name()).arg(trait_pair.first));
    }

    //WEAPONS
    QMenu *m_weapon = m_cmh->create_title_menu(m, tr("Weapon"),
                                        tr("Weapon columns will show an indicator of whether the dwarf can wield the weapon with one hand, two hands or not at all."));
    if (auto df = DT->get_DFInstance()) {
        m_cmh->add_sub_menus(m_weapon,df->get_ordered_weapon_defs().count() / 15);
        foreach(ItemWeaponSubtype *w, df->get_ordered_weapon_defs().values()) {
            QString title = w->name_plural(); //allow adding every type
            QMenu *menu_to_use = m_cmh->find_menu(m_weapon,title);
            QAction *a = menu_to_use->addAction(title, this, SLOT(add_weapon_column()));
            a->setData(w->subType());
            a->setToolTip(tr("Add a column for weapon %1").arg(title));
        }
    }
    else {
        QAction *a = m_weapon->addAction(tr("Not available"));
        a->setEnabled(false);
        a->setToolTip(tr("A DF instance is required"));
    }

    //    }
    m->exec(ui->list_columns->viewport()->mapToGlobal(p));
}

void GridViewDialog::all_clicked(){
    if(m_active_set)
        draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_spacer_column() {
    if (!m_active_set)
        return;

    new SpacerColumn(tr("SPACER"), m_active_set, m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_trained_column(){
    if (!m_active_set)
        return;

    new TrainedColumn(tr("Training"),m_active_set, m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_kills_column(){
    if (!m_active_set)
        return;

    new UnitKillsColumn(tr("Kills"),m_active_set, m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_equipment_column(){
    if (!m_active_set)
        return;

    QAction *a = qobject_cast<QAction*>(QObject::sender());
    if(a->data().isValid()){
        ITEM_TYPE itype = static_cast<ITEM_TYPE>(a->data().toInt());
        QString name = a->text();
        new ItemTypeColumn(name.replace("&&","&"),itype,m_active_set,m_active_set);
    }else{
        new EquipmentColumn(tr("Equipment"),m_active_set, m_active_set);
    }
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_happiness_column() {
    if (!m_active_set)
        return;
    new HappinessColumn(tr("Happiness"), m_active_set, m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_idle_column() {
    if (!m_active_set)
        return;
    new CurrentJobColumn(tr("Current Job"), m_active_set, m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_labor_column() {
    if (!m_active_set)
        return;
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    int labor_id = a->data().toInt();
    Labor *l = GameDataReader::ptr()->get_labor(labor_id);
    if (!l) {
        LOGE << tr("Failed to get a labor with id %1!").arg(labor_id);
        return;
    }
    new LaborColumn(l->name, l->labor_id, l->skill_id, m_active_set, m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_super_labor_column() {
    if (!m_active_set)
        return;
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    QString name = a->data().toString();
    SuperLabor *sl = DT->get_super_labor(name);
    if (!sl) {
        LOGE << tr("Failed to find super labor %1!").arg(name);
        return;
    }
    new SuperLaborColumn(name,name,m_active_set,m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_custom_prof_column() {
    if (!m_active_set)
        return;
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    QString name = a->data().toString();
    CustomProfession *cp = DT->get_custom_profession(name);
    if (!cp) {
        LOGE << tr("Failed to find custom profession %1!").arg(name);
        return;
    }
    new CustomProfessionColumn(name,name,m_active_set,m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_skill_column() {
    if (!m_active_set)
        return;
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    int skill_id = a->data().toInt();
    new SkillColumn(GameDataReader::ptr()->get_skill_name(skill_id), skill_id, m_active_set, m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_trait_column() {
    if (!m_active_set)
        return;
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    int trait_id = a->data().toInt();
    new TraitColumn(GameDataReader::ptr()->get_trait(trait_id)->get_name(), trait_id, m_active_set, m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_belief_column() {
    if (!m_active_set)
        return;
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    int belief_id = a->data().toInt();
    new BeliefColumn(GameDataReader::ptr()->get_belief_name(belief_id), belief_id, m_active_set, m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_attribute_column() {
    if (!m_active_set)
        return;
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    ATTRIBUTES_TYPE type = static_cast<ATTRIBUTES_TYPE>(a->data().toInt());
    new AttributeColumn(GameDataReader::ptr()->get_attribute_name(type), type, m_active_set, m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_role_column() {
    if (!m_active_set)
        return;
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    QString role_name = a->data().toString();
    new RoleColumn(role_name,GameDataReader::ptr()->get_role(role_name), m_active_set, m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_weapon_column(){
    if(!m_active_set)
        return;
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    int sub_type = a->data().toInt();
    ItemWeaponSubtype *w = qobject_cast<ItemWeaponSubtype*>(DT->get_DFInstance()->get_item_subtype(WEAPON,sub_type));
    if(w){
        new WeaponColumn(w->name_plural(),sub_type,m_active_set,m_active_set);
        draw_columns_for_set(m_active_set);
    }
}

void GridViewDialog::add_health_column(){
    if(!m_active_set)
        return;
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    eHealth::H_INFO key = static_cast<eHealth::H_INFO>(a->data().toInt());
    new HealthColumn(UnitHealth::get_display_categories().value(key)->name(),key,m_active_set,m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_profession_column(){
    if (!m_active_set)
        return;
    new ProfessionColumn(tr("Profession"), m_active_set, m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_highest_moodable_column(){
    if (!m_active_set)
        return;
    new HighestMoodColumn(tr("Moodable Skill"), m_active_set, m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::accept() {
    if (ui->le_name->text().isEmpty()) {
        QMessageBox::warning(this, tr("Empty Name"), tr("Cannot save a view with no name!"));
        return;
    } else if (m_manager->get_view(ui->le_name->text())) {
        // this name exists
        if (!m_is_editing || (m_is_editing && m_original_name != ui->le_name->text())) {
            QMessageBox m(QMessageBox::Question, tr("Overwrite View?"),
                tr("There is already a view named <b>%1</b><h3>Do you want to overwrite it?</h3>").arg(ui->le_name->text()),
                QMessageBox::Yes | QMessageBox::No, 0);
            if (m.exec() == QMessageBox::Yes) {
                return QDialog::accept();
            } else {
                return;
            }
        }
    }
    m_pending_view->set_name(ui->le_name->text());
    m_pending_view->set_show_animals(ui->cb_animals->isChecked());

    //add the global sort column
    if(m_pending_view->sets().count() > 0){
        ViewColumnSet *first_set = m_pending_view->get_set(0);
        new SpacerColumn(0,0,first_set,first_set);
    }

    return QDialog::accept();
}
